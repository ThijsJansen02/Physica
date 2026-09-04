#pragma once
#include "RpConnection.h"

#include "RpGui.h"
#include <Engine/Engine.h>
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
		bool32 lowprecision = false;
		uint32 decimation;
	};

	inline real64 prewarp(real64 w0, real64 fs) {
		return 2 * fs * tan(w0 / (2 * fs));
	}


	inline void bilinearTransform(real64 acoefs[], real64 dcoefs[], real64 fs)
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

		az0 = 1.0;

		dcoefs[0] = bz0; dcoefs[1] = bz1; dcoefs[2] = bz2;
		dcoefs[3] = az0; dcoefs[4] = az1; dcoefs[5] = az2;
	}


	inline PH::int16 convertToFixedPoint(real64 in) {
		uint64 mult = 1 << 14;
		real64 result = in * mult;
		return (PH::int16)(result > 0.0f ? result + 0.5 : result - 0.5);
	}

	//should maybe floor or round the value correctly but I can do that another time
	inline PH::int32 convertToFixedPoint32(real64 in) {
		uint64 mult = 1 << 30;
		real64 result = in * mult;
		return (PH::int32)(result > 0.0f ? result + 0.5 : result - 0.5);
	}


	inline Base::Complex<real64> applyFilter(Base::Complex<real64> s, const BiQuadCoefficients& coeffs) {
		return (coeffs.b0 * s * s + coeffs.b1 * s + coeffs.b2) / (coeffs.a0 * s * s + coeffs.a1 * s + coeffs.a2);

	}

	inline BiQuadCoefficients getResonanceAntiResonanceBiquadCoefficientsContinuous(real64 cutoff, real64 Qfactor, real64 anticutoff, real64 antiQfactor) {


		BiQuadCoefficients result;
		result.b0 = 1.0f;
		result.b1 = cutoff / Qfactor;
		result.b2 = cutoff * cutoff;
		result.a0 = 1.0f;
		result.a1 = anticutoff / antiQfactor;
		result.a2 = anticutoff * anticutoff;
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
				return getResonanceAntiResonanceBiquadCoefficientsContinuous(filter.cutoff - (0.5f * filter.df), filter.Qfactor, filter.cutoff + (0.5f * filter.df), filter.antiQfactor);	
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

	inline char valToHex(uint16 val) {
		if (val >= 0 && val < 10) {
			return val + '0';
		}

		if (val >= 10 && val < 16) {
			return val + 'A' - 10;
		}

		return '0';
	}

	inline void writeHexVal32(uint32 coeff, char* buffer) {
		//clear the buffer
		for (uint32 i = 0; i < 9; i++) {
			buffer[i] = '\0';
		}
		buffer[7] = valToHex(coeff & 0x0000000F);
		buffer[6] = valToHex((coeff >> 4) & 0x0000000F);
		buffer[5] = valToHex((coeff >> 8) & 0x0000000F);
		buffer[4] = valToHex((coeff >> 12) & 0x0000000F);
		buffer[3] = valToHex((coeff >> 16) & 0x0000000F);
		buffer[2] = valToHex((coeff >> 20) & 0x0000000F);
		buffer[1] = valToHex((coeff >> 24) & 0x0000000F);
		buffer[0] = valToHex((coeff >> 28) & 0x0000000F);
	}

	inline void writeHexVal(uint16 coeff, char* buffer) {

		//clear the buffer
		for (uint32 i = 0; i < 5; i++) {
			buffer[i] = '\0';
		}

		buffer[3] = valToHex(coeff & 0x000F);
		buffer[2] = valToHex((coeff >> 4) & 0x000F);
		buffer[1] = valToHex((coeff >> 8) & 0x000F);
		buffer[0] = valToHex((coeff >> 12) & 0x000F);
	}

	inline Engine::String generateRpFilterString32(const BiQuadCoefficients& dcoeffs) {
		Base::Stream<Engine::Allocator> s = Base::Stream<Engine::Allocator>::create(100);

		int32 a1 = convertToFixedPoint32(dcoeffs.a1);
		int32 a2 = convertToFixedPoint32(dcoeffs.a2);

		int32 b0 = convertToFixedPoint32(dcoeffs.b0);
		int32 b1 = convertToFixedPoint32(dcoeffs.b1);
		int32 b2 = convertToFixedPoint32(dcoeffs.b2);

		s << "export PATH=$PATH:/opt/redpitaya/bin;";

		//first coeff at 0x41200000
		char buffer[9];
		s << " monitor 0x41200000 0x";
		writeHexVal32(b0, buffer);
		s << buffer;

		s << "; monitor 0x41200008 0x";
		writeHexVal32(b1, buffer);
		s << buffer;

		s << "; monitor 0x41210000 0x";
		writeHexVal32(b2, buffer);
		s << buffer;

		s << "; monitor 0x41210008 0x";
		writeHexVal32(a1, buffer);
		s << buffer;

		s << "; monitor 0x41220000 0x";
		writeHexVal32(a2, buffer);
		s << buffer;

		Engine::String result = s.createString<Engine::Allocator>();
		Base::Stream<Engine::Allocator>::destroy(&s);

		return result;
	}

	inline Engine::String generateRPfilterString(const BiQuadCoefficients& dcoeffs) {

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

	inline void recalculateFilter(Filter* filter) {
		filter->coeffs = calculateCoefficients(*filter);
	}

	inline void serializeFilter(const Filter& filter, YAML::Emitter& out) {
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

	inline Filter deserializeFilter(const YAML::Node& filter) {
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

	inline void serializeTransferFunction(const TransferFunction& function, YAML::Emitter& out) {

		out << YAML::BeginMap;
		out << YAML::Key << "name" << YAML::Value << function.name.getC_Str();
		out << YAML::Key << "remote" << YAML::Value << function.connection.remoteip.getC_Str();
		out << YAML::Key << "decimation" << YAML::Value << function.decimation;
		out << YAML::Key << "lowprecision" << YAML::Value << function.lowprecision;
		out << YAML::Key << "filters" << YAML::Value << YAML::BeginSeq;

		for (const auto& filter : function.filters) {
			serializeFilter(filter, out);
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;
	}

	inline TransferFunction deserializeTransferFunction(const YAML::Node& t) {
		TransferFunction result;
		result.currentcommand = Engine::String::create("");
		result.name = t["name"].as<Engine::String>();
		result.connection.remoteip = t["remote"].as<Engine::String>();
		result.filters = Engine::ArrayList<Filter>::create(1);

		if (t["lowprecision"]) {
			result.lowprecision = t["lowprecision"].as<bool32>();
		}
		else {
			result.lowprecision = false;
		}

		if (t["decimation"]) {
			result.decimation = (real64)t["decimation"].as<uint32>();
		}
		else {
			result.decimation = RpGui::standard_decimation;
		}

		for (auto& f : t["filters"]) {
			Filter f_ = deserializeFilter(f);
			result.filters.pushBack(f_);
		}

		return result;
	}


	inline void sentFilterToRp(Filter f, real64 targetfs, RpGui::RpConnection* connection, bool32 lowprecision) {

		//if the filter type is resonance anti resonance the cutoff is average between both resonances, so we need to calculate the cutoff differently
		//f.antiQfactor = prewarp(2 * M_PI * f.antiQfactor, targetfs);

		BiQuadCoefficients dcoeffs = {};

		if (f.type == FilterType::RESONANCE_ANTI_RESONANCE) {
			real64 cutoff1 = prewarp(2 * M_PI * (f.cutoff - (0.5f * f.df)), targetfs);
			real64 cutoff2 = prewarp(2 * M_PI * (f.cutoff + (0.5f * f.df)), targetfs);

			dcoeffs = bilinearTransform(getResonanceAntiResonanceBiquadCoefficientsContinuous(cutoff1, f.Qfactor, cutoff2, f.antiQfactor), targetfs);
		}
		else {
			f.cutoff = prewarp(2 * M_PI * (real64)f.cutoff, targetfs);
			dcoeffs = bilinearTransform(calculateCoefficients(f), targetfs);
		}
		Engine::String rpcommand;

		if (lowprecision) {
			rpcommand = generateRPfilterString(dcoeffs);
		}
		else {
			rpcommand = generateRpFilterString32(dcoeffs);
		}

		if (connection->open) {
			connection->commandqueue.push({ rpcommand });
			connection->commandqueue.push({ Engine::String::create("export PATH=$PATH:/opt/redpitaya/bin; monitor 0x41230000 1; sleep 0.001; monitor 0x41230000 0") });
			//connection->commandqueue.push({ Engine::String::create("monitor 0x41230000 0") });
			ReleaseSemaphore(connection->semaphore, 1, nullptr);
		}
	}
}
