#pragma once
#include "Basetypes.h"
#include "synchronization.h"

namespace PH::Base {

	#define ALLOCATE static void* alloc(PH::sizeptr size)
	#define DEALLOCATE static PH::bool32 dealloc(void* memory, PH::sizeptr size)

	class AllocatorInterface {
		static void* alloc(sizeptr size) {
			return nullptr;
		}
		static bool32 dealloc(void* memory) {
			return false;
		}
	};

	inline void copyMemory(void* src, void* target, sizeptr size) {
		for (sizeptr byte = 0; byte < size; byte++) {
			*((uint8*)target + byte) = *((uint8*)src + byte);
		}
	}

	inline void copyMemoryReverse(void* src, void* target, int64 size) {
		for (int64 byte = (int64)size - 1; byte >= 0; byte--) {
			*((uint8*)target + byte) = *((uint8*)src + byte);
		}
	}

	inline void setMemoryUint8(void* target, sizeptr size, uint8 value) {
		for (sizeptr i = 0; i < size; i++) {
			((uint8*)target)[i] = value;
		}
	}

	struct DynamicMemoryBuffer {
		mutex_lock lock = MUTEX_UNLOCKED;

		void* memory;
		sizeptr size;
	};

	struct MemoryArena {
		void* memory;
		sizeptr head;
		sizeptr size;
	};

	typedef sizeptr MemoryArenaReset;

	void* retrieveMemory(MemoryArena* arena, sizeptr size);
	MemoryArenaReset getCurrentResetPoint(MemoryArena* arena);
	bool32 resetMemoryArena(MemoryArena* arena, MemoryArenaReset resetpoint);
	MemoryArena createMemoryArena(void* mem, sizeptr size);


	DynamicMemoryBuffer createDynamicMemoryBuffer(void* mem, sizeptr size);
	void* DynamicAllocateFirstFit(DynamicMemoryBuffer* mem, sizeptr size);
	bool DynamicDeallocate(DynamicMemoryBuffer* mem, void* data);

}


