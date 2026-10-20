#pragma once
#include "View.h"

#include <pybind11/pybind11.h>
#include <Engine/AssetLibrary.h>
#include <Engine/cppAPI/Rendering.hpp>

#ifdef PH_LABCORE_EXPORT
	#define PH_LABCORE_API __declspec(dllexport)
#else
#ifdef PH_LABCORE_IMPORT
	#define PH_LABCORE_API __declspec(dllimport)
#else
	#define PH_LABCORE_API
#endif
#endif

namespace PH::LabCore {
	
	PH_LABCORE_API void addView(const ViewDescription& view);

	//when called during any view method returns true if the view is currently focussed, false if it is not
	PH_LABCORE_API bool32 isViewFocussed();

	//returns the current view size;
	PH_LABCORE_API glm::vec2 getViewSize();

	PH_LABCORE_API Engine::Renderer2D::Wrapper* getRenderer2D();

	PH_LABCORE_API Engine::AssetLibrary* getAssetLibrary();

	//will be de deprecated when pipelines are made into material assets
	PH_LABCORE_API PH::Platform::GFX::GraphicsPipeline getDefault2DGraphicsPipeline();
	PH_LABCORE_API PH::Platform::GFX::GraphicsPipeline getDefault2DFontPipeline();


	PH_LABCORE_API PyObject* createPythonModule(const char* name);
}