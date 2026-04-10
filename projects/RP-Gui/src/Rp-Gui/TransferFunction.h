#pragma once
#include <Engine/Engine.h>
#include "RpConnection.h"

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

	inline real32 bandpass(real32 x, void* params_) {

		Filter* params = (Filter*)params_;
		return (x * x + params->cutoff * params->cutoff) / (x * x + ((x * params->cutoff) / params->Qfactor) + params->cutoff * params->cutoff);
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
