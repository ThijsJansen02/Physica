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
	case NOTCH:
		coeffs.b[0] = 1;
		coeffs.b[1] = 2 * cutoffFrequency / qFactor;
		coeffs.b[2] = cutoffFrequency * cutoffFrequency;
		coeffs.a[0] = 1;
		coeffs.a[1] = 2 * pow(10.0, -gain/20.0) * cutoffFrequency / qFactor;
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


Parameters Filter::getParameters()
{
	Parameters parameters;

	switch (type)
	{
	case LOWPASS:
	case HIGHPASS:
		parameters.labels[0] = "Cutoff Frequency";
		parameters.labels[1] = "Q Factor";

		parameters.values[0] = &cutoffFrequency;
		parameters.values[1] = &qFactor;

		parameters.size = 2;

		break;
	case NOTCH:
		parameters.labels[0] = "Cutoff Frequency";
		parameters.labels[1] = "Q Factor";
		parameters.labels[2] = "Gain [dB]";

		parameters.values[0] = &cutoffFrequency;
		parameters.values[1] = &qFactor;
		parameters.values[2] = &gain;

		parameters.size = 3;

		break;
	case ALLPASS:
		parameters.size = 0;

		break;
	case RESONANCE_ANTI_RESONANCE:
		parameters.labels[0] = "Cutoff Frequency";
		parameters.labels[1] = "Q Factor";
		parameters.labels[0] = "Anti-Cutoff Frequency";
		parameters.labels[1] = "Anti-Q Factor";

		parameters.values[0] = &cutoffFrequency;
		parameters.values[1] = &qFactor;
		parameters.values[0] = &antiCutoffFrequency;
		parameters.values[1] = &antiQFactor;

		parameters.size = 4;

		break;
	case COEFFICIENTS:
		parameters.labels[0] = "b0";
		parameters.labels[1] = "b1";
		parameters.labels[2] = "b2";
		parameters.labels[3] = "a0";
		parameters.labels[4] = "a1";
		parameters.labels[5] = "a2";

		parameters.values[0] = &b0;
		parameters.values[1] = &b1;
		parameters.values[2] = &b2;
		parameters.values[3] = &a0;
		parameters.values[4] = &a1;
		parameters.values[5] = &a2;

		parameters.size = 6;

		break;
	};

	return parameters;
}

Filter Filter::deserialize(const YAML::Node& node)
{
	Filter filter;
	filter.type = static_cast<FilterType>(node["filter-type"].as<int>());
	filter.cutoffFrequency = node["cutoff"].as<real64>();
	filter.qFactor = node["qfactor"].as<real64>();
	filter.gain = node["gain"].as<real64>();
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
	out << YAML::Key << "gain" << YAML::Value << gain;
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
