#include "Filter.h"

void LowPassFilter::calculateCoefficients()
{
	coeffs.b[0] = 0;
	coeffs.b[1] = 0;
	coeffs.b[2] = cutoff * cutoff;
	coeffs.a[0] = 1;
	coeffs.a[1] = cutoff / qfactor;
	coeffs.a[2] = cutoff * cutoff;
}

void BandPassFilter::calculateCoefficients()
{
	coeffs.a[0] = 1;
	coeffs.a[1] = cutoff / qfactor;
	coeffs.a[2] = cutoff * cutoff;
	coeffs.b[0] = 1;
	coeffs.b[1] = cutoff;
	coeffs.b[2] = cutoff * cutoff;
}

void BandStopFilter::calculateCoefficients()
{
	coeffs.b[0] = 1;
	coeffs.b[1] = cutoff / qfactor;
	coeffs.b[2] = cutoff * cutoff;
	coeffs.a[0] = 1;
	coeffs.a[1] = cutoff;
	coeffs.a[2] = cutoff * cutoff;
}

void HighPassFilter::calculateCoefficients()
{
	coeffs.b[0] = 1.0f;
	coeffs.b[1] = 0.0f;
	coeffs.b[2] = 0.0f;
	coeffs.a[0] = 1.0f;
	coeffs.a[1] = cutoff / qfactor;
	coeffs.a[2] = cutoff * cutoff;
}

void AllPassFilter::calculateCoefficients()
{
	coeffs.b[0] = 0.0f;
	coeffs.b[1] = 0.0f;
	coeffs.b[2] = 1.0f;
	coeffs.a[0] = 0.0f;
	coeffs.a[1] = 0.0f;
	coeffs.a[2] = 0.0f;
}

void ResAntiResFilter::calculateCoefficients() {
	coeffs.b[0] = 1.0f;
	coeffs.b[1] = cutoff / qfactor;
	coeffs.b[2] = cutoff * cutoff;
	coeffs.a[0] = 1.0f;
	coeffs.a[1] = anticutoff / antiqfactor;
	coeffs.a[2] = anticutoff * anticutoff;
}

void CoefficientsFilter::calculateCoefficients()
{
	coeffs.b[0] = b0;
	coeffs.b[1] = b1;
	coeffs.b[2] = b2;
	coeffs.a[0] = a0;
	coeffs.a[1] = a1;
	coeffs.a[2] = a2;
}

Filter* Filter::getType(FilterType type)
{
	switch (type)
	{
	case FilterType::LOWPASS:
		return new LowPassFilter();
	case FilterType::BANDPASS:
		return new BandPassFilter();
	case FilterType::HIGHPASS:
		return new HighPassFilter();
	case FilterType::ALLPASS:
		return new AllPassFilter();
	case FilterType::RESONANCE_ANTI_RESONANCE:
		return new ResAntiResFilter();
	case FilterType::COEFFICIENTS:
		return new CoefficientsFilter();
	default:
		return new AllPassFilter();
	};
}

Filter* Filter::deserialize(const YAML::Node& node)
{
	Filter* filter = getType(static_cast<FilterType>(node["filter-type"].as<int>()));

	for (const auto& parameter : filter->parameters)
	{
		*parameter.second = node[parameter.first].as<real64>();
	}
	return filter;
}

void Filter::serialize(YAML::Emitter& out) const
{
	out << YAML::BeginMap;

	out << YAML::Key << "filter-type" << YAML::Value << static_cast<int>(type());

	for (const auto& pair : parameters)
	{
		out << YAML::Key << pair.first << YAML::Value << *pair.second;
	}

	out << YAML::Key << "b0" << YAML::Value << coeffs.b[0];
	out << YAML::Key << "b1" << YAML::Value << coeffs.b[1];
	out << YAML::Key << "b2" << YAML::Value << coeffs.b[2];

	out << YAML::Key << "a0" << YAML::Value << coeffs.a[0];
	out << YAML::Key << "a1" << YAML::Value << coeffs.a[1];
	out << YAML::Key << "a2" << YAML::Value << coeffs.a[2];

	out << YAML::EndMap;
}
