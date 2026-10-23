#include <LabCore/LabCoreAPI.h>
#include <Platform/platformAPI.h>
#include <imgui.h>

#include "assets/GraphicsPipeline.h"
#include "assets/Mesh.h"
#include "DefferredRenderer.h"

#include <Engine/Engine.h>
#include "views/SceneView.h"
#include "views/MaterialView.h"
#include "LabCore3D.h"
#include "assets/Texture.h"

namespace py = pybind11;

const char* PH::LabCore3D::plugindir = nullptr;

extern "C" {
	_declspec(dllexport) void init(const char* plugindir) {
		
		//set the directory of this plugin
		PH::LabCore3D::plugindir = plugindir;


		//the global scene descriptor set layout for deffered renderer
		PH::LabCore3D::DefferedRenderer::scenedescriptorlayout = PH::LabCore3D::DefferedRenderer::createSceneDescriptorLayout();
		PH::LabCore3D::DefferedRenderer::geometryrenderpass = PH::LabCore3D::DefferedRenderer::createGeometryRenderpass();
		PH::LabCore3D::DefferedRenderer::lightingrenderpass = PH::LabCore3D::DefferedRenderer::createLightingRendererpass();

		PH::LabCore3D::renderer = PH::LabCore3D::DefferedRenderer::create();

		PH::LabCore::addView(PH::LabCore3D::createSceneViewDescription());
		PH::LabCore::addView(PH::LabCore3D::createMaterialViewDescription());

		PH::Engine::AssetLibrary* assetlib = PH::LabCore::getAssetLibrary();
		assetlib->addAssetType(PH::LabCore3D::createGraphicsPipelineDescription());
		assetlib->addAssetType(PH::LabCore3D::createMeshAssetDescription());
		assetlib->addAssetType(PH::LabCore3D::createTextureAssetDescription());

		PH::Engine::UUID pipeid = 16439805985763607426;
		PH::Engine::UUID cubeid = 9808924416449029863;
		PH::Engine::UUID lightingpipeid = 2983062809953302303;
		PH::Engine::UUID albedoid = 328866994555156235;

		//graphics pipelines
		char srcpath[256];
		char binpath[256];

		sprintf_s<256>(srcpath, "%s/res/shaders/defferedmaterial.phshadersrc", plugindir);
		sprintf_s<256>(binpath, "%s/res/shaders/bin/defferedmaterial.phshaderbin", plugindir);

		PH::LabCore3D::GraphicsPipeline* defaultpipe = PH::Engine::Allocator::push(
			PH::LabCore3D::createGraphicsPipeline(srcpath, binpath, PH::LabCore3D::LC_PIPELINE_TYPE_3D_DEFFERED_MATERIAL)
		);

		assetlib->addAsset(defaultpipe, pipeid, "defaultmaterialpipe");

		sprintf_s<256>(srcpath, "%s/res/shaders/defferedlighting.phshadersrc", plugindir);
		sprintf_s<256>(binpath, "%s/res/shaders/bin/defferedlighting.phshaderbin", plugindir);

		PH::LabCore3D::GraphicsPipeline* lightingpipe = PH::Engine::Allocator::push(
			PH::LabCore3D::createGraphicsPipeline(srcpath, binpath, PH::LabCore3D::LC_PIPELINE_TYPE_3D_DEFFERED_LIGHTING)
		);

		assetlib->addAsset(lightingpipe, lightingpipeid, "defaultlightingpipe");

		//textures
		sprintf_s<256>(srcpath, "%s/res/textures/brickmaterial/albedo.jpg", plugindir);
		PH::LabCore3D::Texture* albedo = PH::Engine::Allocator::push(PH::LabCore3D::createTexture(srcpath));
		
		assetlib->addAsset(albedo, albedoid, "albedotexture");

		//meshes
		PH::LabCore3D::Mesh* cube = PH::Engine::Allocator::push(PH::LabCore3D::createCubeMesh());
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