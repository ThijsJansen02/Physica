#pragma once
#include "Base.h"

namespace PH::Base {

	typedef volatile PH::uint32 mutex_lock;
#define MUTEX_LOCKED 1
#define MUTEX_UNLOCKED 0

	void lock(mutex_lock& lock, uint32 syncgranularity_ms = 0);
	void unlock(mutex_lock& lock);



}