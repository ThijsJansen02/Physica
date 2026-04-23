#pragma once
#include <Engine/Engine.h>
#include "RpConnection.h"
#include <Base/Math/Complex.h>
#include <Base/Datastructures/Stream.h>

namespace PH::RpGui {

	struct BiQuadCoefficients {
		union {
			struct {
				real64 b0;
				real64 b1;
				real64 b2;

				real64 a0;
				real64 a1;
				real64 a2;

			};

			struct {
				real64 b[3];
				real64 a[3];
			};
		};
	};

#define FILTER_TYPE_COUNT 7

	enum FilterType {
		LOWPASS,
		BANDPASS,
		BANDSTOP,
		HIGHPASS,
		ALLPASS,
		RESONANCE_ANTI_RESONANCE,
		COEFFICIENTS
	};

	static const char* FilterTypeStrings[] = {
		"lowpass",
		"bandpass",
		"bandstop",
		"highpass",
		"allpass",
		"resonance anti resonance",
		"coefficients"
	};

	struct Filter {
		FilterType type;

		real32 cutoff;
		real32 Qfactor;

		union {
			real32 gain;
			struct {
				real32 df;
				real32 antiQfactor;
			};
		};

		BiQuadCoefficients coeffs;
	};


	struct TransferFunction {
		Engine::String name;
		Engine::String currentcommand;
		RpConnection connection;
		Engine::ArrayList<Filter> filters;
	};

	real64 prewarp(real64 w0, real64 fs) {
		return 2 * fs * tan(w0 / (2 * fs));
	}

	void bilinearTransform(real64 acoefs[], real64 dcoefs[], real64 fs)
	{
		double b0, b1, b2, a0, a1, a2;
		double bz0, bz1, bz2, az0, az1, az2;

		b0 = acoefs[0]; b1 = acoefs[1]; b2 = acoefs[2];
		a0 = acoefs[3]; a1 = acoefs[4]; a2 = acoefs[5];

		real64 T = 1 / fs;
		real64 K = 2 / T;
		real64 Ks = K * K;

		az0 = a0 * Ks + a1 * K + a2;

		bz0 = ((b0 * Ks) + (b1 * K) + b2) / az0;
		bz1 = (2 * b2 - (2 * b0 * Ks)) / az0;
		bz2 = ((b0 * Ks) - (b1 * K) + b2) / az0;

		az1 = ((2 * a2) - (2 * a0 * Ks)) / az0;
		az2 = ((a0 * Ks) - (a1 * K) + a2) / az0;

		az0 = 1.0f;

		dcoefs[0] = bz0; dcoefs[1] = bz1; dcoefs[2] = bz2;
		dcoefs[3] = az0; dcoefs[4] = az1; dcoefs[5] = az2;
	}

	inline PH::int16 convertToFixedPoint(real64 in) {
		uint64 mult = 1 << 14;
		int64 result = in * mult;
		return (PH::int16)result;
	}


	inline Base::Complex<real32> bandpass(Base::Complex<real32> s, void* params_) {

		Filter* params = (Filter*)params_;
		return (params->cutoff * params->cutoff + (params->cutoff / params->Qfactor) * s + s * s) / (params->cutoff * params->cutoff + (params->cutoff) * s + s * s);
	}

	inline Base::Complex<real64> applyFilter(Base::Complex<real64> s, const BiQuadCoefficients& coeffs) {
		return (coeffs.b0 * s * s + coeffs.b1 * s + coeffs.b2) / (coeffs.a0 * s * s + coeffs.a1 * s + coeffs.a2);

	}

	inline BiQuadCoefficients getResonanceAntiResonanceBiquadCoefficientsContinuous(real64 cutoff, real64 Qfactor, real64 df, real64 antiQfactor) {

		cutoff = cutoff - (0.5 * df);

		real64 antiCutoff = cutoff + (0.5 * df);

		BiQuadCoefficients result;
		result.b0 = 1.0f;
		result.b1 = cutoff / Qfactor;
		result.b2 = cutoff * cutoff;
		result.a0 = 1.0f;
		result.a1 = antiCutoff / antiQfactor;
		result.a2 = antiCutoff * antiCutoff;
		return result;
	}

	inline BiQuadCoefficients getlowPassBiquadCoefficientsContinuous(real64 cutoff, real64 Qfactor) {
		
		BiQuadCoefficients result;
		result.b0 = 0;
		result.b1 = 0;
		result.b2 = cutoff * cutoff;
		result.a0 = 1;
		result.a1 = cutoff / Qfactor;
		result.a2 = cutoff * cutoff;
		return result;
	}

	inline BiQuadCoefficients getBandPassBiquadCoefficientsContinuous(real64 cutoff, real64 Qfactor) {
		
		BiQuadCoefficients result;
		result.a0 = 1;
		result.a1 = cutoff / Qfactor;
		result.a2 = cutoff * cutoff;

		result.b0 = 1;
		result.b1 = cutoff;
		result.b2 = cutoff * cutoff;

		return result;
	}

	inline BiQuadCoefficients getBandStopBiquadCoefficientsContinuous(real64 cutoff, real64 Qfactor) {

		BiQuadCoefficients result;
		result.b0 = 1;
		result.b1 = cutoff / Qfactor;
		result.b2 = cutoff * cutoff;

		result.a0 = 1;
		result.a1 = cutoff;
		result.a2 = cutoff * cutoff;

		return result;
	}

	inline BiQuadCoefficients getHighPassBiquadCoefficientsContinuous(real64 cutoff, real64 Qfactor) {

		BiQuadCoefficients result;
		result.b0 = 1.0f;
		result.b1 = 0.0f;
		result.b2 = 0.0f;

		result.a0 = 1;
		result.a1 = cutoff / Qfactor;
		result.a2 = cutoff * cutoff;

		return result;
	}

	inline BiQuadCoefficients getAllpassBiquadCoefficientsContinuous(real64 cutoff, real64 Qfactor) {
		BiQuadCoefficients result;
		result.b0 = 0.0f;
		result.b1 = 0.0f;
		result.b2 = 1.0f;
		result.a0 = 0.0f;
		result.a1 = 0.0f;
		result.a2 = 1.0f;
		return result;
	}

	inline BiQuadCoefficients calculateCoefficients(const Filter& filter) {
		switch (filter.type) {
			case FilterType::LOWPASS:
				return getlowPassBiquadCoefficientsContinuous(filter.cutoff, filter.Qfactor);
			case FilterType::BANDPASS:
				return getBandPassBiquadCoefficientsContinuous(filter.cutoff, filter.Qfactor);
			case FilterType::BANDSTOP:
				return getBandStopBiquadCoefficientsContinuous(filter.cutoff, filter.Qfactor);
			case FilterType::HIGHPASS:
				return getHighPassBiquadCoefficientsContinuous(filter.cutoff, filter.Qfactor);
			case FilterType::RESONANCE_ANTI_RESONANCE:
				return getResonanceAntiResonanceBiquadCoefficientsContinuous(filter.cutoff, filter.Qfactor, filter.df, filter.antiQfactor);	
			case FilterType::ALLPASS:
				return getAllpassBiquadCoefficientsContinuous(filter.cutoff, filter.Qfactor);
			case FilterType::COEFFICIENTS:
				return filter.coeffs;
		}
	}

	inline BiQuadCoefficients bilinearTransform(BiQuadCoefficients continuous, real64 targetfs) {
		BiQuadCoefficients result;
		bilinearTransform(continuous.b, result.b, targetfs);
		return result;
	}

	char valToHex(uint16 val) {
		if (val >= 0 && val < 10) {
			return val + '0';
		}

		if (val >= 10 && val < 16) {
			return val + 'A' - 10;
		}

		return '0';
	}


	void writeHexVal(uint16 coeff, char* buffer) {

		//clear the buffer
		for (uint32 i = 0; i < 5; i++) {
			buffer[i] = '\0';
		}

		buffer[3] = valToHex(coeff & 0x000F);
		buffer[2] = valToHex((coeff >> 4) & 0x000F);
		buffer[1] = valToHex((coeff >> 8) & 0x000F);
		buffer[0] = valToHex((coeff >> 12) & 0x000F);
	}

	Engine::String generateRPfilterString(const BiQuadCoefficients& dcoeffs) {

		Base::Stream<Engine::Allocator> s = Base::Stream<Engine::Allocator>::create(100);
		

		int16 a1 = convertToFixedPoint(dcoeffs.a1);
		int16 a2 = convertToFixedPoint(dcoeffs.a2);

		int16 b0 = convertToFixedPoint(dcoeffs.b0);
		int16 b1 = convertToFixedPoint(dcoeffs.b1);
		int16 b2 = convertToFixedPoint(dcoeffs.b2);

		s << "export PATH=$PATH:/opt/redpitaya/bin;";

		//first 2 coeffs at 0x41200000
		s << "monitor 0x41200000 0x";
		char buffer[5];
		writeHexVal((uint16)a1, buffer);
		s << buffer;

		writeHexVal((uint16)a2, buffer);
		s << buffer;

		//next 2 coeffs at 0x41200008
		s << "; monitor 0x41200008 0x";
		writeHexVal((uint16)b0, buffer);
		s << buffer;

		writeHexVal((uint16)b1, buffer);
		s << buffer;
		

		//last coeff at 0x41210000
		s << "; monitor 0x41210000 0x";
		writeHexVal((uint16)b2, buffer);
		s << buffer;
		s << "0000";

		Engine::String result = s.createString<Engine::Allocator>();
		Base::Stream<Engine::Allocator>::destroy(&s);
		return result;
	}

	void recalculateFilter(Filter* filter) {
		filter->coeffs = calculateCoefficients(*filter);
	}

	void serializeFilter(const Filter& filter, YAML::Emitter& out) {
		out << YAML::BeginMap;
		out << YAML::Key << "cutoff" << YAML::Value << filter.cutoff;
		out << YAML::Key << "Qfactor" << YAML::Value << filter.Qfactor;
		if (filter.type == FilterType::RESONANCE_ANTI_RESONANCE) {
			out << YAML::Key << "df" << YAML::Value << filter.df;
			out << YAML::Key << "antiQfactor" << YAML::Value << filter.antiQfactor;
		}
		else {
			out << YAML::Key << "gain" << YAML::Value << filter.gain;
		}

		out << YAML::Key << "FilterType" << YAML::Value << (int)filter.type;
		out << YAML::EndMap;
	}

	Filter deserializeFilter(const YAML::Node& filter) {
		Filter result;
		result.cutoff = filter["cutoff"].as<real32>();
		result.Qfactor = filter["Qfactor"].as<real32>();
		result.type = (FilterType)filter["FilterType"].as<int>();
		if (result.type == FilterType::RESONANCE_ANTI_RESONANCE) {
			result.df = filter["df"].as<real32>();
			result.antiQfactor = filter["antiQfactor"].as<real32>();
		}
		else {
			result.gain = filter["gain"].as<real32>();
		}


		recalculateFilter(&result);
		return result;
	}

	void serializeTransferFunction(const TransferFunction& function, YAML::Emitter& out) {

		out << YAML::BeginMap;
		out << YAML::Key << "name" << YAML::Value << function.name.getC_Str();
		out << YAML::Key << "remote" << YAML::Value << function.connection.remoteip.getC_Str();
		out << YAML::Key << "filters" << YAML::Value << YAML::BeginSeq;

		for (const auto& filter : function.filters) {
			serializeFilter(filter, out);
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;
	}

	TransferFunction deserializeTransferFunction(const YAML::Node& t) {
		TransferFunction result;
		result.currentcommand = Engine::String::create("");
		result.name = t["name"].as<Engine::String>();
		result.connection.remoteip = t["remote"].as<Engine::String>();
		result.filters = Engine::ArrayList<Filter>::create(1);

		for (auto& f : t["filters"]) {
			Filter f_ = deserializeFilter(f);
			result.filters.pushBack(f_);
		}

		return result;
	}

}
