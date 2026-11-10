#include "Filter.h"

void Filter::calculateCoefficients()
{
	switch (type)
	{
	case LOWPASS:
		coeffs.b[0] = 0;
		coeffs.b[1] = 0;
		coeffs.b[2] = cutoffFrequency * cutoffFrequency;
		coeffs.a[0] = 1;
		coeffs.a[1] = cutoffFrequency / qFactor;
		coeffs.a[2] = cutoffFrequency * cutoffFrequency;
		break;
	case BANDPASS:
		coeffs.a[0] = 1;
		coeffs.a[1] = cutoffFrequency / qFactor;
		coeffs.a[2] = cutoffFrequency * cutoffFrequency;
		coeffs.b[0] = 1;
		coeffs.b[1] = cutoffFrequency;
		coeffs.b[2] = cutoffFrequency * cutoffFrequency;
		break;
	case BANDSTOP:
		coeffs.b[0] = 1;
		coeffs.b[1] = cutoffFrequency / qFactor;
		coeffs.b[2] = cutoffFrequency * cutoffFrequency;
		coeffs.a[0] = 1;
		coeffs.a[1] = cutoffFrequency;
		coeffs.a[2] = cutoffFrequency * cutoffFrequency;
		break;
	case HIGHPASS:
		coeffs.b[0] = 1.0f;
		coeffs.b[1] = 0.0f;
		coeffs.b[2] = 0.0f;
		coeffs.a[0] = 1.0f;
		coeffs.a[1] = cutoffFrequency / qFactor;
		coeffs.a[2] = cutoffFrequency * cutoffFrequency;
		break;
	case ALLPASS:
		coeffs.b[0] = 0.0f;
		coeffs.b[1] = 0.0f;
		coeffs.b[2] = 1.0f;
		coeffs.a[0] = 0.0f;
		coeffs.a[1] = 0.0f;
		coeffs.a[2] = 1.0f;
		break;
	case RESONANCE_ANTI_RESONANCE:
		coeffs.b[0] = 1.0f;
		coeffs.b[1] = cutoffFrequency / qFactor;
		coeffs.b[2] = cutoffFrequency * cutoffFrequency;
		coeffs.a[0] = 1.0f;
		coeffs.a[1] = antiCutoffFrequency / antiQFactor;
		coeffs.a[2] = antiCutoffFrequency * antiCutoffFrequency;
		break;
	case COEFFICIENTS:
		coeffs.b[0] = b0;
		coeffs.b[1] = b1;
		coeffs.b[2] = b2;
		coeffs.a[0] = a0;
		coeffs.a[1] = a1;
		coeffs.a[2] = a2;
		break;
	};
}

const std::map<const char*, const real64*> Filter::getParameters() const
{
	std::map<const char*, const real64*> map;
	switch (type)
	{
	case LOWPASS:
	case BANDPASS:
	case BANDSTOP:
	case HIGHPASS:
		map.insert(std::pair("Cutoff Frequency", &cutoffFrequency));
		map.insert(std::pair("Q Factor", &qFactor));
		break;
	case ALLPASS:
		break;
	case RESONANCE_ANTI_RESONANCE:
		map.insert(std::pair("Cutoff Frequency", &cutoffFrequency));
		map.insert(std::pair("Q Factor", &qFactor));
		map.insert(std::pair("Anti-Cutoff Frequency", &antiCutoffFrequency));
		map.insert(std::pair("Anti-Q Factor", &antiQFactor));
		break;
	case COEFFICIENTS:
		map.insert(std::pair("b0", &b0));
		map.insert(std::pair("b1", &b1));
		map.insert(std::pair("b2", &b2));
		map.insert(std::pair("a0", &a0));
		map.insert(std::pair("a1", &a1));
		map.insert(std::pair("a2", &a2));
		break;
	};
	return map;
}

Filter Filter::deserialize(const YAML::Node& node)
{
	Filter filter;
	filter.type = static_cast<FilterType>(node["filter-type"].as<int>());
	filter.cutoffFrequency = node["cutoff"].as<real64>();
	filter.qFactor = node["qfactor"].as<real64>();
	filter.antiCutoffFrequency = node["anticutoff"].as<real64>();
	filter.antiQFactor = node["antiqfactor"].as<real64>();
	filter.b0 = node["b0"].as<real64>();
	filter.b1 = node["b1"].as<real64>();
	filter.b2 = node["b2"].as<real64>();
	filter.a0 = node["a0"].as<real64>();
	filter.a1 = node["a1"].as<real64>();
	filter.a2 = node["a2"].as<real64>();

	filter.calculateCoefficients();
	return filter;
}

void Filter::serialize(YAML::Emitter& out) const
{
	out << YAML::BeginMap;

	out << YAML::Key << "filter-type" << YAML::Value << static_cast<int>(type);

	out << YAML::Key << "cutoff" << YAML::Value << cutoffFrequency;
	out << YAML::Key << "qfactor" << YAML::Value << qFactor;
	out << YAML::Key << "anticutoff" << YAML::Value << antiCutoffFrequency;
	out << YAML::Key << "antiqfactor" << YAML::Value << antiQFactor;
	out << YAML::Key << "b0" << YAML::Value << b0;
	out << YAML::Key << "b1" << YAML::Value << b1;
	out << YAML::Key << "b2" << YAML::Value << b2;
	out << YAML::Key << "a0" << YAML::Value << a0;
	out << YAML::Key << "a1" << YAML::Value << a1;
	out << YAML::Key << "a2" << YAML::Value << a2;

	out << YAML::Key << "_b0" << YAML::Value << coeffs.b[0];
	out << YAML::Key << "_b1" << YAML::Value << coeffs.b[1];
	out << YAML::Key << "_b2" << YAML::Value << coeffs.b[2];

	out << YAML::Key << "_a0" << YAML::Value << coeffs.a[0];
	out << YAML::Key << "_a1" << YAML::Value << coeffs.a[1];
	out << YAML::Key << "_a2" << YAML::Value << coeffs.a[2];

	out << YAML::EndMap;
}
