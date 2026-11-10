#pragma once
#include "View.h"

#include <Engine/AssetLibrary.h>
#include <Engine/cppAPI/Rendering.hpp>

#ifdef LC_PLUGIN_PYTHON
#include <pybind11/pybind11.h>
#endif

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

	typedef bool32(*importAssetCallback)(Engine::AssetLibrary* lib, const char* filepath);
	typedef bool32(*onRenderCallback)(Engine::AssetLibrary* lib);
	
	PH_LABCORE_API void addView(const ViewDescription& view);

	//when called during any view method returns true if the view is currently focussed, false if it is not
	PH_LABCORE_API bool32 isViewFocussed();

	//returns the current view size;
	PH_LABCORE_API glm::vec2 getViewSize();

	//returns the mouseposition relative to the view that is calling the function, should be called from within a viewfunction
	PH_LABCORE_API glm::vec2 getViewMousePos();

	PH_LABCORE_API Engine::Renderer2D::Wrapper* getRenderer2D();

	PH_LABCORE_API Engine::AssetLibrary* getAssetLibrary();

	PH_LABCORE_API Base::SubString getCurrentProjectDir();

	//will be de deprecated when pipelines are made into material assets
	PH_LABCORE_API PH::Platform::GFX::GraphicsPipeline getDefault2DGraphicsPipeline();
	PH_LABCORE_API PH::Platform::GFX::GraphicsPipeline getDefault2DFontPipeline();

	//adds a callback for importing assets with a specific extension. the extension has to be persistent in memory
	PH_LABCORE_API void addImportAssetCallback(const char* extension, importAssetCallback callback);
	PH_LABCORE_API void addOnRenderCallback(onRenderCallback callback);

#ifdef LC_PLUGIN_PYTHON
	PH_LABCORE_API PyObject* createPythonModule(const char* name);
#endif
}