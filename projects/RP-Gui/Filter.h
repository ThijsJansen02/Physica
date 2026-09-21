#pragma once
#include <Base/Basetypes.h>

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
	virtual BiQuadCoefficients getBiquadCoefficients() = 0;

private:
	FilterType type;

	// Not bilinear transformed
	BiQuadCoefficients coeffs;
};

