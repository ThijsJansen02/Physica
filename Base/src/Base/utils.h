#pragma once
#include <Base/Base.h>
#include <math.h>

namespace PH::Base {

	template<typename T>
	inline T minVal(T val1, T val2) {

		if (val1 < val2) {
			return val1;
		}
		else {
			return val2;
		}
	}

	template<typename T>
	inline T maxVal(T val1, T val2) {
		if (val1 > val2) {
			return val1;
		}
		else {
			return val2;
		}
	}

	template<typename T>
	inline T clamp(T val, T min, T max) {
		if (val < min) {
			return min;
		}
		if (val > max) {
			return max;
		}
		return val;
	}

	template<typename T1, typename T2>
	struct Pair
	{
		T1 v1;
		T2 v2;
	};

	inline int64 floorReal64(real64 v) {
		return (uint64)floor(v);
	}

	inline real64 powReal64(real64 b, real32 exp) {
		return pow(b, exp);
	}

	inline uint64 powUint64(uint64 b, uint64 exp) {
		uint64 r = 1;
		while (exp > 0) {
			r *= b;
			exp--;
		}
		return r;
	}

	inline real64 pow_b_Uint64_exp_Int64(uint64 b, int64 exp) {
		real64 r = 1;

		if (exp >= 0) {
			while (exp > 0) {
				r *= b;
				exp--;
			}
			return r;
		}
		else {
			while (exp < 0) {
				r /= b;
				exp++;
			}
			return r;
		}
	}
}