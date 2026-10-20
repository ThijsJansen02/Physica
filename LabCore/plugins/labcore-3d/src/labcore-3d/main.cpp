#include <LabCore/LabCoreAPI.h>
#include <Platform/platformAPI.h>
#include <imgui.h>

#include "assets/GraphicsPipeline.h"
#include "assets/Mesh.h"
#include "DefferredRenderer.h"

#include <Engine/Engine.h>
#include "SceneView.h"

namespace py = pybind11;

extern "C" {
	_declspec(dllexport) void init() {
		
		//the global scene descriptor set layout for deffered renderer
		PH::LabCore3D::DefferedRenderer::scenedescriptorlayout = PH::LabCore3D::DefferedRenderer::createSceneDescriptorLayout();
		PH::LabCore3D::renderer = PH::LabCore3D::DefferedRenderer::create();

	
		PH::LabCore::addView(PH::LabCore3D::createSceneViewDescription());

		PH::Engine::AssetLibrary* assetlib = PH::LabCore::getAssetLibrary();
		assetlib->addAssetType(PH::LabCore3D::createGraphicsPipelineDescription());
		assetlib->addAssetType(PH::LabCore3D::createMeshAssetDescription());

		PH::Engine::UUID pipeid = 16439805985763607426;
		PH::Engine::UUID cubeid = 9808924416449029863;

		PH::LabCore3D::GraphicsPipeline* defaultpipe = PH::Engine::Allocator::push(PH::LabCore3D::createGraphicsPipeline("res/shaders/material3d.phmatsrc", "res/shaders/bin/material3d.phmatbin"));
		PH::LabCore3D::Mesh* cube = PH::Engine::Allocator::push(PH::LabCore3D::createCubeMesh());

		assetlib->addAsset(defaultpipe, pipeid, "defaultmaterialpipe");
		assetlib->addAsset(cube, cubeid, "cubemesh");

		//python module test
		PyObject* raw = PH::LabCore::createPythonModule("LabCore3D");
		py::module_ m = py::reinterpret_steal<py::module_>(raw);

		m.def("print", [](py::object obj) {
			std::string str = py::str(obj);

			PH::Engine::INFO << str.c_str();

			});
	}
}