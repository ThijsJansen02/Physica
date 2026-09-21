#pragma once
#include <Base/Basetypes.h>
#include <Engine/AssetLibrary.h>
#include <Base/Datastructures/String.h>

using namespace PH;

struct BiQuadCoefficients {
	real64 b[3];
	real64 a[3];
};

struct FilterParameter {
	const char* name;
	real64* valuePtr;
	const char* unit = "";

	FilterParameter(const char* name, real64* valuePtr, const char* unit) : name(name), valuePtr(valuePtr), unit(unit) {};
	FilterParameter(const char* name, real64* valuePtr) : name(name), valuePtr(valuePtr), unit("") {};
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
	const char* typeStr() const { return FilterTypeStrings[type()]; };

	// Calculate biquad coefficients from specific parametrization
	virtual void calculateCoefficients() = 0;
	inline const BiQuadCoefficients getBiquadCoefficients() const { return coeffs; };

	static Filter* deserialize(const YAML::Node& filter);
	void serialize(YAML::Emitter& out) const;

	// Returns new instance of filter of specific filter type
	static Filter* getType(FilterType type);

	// Holds parameters such as Q factor, cutoff frequency
	std::vector<FilterParameter> parameters;

protected:
	// Not bilinear transformed
	BiQuadCoefficients coeffs = { {0,0,1}, {0,0,1} };

	void addParameter(const char* name, real64* valuePtr, const char* unit) { parameters.push_back(FilterParameter(name, valuePtr, unit)); };
	void addParameter(const char* name, real64* valuePtr) { parameters.push_back(FilterParameter(name, valuePtr)); };
};

class LowPassFilter : public Filter {
public:
	LowPassFilter()
	{
		addParameter("Cutoff Frequency", &cutoff, "Hz");
		addParameter("Q Factor", &qfactor);
	};

	real64 cutoff = 10e2;
	real64 qfactor = 0.717f;

	const FilterType type() const override { return FilterType::LOWPASS; };
	void calculateCoefficients() override;
};

class BandPassFilter : public Filter {
public:
	BandPassFilter()
	{
		addParameter("Cutoff Frequency", &cutoff, "Hz");
		addParameter("Q Factor", &qfactor);
	};

	real64 cutoff = 10e2;
	real64 qfactor = 3.1f;

	const FilterType type() const override { return FilterType::BANDPASS; };
	void calculateCoefficients() override;
};

class BandStopFilter : public Filter {
public:
	BandStopFilter()
	{
		addParameter("Cutoff Frequency", &cutoff, "Hz");
		addParameter("Q Factor", &qfactor);
	};

	real64 cutoff = 10e2;
	real64 qfactor = 3.0f;

	const FilterType type() const override { return FilterType::BANDSTOP; };
	void calculateCoefficients() override;
};

class HighPassFilter : public Filter {
public:
	HighPassFilter()
	{
		addParameter("Cutoff Frequency", &cutoff, "Hz");
		addParameter("Q Factor", &qfactor);
	};

	real64 cutoff = 10e2;
	real64 qfactor = 0.717f;

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
		addParameter("Cutoff Frequency", &cutoff, "Hz");
		addParameter("Q Factor", &qfactor);
		addParameter("Anti-Cutoff Frequency", &anticutoff, "Hz");
		addParameter("Anti-Q Factor", &antiqfactor);
	};

	real64 cutoff = 1100.0f;
	real64 qfactor = 10.0f;
	real64 anticutoff = 900.0f;
	real64 antiqfactor = 10.0f;

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

	real64 b0 = 1;
	real64 b1 = 1;
	real64 b2 = 1;
	real64 a0 = 1;
	real64 a1 = 1;
	real64 a2 = 1;

	const FilterType type() const override { return FilterType::COEFFICIENTS; };
	void calculateCoefficients() override;
};
