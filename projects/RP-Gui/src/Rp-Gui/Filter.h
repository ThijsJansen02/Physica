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
};

// Stores arrays of displayed labels with corresponding value pointer
struct Parameters {
	const char* labels[6];
	const real64* valuePtrs[6];

	const int totalParams;

	Parameters(const char* l[], const real64* v[], const int t)
		: totalParams(t)
	{
		for (size_t i = 0; i < t; i++)
		{
			labels[i] = l[i];
			valuePtrs[i] = v[i];
		}
	}
};

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
	"Low-Pass",
	"Band-Pass",
	"Band-Stop",
	"High-Pass",
	"All-Pass",
	"Resonance Anti-Resonance",
	"Coefficients"
};

class Filter {
public:
	const char* typeStr() const { return FilterTypeStrings[type]; };

	// Calculate biquad coefficients from specific parametrization
	void calculateCoefficients();
	inline const BiQuadCoefficients& getBiquadCoefficients() const { return coeffs; };
	inline const BiQuadCoefficients& getInvertedCoefficients() const { return { coeffs.a[0], coeffs.a[1], coeffs.a[2], coeffs.b[0], coeffs.b[1], coeffs.b[2] }; };

	const Parameters getParameters() const;

	static Filter deserialize(const YAML::Node& filter);
	void serialize(YAML::Emitter& out) const;

	real64 cutoffFrequency = 1e3;
	real64 qFactor = 0.1f;
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