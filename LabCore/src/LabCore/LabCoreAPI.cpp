#include "LabCore.h"
#include "LabCoreAPI.h"
#include <pybind11/pybind11.h>

namespace PH::LabCore {

	PH_LABCORE_API void addView(const ViewDescription& view) {
		appcontext->views.pushBack(view);
	}

	PH_LABCORE_API glm::vec2 getViewSize() {

		Engine::Box2D region = LabCore::openview->region;
		return { region.right - region.left, region.top - region.bottom };
	}

	Engine::Renderer2D::Wrapper* getRenderer2D() {
		return &appcontext->renderer2D;
	}

	PH::Platform::GFX::GraphicsPipeline getDefault2DGraphicsPipeline() {
		return appcontext->defaultgraphicspipeline2D;

	}
	PH::Platform::GFX::GraphicsPipeline getDefault2DFontPipeline() {
		return appcontext->defaultfontpipeline2D;
	}

	bool32 isViewFocussed() {
		return LabCore::openview->isfocussed;
	}

	Engine::AssetLibrary* getAssetLibrary() {
		return &LabCore::appcontext->assets;
	}

	Base::SubString getCurrentProjectDir() {
		return appcontext->projectdir.getSubString();
	}

	namespace py = pybind11;

	PyObject* createPythonModule(const char* name)
	{
		py::module_ module = py::module_::create_extension_module(
			name, nullptr, new PyModuleDef{}
		);

		PyObject* modules = PyImport_GetModuleDict();

		if (PyDict_SetItemString(modules, name, module.ptr()) < 0)
			throw py::error_already_set();

		return module.release().ptr();
	}
}