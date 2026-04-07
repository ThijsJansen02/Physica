#pragma once

#include "Array.h"

///code is currently only implemented for windows, but it should be possible to implement it for other platforms as well, if needed. The circular work queue is used to distribute work between threads, and is used in the asset library to distribute loading work between the loader threads.
#ifdef PH_PLATFORM_WINDOWS
#include <windows.h>
/// <summary>
/// work queue capable of multithreaded queue operations
/// </summary>
/// <typeparam name="Work"></typeparam>
/// 
/// 


namespace PH::Base {

	template<typename Work, typename allocator>
	struct CircularWorkQueue {

		volatile DWORD writeptr = 0;
		volatile DWORD readptr = 0;

		static CircularWorkQueue create(sizeptr capacity) {
			CircularWorkQueue q;
			q.storage = DynamicArray<Work, allocator>::create(capacity);
			q.writeptr = 0;
			q.readptr = 0;
			return q;
		}

		void push(const Work& work) {

			storage[writeptr] = work;

			_ReadWriteBarrier(); //make sure the compiler doesnt increase the writeptr before the work is submitted to the queue

			writeptr++;
			writeptr %= storage.getCapacity();
		}

		bool32 hasWork() {
			return writeptr != readptr;
		}

		bool32 isFull() {
			return (writeptr + 1) % storage.getCapacity() == readptr;
		}

		Work* pop() {
			while (true) {
				if (!hasWork()) {
					return nullptr;
				}

				DWORD index = readptr;
				DWORD newreadptr = (readptr + 1) % storage.getCapacity();
				Work* w = &storage[index % storage.getCapacity()];

				if (InterlockedCompareExchange(&readptr, newreadptr, index) == index) {
					return w;
				}
			}
		}
		DynamicArray<Work, allocator> storage;
	};


}


#endif