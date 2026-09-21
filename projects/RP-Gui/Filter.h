#pragma once
#include <Base/Basetypes.h>
#include <Engine/AssetLibrary.h>
#include <Base/Datastructures/String.h>

using namespace PH;

struct BiQuadCoefficients {
	real64 b[3];
	real64 a[3];
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
	virtual const FilterType type() const = 0;

	// Calculate biquad coefficients from specific parametrization
	virtual void calculateCoefficients() = 0;
	inline const BiQuadCoefficients getBiquadCoefficients() const { return coeffs; };

	static Filter* deserialize(const YAML::Node& filter);
	void serialize(YAML::Emitter& out) const;

	// Returns new instance of filter of specific filter type
	static Filter* getType(FilterType type);

	// Holds parameters such as Q factor, cutoff frequency
	std::map<const char*, real64*> parameters;

protected:
	// Not bilinear transformed
	BiQuadCoefficients coeffs = { {0,0,1}, {0,0,0} };

	void addParameter(const char* name, real64* valuePtr) { parameters.insert(std::pair(name, valuePtr)); };
};

class LowPassFilter : public Filter {
public:
	LowPassFilter()
	{
		addParameter("Cutoff Frequency", &cutoff);
		addParameter("Q Factor", &qfactor);
	};

	real64 cutoff = 10e3;
	real64 qfactor = 1.0f;

	const FilterType type() const override { return FilterType::LOWPASS; };
	void calculateCoefficients() override;
};

class BandPassFilter : public Filter {
public:
	BandPassFilter()
	{
		addParameter("Cutoff Frequency", &cutoff);
		addParameter("Q Factor", &qfactor);
	};

	real64 cutoff = 10e3;
	real64 qfactor = 1.0f;

	const FilterType type() const override { return FilterType::BANDPASS; };
	void calculateCoefficients() override;
};

class BandStopFilter : public Filter {
public:
	BandStopFilter()
	{
		addParameter("Cutoff Frequency", &cutoff);
		addParameter("Q Factor", &qfactor);
	};

	real64 cutoff = 10e3;
	real64 qfactor = 1.0f;

	const FilterType type() const override { return FilterType::BANDSTOP; };
	void calculateCoefficients() override;
};

class HighPassFilter : public Filter {
public:
	HighPassFilter()
	{
		addParameter("Cutoff Frequency", &cutoff);
		addParameter("Q Factor", &qfactor);
	};

	real64 cutoff = 10e3;
	real64 qfactor = 1.0f;

	const FilterType type() const override { return FilterType::HIGHPASS; };
	void calculateCoefficients() override;
};

class AllPassFilter : public Filter {
public:
	const FilterType type() const override { return FilterType::ALLPASS; };
	void calculateCoefficients() override;
};

class ResAntiResFilter : public Filter {
public:
	ResAntiResFilter()
	{
		addParameter("Cutoff Frequency", &cutoff);
		addParameter("Q Factor", &qfactor);
		addParameter("Anti-Cutoff Frequency", &anticutoff);
		addParameter("Anti-Q Factor", &antiqfactor);
	};

	real64 cutoff = 10e3;
	real64 qfactor = 1.0f;
	real64 anticutoff = 1.0f;
	real64 antiqfactor = 1.0f;

	const FilterType type() const override { return FilterType::RESONANCE_ANTI_RESONANCE; };
	void calculateCoefficients() override;
};

class CoefficientsFilter : public Filter {
public:
	CoefficientsFilter()
	{
		addParameter("b0", &b0);
		addParameter("b1", &b1);
		addParameter("b2", &b2);
		addParameter("a0", &a0);
		addParameter("a1", &a1);
		addParameter("a2", &a2);
	};

	real64 b0;
	real64 b1;
	real64 b2;
	real64 a0;
	real64 a1;
	real64 a2;

	const FilterType type() const override { return FilterType::COEFFICIENTS; };
	void calculateCoefficients() override;
};
