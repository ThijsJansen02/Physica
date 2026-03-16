#pragma once

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
}