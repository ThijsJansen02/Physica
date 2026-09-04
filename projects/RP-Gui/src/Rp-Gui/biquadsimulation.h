#pragma once
#include <Engine/Engine.h>
#include "TransferFunction.h"

namespace PH::RpGui {
	struct BiquadRealCoefs {
		real32 b0;
		real32 b1;
		real32 b2;

		real32 a1;
		real32 a2;

		real64 zx0;
		real64 zx1;
		real64 zx2;

		real64 zy1;
		real64 zy2;

	};

	struct Biquad {
		int32 b0;
		int32 b1;
		int32 b2;

		int32 a1;
		int32 a2;

		int64 zx0;
		int64 zx1;
		int64 zx2;

		int64 zy1;
		int64 zy2;
	};

	BiquadRealCoefs createBiquadRealCoefs(BiQuadCoefficients coeffs) {

		BiquadRealCoefs bq;
		bq.b0 = (real32)coeffs.b0;
		bq.b1 = (real32)coeffs.b1;
		bq.b2 = (real32)coeffs.b2;
		bq.a1 = (real32)coeffs.a1;
		bq.a2 = (real32)coeffs.a2;

		bq.zx0 = 0.0f;
		bq.zx1 = 0.0f;
		bq.zx2 = 0.0f;
		bq.zy1 = 0.0f;
		bq.zy2 = 0.0f;
		return bq;
	}

	Biquad createBiquad(BiQuadCoefficients coeffs) {

		Biquad bq;

		bq.b0 = RpGui::convertToFixedPoint32(coeffs.b0);
		bq.b1 = RpGui::convertToFixedPoint32(coeffs.b1);
		bq.b2 = RpGui::convertToFixedPoint32(coeffs.b2);

		bq.a1 = RpGui::convertToFixedPoint32(coeffs.a1);
		bq.a2 = RpGui::convertToFixedPoint32(coeffs.a2);

		bq.zx0 = 0;
		bq.zx1 = 0;
		bq.zx2 = 0;

		bq.zy1 = 0;
		bq.zy2 = 0;

		return bq;
	}





	int16 applyBiquadRealCoefs(BiquadRealCoefs& bq, int16 input) {
		real64 acc = (real64)bq.b0 * input + (real64)bq.b1 * bq.zx1 + (real64)bq.b2 * bq.zx2 - (real64)bq.a1 * bq.zy1 - (real64)bq.a2 * bq.zy2;
		acc = acc;
		//clipping
		if (acc > 32767) {
			acc = 32767;
		}
		else if (acc < -32768) {
			acc = -32768;
		}
		//update the state
		bq.zx2 = bq.zx1;
		bq.zx1 = (real64)input;
		bq.zy2 = bq.zy1;
		bq.zy1 = acc;
		return (int16)acc;
	}



	int16 applyBiquad(Biquad& bq, int16 input) {

		int64 acc = (int64)bq.b0 * input + (int64)bq.b1 * bq.zx1 + (int64)bq.b2 * bq.zx2 - (int64)bq.a1 * (bq.zy1 >> 30) - (int64)bq.a2 * (bq.zy2 >> 30);
		acc = acc;

		/*
		//clipping
		if (acc > 32767) {
			acc = 32767;
		}

		else if (acc < -32768) {
			acc = -32768;
		}
		*/


		//update the state
		bq.zx2 = bq.zx1;
		bq.zx1 = input;
		bq.zy2 = bq.zy1;
		bq.zy1 = (int64)acc;
		return (int16)(acc >> 30);
	}

	void writeCSV(const Engine::DynamicArray<int16>& input, const Engine::DynamicArray<int16>& output, const char* filename);

#define N_SAMPLES 10000000

	void runBiquadSim() {

		Engine::DynamicArray<int16> testdata = Engine::DynamicArray<int16>::create(N_SAMPLES);
		Engine::DynamicArray<int16> output = Engine::DynamicArray<int16>::create(N_SAMPLES);

		for (uint64 i = 0; i < N_SAMPLES; i++) {

			testdata[i] = 0;
		}

		testdata[0] = 32767;

		RpGui::Filter examplefilter{};
		examplefilter.type = FilterType::BANDSTOP;
		examplefilter.cutoff = 5000.0f;
		examplefilter.Qfactor = 3.6f;
		examplefilter.gain = 1.0f;

		examplefilter.cutoff = prewarp(2 * M_PI * examplefilter.cutoff, RP_FPGA_SAMPLERATE);

		BiQuadCoefficients coeffs = RpGui::bilinearTransform(RpGui::calculateCoefficients(examplefilter), RP_FPGA_SAMPLERATE);

		Biquad bq = createBiquad(coeffs);

		INFO << "b0: " << coeffs.b0 << ", " << bq.b0 << "\n";
		INFO << "b1: " << coeffs.b1 << ", " << bq.b1 << "\n";
		INFO << "b2: " << coeffs.b2 << ", " << bq.b2 << "\n";

		INFO << "a0: " << coeffs.a0 << ", " << "" << "\n";
		INFO << "a1: " << coeffs.a1 << ", " << bq.a1 << "\n";
		INFO << "a2: " << coeffs.a2 << ", " << bq.a2 << "\n";


		BiquadRealCoefs b = createBiquadRealCoefs(coeffs);

		for (uint64 i = 0; i < N_SAMPLES; i++) {
			output[i] = applyBiquad(bq, testdata[i]);
		}

		writeCSV(testdata, output, "testdata.csv");

		Engine::DynamicArray<int16>::destroy(&testdata);
		Engine::DynamicArray<int16>::destroy(&output);
	}
}