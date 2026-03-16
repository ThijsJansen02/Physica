#pragma once
#include "Base.h"

namespace PH::Base {

	inline uint32 pcg_hash(uint32 input)
	{
		uint32 state = input * 747796405u + 2891336453u;
		uint32 word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		return (word >> 22u) ^ word;
	}

	//only supports a uint32 size because there are only 32bit amount of different inputs for the pcg_hash
	inline void fillArrayUniformUint32(uint32* array, uint32 size, uint32 seed) {
		for (sizeptr i = 0; i < size; i++) {
			array[i] = pcg_hash(seed + i);
		}
	}

	inline void fillArrayUniformReal32(real32* array, uint32 size, uint32 seed) {
		for (sizeptr i = 0; i < size; i++) {
			array[i] = (real32)((real64)pcg_hash(seed + i) / 0xFFFFFFFF);
		}
	}


}