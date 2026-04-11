#pragma once
#include <Engine/Engine.h>
#include "RpConnection.h"
#include <Base/Math/Complex.h>

namespace PH::RpGui {

	struct Filter {
		real32 cutoff;
		real32 Qfactor;
		real32 gain;
	};


	struct TransferFunction {
		Engine::String name;
		RpConnection connection;
		Engine::ArrayList<Filter> filters;
	};


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


	inline Base::Complex<real32> bandpass(Base::Complex<real32> s, void* params_) {

		Filter* params = (Filter*)params_;
		return (params->cutoff * params->cutoff + (params->cutoff / params->Qfactor) * s + s * s) / (params->cutoff * params->cutoff + (params->cutoff) * s + s * s);
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
