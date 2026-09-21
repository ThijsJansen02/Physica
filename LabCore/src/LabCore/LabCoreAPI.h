#pragma once
#include "View.h"

#define PH_LABCORE_API __declspec(dllexport)

namespace PH::LabCore {
	
	PH_LABCORE_API void addView(const View& view);

}