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

	struct Filter {
		real32 cutoff;
		real32 Qfactor;
		real32 gain;

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


	//copied from: https://gist.github.com/felakuti4life/34b20a2517d63baa00b9
	void bilinearTransform(real64 acoefs[], real64 dcoefs[], real64 fs)
	{
		double b0, b1, b2, a0, a1, a2;
		double bz0, bz1, bz2, az0, az1, az2;

		b0 = acoefs[0]; b1 = acoefs[1]; b2 = acoefs[2];
		a0 = acoefs[3]; a1 = acoefs[4]; a2 = acoefs[5];


		//where we're pinning to
		real32 c = 2 * fs;
		real32 c_sq = powf(c, 2);

		//apply bilinear transform
		az0 = (a2 * c_sq + a1 * c + a0);

		bz0 = (b2 * c_sq + b1 * c + b0) / az0;
		bz1 = (-2 * b2 * c_sq + 2 * b0) / az0;
		bz2 = (b2 * c_sq - b1 * c + b0) / az0;


		az1 = (-2 * a2 * c_sq + 2 * a0) / az0;
		az2 = (a2 * c_sq - a1 * c + a0) / az0;
		az0 = 1.0f;
		dcoefs[0] = bz0; dcoefs[1] = bz1; dcoefs[2] = bz2;
		dcoefs[3] = az1; dcoefs[4] = az2;

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

	inline BiQuadCoefficients getBandPassBiquadCoefficientsContinuous(void* params_) {
		Filter* params = (Filter*)params_;

		BiQuadCoefficients result;
		result.b0 = 1;
		result.b1 = params->cutoff / params->Qfactor;
		result.b2 = params->cutoff * params->cutoff;

		result.a0 = 1;
		result.a1 = params->cutoff;
		result.a2 = params->cutoff * params->cutoff;

		return result;
	}

	inline BiQuadCoefficients bilinearTransform(BiQuadCoefficients continuous, real64 targetfs) {
		BiQuadCoefficients result;
		bilinearTransform(continuous.b, result.b, targetfs);
		return result;
	}


	void writeHexVal(uint16 coeff, char* buffer) {

		//clear the buffer
		for (uint32 i = 0; i < 5; i++) {
			buffer[i] = '\0';
		}

		buffer[3] = coeff & 0x000F;
		buffer[2] = (coeff >> 4) & 0x000F;
		buffer[1] = (coeff >> 8) & 0x000F;
		buffer[0] = (coeff >> 12) & 0x000F;
	}

	Engine::String generateRPfilterString(const BiQuadCoefficients& dcoeffs) {

		Base::Stream<Engine::Allocator> s = Base::Stream<Engine::Allocator>::create(100);
		
		//first 2 coeffs at 0x41200000
		s << "monitor 0x41200000 0x";
		char buffer[5];
		writeHexVal((uint16)dcoeffs.a1, buffer);
		s << buffer;

		writeHexVal((uint16)dcoeffs.a2, buffer);
		s << buffer;

		//next 2 coeffs at 0x41200008
		s << "; monitor 0x41200008 0x";
		writeHexVal((uint16)dcoeffs.b0, buffer);
		s << buffer;

		writeHexVal((uint16)dcoeffs.b1, buffer);
		s << buffer;
		

		//last coeff at 0x41210000
		s << "; monitor 0x41210000 0x";
		writeHexVal((uint16)dcoeffs.b2, buffer);
		s << "0000";

		return s.getString<Engine::Allocator>();


	}

	void recalculateFilter(Filter* filter) {
		filter->coeffs = getBandPassBiquadCoefficientsContinuous(filter);
	}

	void serializeFilter(const Filter& filter, YAML::Emitter& out) {
		out << YAML::BeginMap;
		out << YAML::Key << "cutoff" << YAML::Value << filter.cutoff;
		out << YAML::Key << "Qfactor" << YAML::Value << filter.Qfactor;
		out << YAML::Key << "gain" << YAML::Value << filter.gain;
		out << YAML::EndMap;
	}

	Filter deserializeFilter(const YAML::Node& filter) {
		Filter result;
		result.cutoff = filter["cutoff"].as<real32>();
		result.Qfactor = filter["Qfactor"].as<real32>();
		result.gain = filter["gain"].as<real32>();

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
