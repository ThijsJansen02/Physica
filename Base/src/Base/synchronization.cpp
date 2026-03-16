#include "synchronization.h"
#include "Log.h"

namespace PH::Base {


	void lock(mutex_lock& lock, uint32 syncgranularity_ms) {
		_ReadWriteBarrier();
		while (true) {

			//wait on the mutex to open
			while (lock == MUTEX_LOCKED) {
				INFO << "waiting on mutex lock!\n";
				Sleep(syncgranularity_ms);
			}

			if (InterlockedCompareExchange(&lock, MUTEX_LOCKED, MUTEX_UNLOCKED) == MUTEX_UNLOCKED) {
				break;
			}
		}
		_ReadWriteBarrier();
	}

	void unlock(mutex_lock& lock) {
		_ReadWriteBarrier();
		lock = MUTEX_UNLOCKED;
	}


}