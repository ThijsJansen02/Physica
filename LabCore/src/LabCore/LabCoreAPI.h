#pragma once
#include "View.h"

#define PH_LABCORE_API __declspec(dllexport)

namespace PH::LabCore {
	
	PH_LABCORE_API void addView(const View& view);

	//when called during any view method returns true if the view is currently focussed, false if it is not
	PH_LABCORE_API bool32 isViewFocussed();

	//returns the current view size;
	PH_LABCORE_API glm::vec2 getViewSize();

}