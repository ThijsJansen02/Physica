#pragma once
#include "Basetypes.h"
#include "Memory.h"
#include "Base.h"
#include "Log.h"

#define ASSERT_BREAK(x, m) if(!(x)) {/*TO print error message*/}

namespace PH::Base {

	struct MemoryHeader {
		uint64 head;
	};

	/// <summary>
	/// sets the bit a parameter position to 1. the postion is from least significant bit to most significant bit.
	/// </summary>
	/// <param name="target">the sizeptr which bit needs to be set</param>
	/// <param name="position">the position of the bit that needs to be set</param>
	inline void setbitSizeptr(sizeptr* target, uint32 position) {

		sizeptr mask = 1;
		mask = mask << position;
		*target = *target | mask;
	}

	/// <summary>
	/// sets the bit at parameter position to 0. the position is from the least significant bit tot most significant bit.
	/// </summary>
	/// <param name="target">the sizeptr that needs to be set</param>
	/// <param name="position">the position the needs to be set</param>
	inline void resetBitSizeptr(sizeptr* target, uint32 position) {
		sizeptr mask = 1;
		mask = mask << position;
		*target = *target & ~mask;
	}

	inline bool getBitSizeptr(sizeptr target, uint32 position) {
		sizeptr mask = 1;
		mask = mask << position;
		return (target & mask) > 0;
	}

	inline void resetLast4bitsSizeptr(sizeptr* target) {
		sizeptr mask = 15;
		*target = *target & ~mask;
	}

#define BLOCKSIZE 16
#define BLOCKSIZEBITS 4
#define HEADERSIZE sizeof(MemoryHeader)

	inline sizeptr getnBlocks(MemoryHeader* header) {
		return header->head >> BLOCKSIZEBITS;
	}

	inline MemoryHeader* getNextHead(MemoryHeader* head) {
		sizeptr nblocks = getnBlocks(head);
		return head + (nblocks << 1);
	}

	inline MemoryHeader* getPrevHead(MemoryHeader* head) {
		sizeptr nblocks = getnBlocks(head - 1);
		return head - (nblocks << 1);
	}

	inline MemoryHeader* getFooter(MemoryHeader* head) {
		sizeptr nblocks = getnBlocks(head);
		return head + (nblocks << 1) - 1;
	}

	inline void setHeaderForAllocatedMemory(MemoryHeader* header, sizeptr nblocks) {
		sizeptr size = nblocks << BLOCKSIZEBITS;
		header->head = size;
		setbitSizeptr(&header->head, 0);
	}

	inline void setHeaderForDeallocatedMemory(MemoryHeader* header, sizeptr nblocks) {
		sizeptr size = nblocks << BLOCKSIZEBITS;
		header->head = size;
		resetBitSizeptr(&header->head, 0);
	}

	inline sizeptr getnHeadersSizes(MemoryHeader* head) {
		return head->head >> (BLOCKSIZEBITS - 1);
	}

	DynamicMemoryBuffer createDynamicMemoryBuffer(void* mem, sizeptr size) {
		
		size -= HEADERSIZE;
		size -= size % BLOCKSIZE;
		sizeptr nblocks = size / BLOCKSIZE;

		DynamicMemoryBuffer buffer;
		buffer.size = size;
		buffer.memory = mem;
		
		//set the header
		MemoryHeader* headptr = (MemoryHeader*)mem;
		setHeaderForDeallocatedMemory(headptr, nblocks);

		//set the footer
		MemoryHeader* footerptr = getFooter(headptr);
		setHeaderForDeallocatedMemory(footerptr, nblocks);

		//set the last header with size null to indicate that this is the end of the buffer
		MemoryHeader* nullheader = getNextHead(headptr);
		setHeaderForDeallocatedMemory(nullheader, 0);

		return buffer;
	}

	void* DynamicAllocateFirstFit(DynamicMemoryBuffer* mem, sizeptr size) {

		if (size == 0) {
			return nullptr;
		}

		if (size % BLOCKSIZE == 0) {
			size += BLOCKSIZE;
		} else {
			size += (2 * BLOCKSIZE);
		}

		//NOTE (Thijs): if blocksize changes this needs to change as well.
		resetLast4bitsSizeptr(&size);

		MemoryHeader* ptr = (MemoryHeader*)mem->memory;

		sizeptr blocksneeded = size >> BLOCKSIZEBITS;

		uint32 searchcount = 0;

		lock(mem->lock);
		while (true) {
			sizeptr blocksavailable = getnBlocks(ptr);
			//if nblocks == 0 then we are at the last block and this means we are add the end of the memory and we cant find memory that fits
			
			if (blocksavailable == 0) {

				Base::ERR << "Tried to allocate more memory than available in the this memory buffer!";
				PH_BREAK();
				return nullptr;
			}


			if (ptr->head != getFooter(ptr)->head) {
				Base::ERR << "head does not equal footer at block: " << (uint32)((MemoryHeader*)ptr - (MemoryHeader*)mem->memory) / 2 << "\n";
				Base::ERR << "memory integrity is off\n";
				PH_BREAK();
			}



			//if the block is already allocated, move to the next block
			if (getBitSizeptr(ptr->head, 0)) {

				//nbocks shifted left once becauce memoryheader is half a blocksize;
				ptr = getNextHead(ptr);
			}
			else {
				if (blocksavailable >= blocksneeded) {

					/*if ((uint32)((MemoryHeader*)ptr - (MemoryHeader*)mem->memory) / 2 == 2155) {
						DebugBreak();
					}*/
					//Base::INFO << "allocated nblocks: " << (uint32)blocksneeded << " at location block: " << (uint32)((MemoryHeader*)ptr - (MemoryHeader*)mem->memory) / 2 << "\n";


					sizeptr surplusblocks = blocksavailable - blocksneeded;


					if (surplusblocks < 4) {
						//Base::INFO << "allocated perfect fit " << (uint32)surplusblocks << "\n";

						setHeaderForAllocatedMemory(ptr, blocksavailable);
						MemoryHeader* footer = getFooter(ptr);
						setHeaderForAllocatedMemory(footer, blocksavailable);

						_ReadWriteBarrier();
						unlock(mem->lock);
						return (void*)(ptr + 1);
					} else {
						setHeaderForAllocatedMemory(ptr, blocksneeded);
						MemoryHeader* footer = getFooter(ptr);
						setHeaderForAllocatedMemory(footer, blocksneeded);
						
						MemoryHeader* nexthead = getNextHead(ptr);
						setHeaderForDeallocatedMemory(nexthead, surplusblocks);
						MemoryHeader* nextfooter = getFooter(nexthead);
						setHeaderForDeallocatedMemory(nextfooter, surplusblocks);

						_ReadWriteBarrier();
						unlock(mem->lock);
						return (void*)(ptr + 1);
					}
				} else {
					ptr = getNextHead(ptr);
				}
			}
			searchcount++;
		}
	}

	bool DynamicDeallocate(DynamicMemoryBuffer* mem, void* data) {

		//PH_DEBUG_ASSERT(data, "data is null")
		if (!data) {
			return false;
		}


		MemoryHeader* head = (MemoryHeader*)data;
		head--;
		MemoryHeader* footer = getFooter(head);

		sizeptr nblocks = getnBlocks(head);

		//Base::INFO << "deallocated nblocks: " << (uint32)nblocks << "at block location" << (uint32)((MemoryHeader*)head - (MemoryHeader*)mem->memory) / 2 << "\n";

		MemoryHeader* prev = getPrevHead(head);
		MemoryHeader* next = getNextHead(head);

		if (!getBitSizeptr(prev->head, 0)) {
			sizeptr prevblocks = getnBlocks(prev);
			nblocks += prevblocks;
			head = prev;
		}

		if (!getBitSizeptr(next->head, 0)) {
			sizeptr nextblocks = getnBlocks(next);
			nblocks += nextblocks;
			footer = getFooter(next);
		}

		setHeaderForDeallocatedMemory(head, nblocks);
		setHeaderForDeallocatedMemory(footer, nblocks);

		return true;
	}

	void* retrieveMemory(MemoryArena* arena, sizeptr size) {
		PH_ASSERT_BREAK(!(arena->head + size > arena->size), "Memory arena out of bounds!");
		void* mem = (uint8*)arena->memory + arena->head;
		arena->head += size;
		return mem;
	}


	MemoryArenaReset getCurrentResetPoint(MemoryArena* arena) {
		return arena->head;
	}

	bool32 resetMemoryArena(MemoryArena* arena, MemoryArenaReset resetpoint) {
		arena->head = resetpoint;
		return true;
	}

	MemoryArena createMemoryArena(void* mem, sizeptr size) {
		MemoryArena result;
		result.head = 0;
		result.memory = mem;
		result.size = size;

		return result;
	}

}