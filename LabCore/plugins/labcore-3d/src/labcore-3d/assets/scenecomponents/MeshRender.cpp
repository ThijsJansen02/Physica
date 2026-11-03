#include "Transform.h"
#include "MeshRender.h"
#include <Engine/imgui/extensions.h>
#include "../../DefferredRenderer.h"
#include <LabCore/LabCoreAPI.h>

namespace PH::LabCore3D {

	bool32 serializeMeshRender(YAML::Emitter& out, entt::entity entity, Scene* scene) {

		const MeshRender& mr = getComponent<MeshRender>(scene, entity);

		out << YAML::BeginMap;
		out << YAML::Key << "mesh" << YAML::Value << mr.mesh->assetid;
		out << YAML::Key << "material" << YAML::Value << mr.material->assetid;
		out << YAML::EndMap;

		return true;
	}


	bool32 deserializeMeshRender(YAML::Node in, entt::entity entity, Scene* scene) {
		MeshRender mr{};
		
		auto assets = LabCore::getAssetLibrary();

		if (in["mesh"]) {
			mr.mesh = assets->getLoadedAssetIdentifier<Mesh>(in["mesh"].as<Engine::AssetID>());
		}

		if (in["material"]) {
			mr.material = assets->getLoadedAssetIdentifier<GraphicsPipeline>(in["material"].as<Engine::AssetID>());
		}

		addComponent<MeshRender>(scene, entity, mr);
		return true;
	}

	bool32 addMeshRender(entt::entity entity, Scene* scene) {
		MeshRender mr{};
		mr.material = nullptr;
		mr.mesh = nullptr;

		scene->registry.emplace<MeshRender>(entity, mr);
		return true;
	}



	bool32 drawMeshRenderUI(entt::entity entity, Scene* scene) {
		MeshRender& mr = getComponent<MeshRender>(scene, entity);
		
		mr.material = ImGuiext::assetSelecter<GraphicsPipeline>("material", mr.material, "select material", LabCore::getAssetLibrary());
		mr.mesh = ImGuiext::assetSelecter<Mesh>("mesh", mr.mesh, "select mesh", LabCore::getAssetLibrary());

		return true;
	}

	bool32 hasMeshRender(entt::entity entity, Scene* scene) {
		return scene->registry.all_of<MeshRender>(entity);
	}

	bool32 drawMeshRender(Scene* scene, DefferedRenderer* renderer) {

		Engine::AssetLibrary* assets = LabCore::getAssetLibrary();

		for (auto ent : scene->registry.view<Transform, MeshRender>()) {

			const auto& [tf, mr] = scene->registry.get<Transform, MeshRender>(ent);
			if (!mr.material || !mr.mesh) {
				continue;
			}

			renderer->drawMesh((Mesh*)mr.mesh->assetdata, (GraphicsPipeline*)mr.material->assetdata, tf.transform, (uint32)ent);
		}

		return true;
	}

	SceneComponentDescription MeshRender::createDescription() {


		SceneComponentDescription desc{};
		desc.hasComponent_ = hasMeshRender;
		desc.deserializeComponent_ = deserializeMeshRender;
		desc.serializeComponent_ = serializeMeshRender;
		desc.drawComponents_ = drawMeshRender;
		desc.addComponent = addMeshRender;
		desc.drawUI_ = drawMeshRenderUI;
		desc.name = Engine::String::create(getComponentName());
		return desc;
	}


}