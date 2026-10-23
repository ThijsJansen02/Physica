#pragma once
#include <Base/Basetypes.h>
#include <Engine/AssetLibrary.h>
#include <Base/Datastructures/String.h>

#include <map>
#include <yaml-cpp/yaml.h>

using namespace PH;

struct BiQuadCoefficients {
	real64 b[3];
	real64 a[3];

	BiQuadCoefficients& operator*=(const real64 scalar)
	{
		b[0] *= scalar;
		b[1] *= scalar;
		b[2] *= scalar;

		return *this;
	}
};

enum FilterType {
	LOWPASS,
	NOTCH,
	HIGHPASS,
	ALLPASS,
	RESONANCE_ANTI_RESONANCE,
	COEFFICIENTS
};

static const char* FilterTypeStrings[] = {
	"Low-Pass",
	"Notch",
	"High-Pass",
	"All-Pass",
	"Resonance Anti-Resonance",
	"Coefficients"
};

struct Parameters;
class Filter {
public:
	const char* typeStr() const { return FilterTypeStrings[type]; };

	// Calculate biquad coefficients from specific parametrization
	void calculateCoefficients();
	inline const BiQuadCoefficients& getBiquadCoefficients() const { return coeffs; };
	inline const BiQuadCoefficients& getInvertedCoefficients() const { return { coeffs.a[0], coeffs.a[1], coeffs.a[2], coeffs.b[0], coeffs.b[1], coeffs.b[2] }; };

	Parameters getParameters();
	static constexpr int maxParameters() { return 6; };

	static Filter deserialize(const YAML::Node& filter);
	void serialize(YAML::Emitter& out) const;

	real64 cutoffFrequency = 1e3;
	real64 qFactor = 0.1f;
	real64 gain = 0.0f; // in decibels
	real64 antiCutoffFrequency = 800.0f;
	real64 antiQFactor = 10.0f;
	real64 b0 = 1;
	real64 b1 = 1;
	real64 b2 = 1;
	real64 a0 = 1;
	real64 a1 = 1;
	real64 a2 = 1;

	FilterType type = ALLPASS;

	BiQuadCoefficients coeffs = { {0,0,1}, {0,0,1} };
};

struct Parameters {
	const char* labels[Filter::maxParameters()];
	real64* values[Filter::maxParameters()];

	//Parameters(const char** l, real64** v, int n)
	//	: size(n)
	//{
	//	for (size_t i = 0; i < n; i++)
	//	{
	//		labels[i] = l[i];
	//		values[i] = v[i];
	//	}
	//}

	int size;
};