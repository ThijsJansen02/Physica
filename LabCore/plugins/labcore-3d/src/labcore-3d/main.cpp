#include <Engine/Engine.h>
#include <LabCore/LabCoreAPI.h>
#include <Platform/platformAPI.h>
#include <imgui.h>

#include "assets/GraphicsPipeline.h"
#include "assets/Mesh.h"
#include "DefferredRenderer.h"

#include "views/SceneView.h"
#include "views/MaterialView.h"
#include "views/SceneHierarchyView.h"
#include "assets/Scene.h"
#include "LabCore3D.h"
#include "assets/Texture.h"
#include "assets/Scene.h"
#include "assets/scenecomponents/NameTag.h"
#include "assets/scenecomponents/TransformComponent.h"

namespace py = pybind11;
using namespace PH;

PH::Engine::String PH::LabCore3D::home = {};

extern "C" {
	_declspec(dllexport) void init(const char* plugindir) {
		
		//set the directory of this plugin
		PH::LabCore3D::home = Engine::String::create(plugindir);

		//set directory to labcore home
		PH::Platform::setCurrentDirectory(PH::LabCore3D::home.getSubString());

		PH::LabCore3D::Scene::scenecomponentdescriptions.pushBack(LabCore3D::Tag::createDescription());
		PH::LabCore3D::Scene::scenecomponentdescriptions.pushBack(LabCore3D::Transform::createDescription());

		//the global scene descriptor set layout for deffered renderer
		PH::LabCore3D::DefferedRenderer::scenedescriptorlayout = PH::LabCore3D::DefferedRenderer::createSceneDescriptorLayout();
		PH::LabCore3D::DefferedRenderer::geometryrenderpass = PH::LabCore3D::DefferedRenderer::createGeometryRenderpass();
		PH::LabCore3D::DefferedRenderer::lightingrenderpass = PH::LabCore3D::DefferedRenderer::createLightingRendererpass();

		PH::LabCore3D::renderer = PH::LabCore3D::DefferedRenderer::create();

		PH::LabCore::addView(PH::LabCore3D::createSceneViewDescription());
		PH::LabCore::addView(PH::LabCore3D::createMaterialViewDescription());
		PH::LabCore::addView(PH::LabCore3D::createSceneHierarchyViewDescription());

		PH::Engine::AssetLibrary* assetlib = PH::LabCore::getAssetLibrary();
		assetlib->addAssetType(PH::LabCore3D::createGraphicsPipelineDescription());
		assetlib->addAssetType(PH::LabCore3D::createMeshAssetDescription());
		assetlib->addAssetType(PH::LabCore3D::createTextureAssetDescription());
		assetlib->addAssetType(PH::LabCore3D::createSceneAssetDescription());

		PH::Engine::UUID pipeid = 16439805985763607426;
		PH::Engine::UUID cubeid = 9808924416449029863;
		PH::Engine::UUID lightingpipeid = 2983062809953302303;
		PH::Engine::UUID albedoid = 328866994555156235;
		PH::Engine::UUID brickid = 65465143419846515;
		PH::Engine::UUID normalid = 64641685878151;


		//textures
		PH::LabCore3D::Texture* albedo = PH::Engine::Allocator::push(PH::LabCore3D::createTexture("res/textures/brickmaterial/albedo.jpg"));
		assetlib->addPersistentAsset(albedo, brickid, "brick_albedo");

		albedo = PH::Engine::Allocator::push(PH::LabCore3D::createTexture("res/textures/brickmaterial/normal_dx.jpg", PH::Platform::GFX::FORMAT_R8G8B8A8_UNORM));
		assetlib->addPersistentAsset(albedo, normalid, "brick_normal");

		albedo = PH::Engine::Allocator::push(PH::LabCore3D::createTexture("res/textures/metal/albedo.jpg"));
		assetlib->addPersistentAsset(albedo, albedoid, "metal_albedo");



		//default deffered material
		assetlib->deserializeAddAsset<PH::LabCore3D::GraphicsPipeline>("res/pipelines/defaultmaterialpipe.phpipe");

		//default lightingpipe
		assetlib->deserializeAddAsset<PH::LabCore3D::GraphicsPipeline>("res/pipelines/defaultlightingpipe.phpipe");

		//meshes
		PH::LabCore3D::Mesh* cube = PH::Engine::Allocator::push(PH::LabCore3D::createCubeMesh());
		assetlib->addPersistentAsset(cube, cubeid, "cubemesh");

		//python module test
		PyObject* raw = PH::LabCore::createPythonModule("LabCore3D");
		py::module_ m = py::reinterpret_steal<py::module_>(raw);

		m.def("print", [](py::object obj) {
			std::string str = py::str(obj);

			PH::Engine::INFO << str.c_str();

			});
	}
}