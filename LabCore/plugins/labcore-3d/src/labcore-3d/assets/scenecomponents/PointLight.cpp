#include "PointLight.h"
#include <Engine/imgui/extensions.h>
#include <LabCore/LabCoreAPI.h>
#include "Transform.h"

namespace PH::LabCore3D {

	bool32 serializePointlight(YAML::Emitter& out, entt::entity entity, Scene* scene) {

		const PointLight& pl = getComponent<PointLight>(scene, entity);

		out << YAML::BeginMap;
		out << YAML::Key << "postion" << YAML::Value << pl.light.position;
		out << YAML::Key << "intensity" << YAML::Value << pl.light.intensity;
		out << YAML::Key << "color" << YAML::Value << pl.light.color;
		out << YAML::EndMap;

		return true;
	}

	bool32 deserializePointlight(YAML::Node in, entt::entity entity, Scene* scene) {
		PointLight pl{};
		if (in["position"]) {
			pl.light.position = in["position"].as<glm::vec3>();
		}

		if (in["intensity"]) {
			pl.light.intensity = in["intensity"].as<real32>();
		}

		if (in["color"]) {
			pl.light.color = in["color"].as<glm::vec3>();
		}

		addComponent<PointLight>(scene, entity, pl);
		return true;
	}

	bool32 addPointLight(entt::entity entity, Scene* scene) {
		PointLight pl{};
		pl.light.position = { 0.0f, 0.0f, 0.0f };
		pl.light.color = { 1.0f, 1.0f, 1.0f };
		pl.light.intensity = 1.0f;
		pl.light.attenuation = 1.0f;
		scene->registry.emplace<PointLight>(entity, pl);
		return true;
	}



	bool32 drawTransformUI(entt::entity entity, Scene* scene) {
		PointLight& pl = getComponent<PointLight>(scene, entity);
		bool dirty = false;

		//dirty |= ImGui::DragFloat3("position", (float*)&pl.light.position, 0.01);
		dirty |= ImGui::DragFloat3("color", (float*)&pl.light.color, 0.01);
		dirty |= ImGui::DragFloat3("intensity", (float*)&pl.light.intensity, 0.01);

		return true;
	}

	bool32 hasTransform(entt::entity entity, Scene* scene) {
		return scene->registry.all_of<PointLight>(entity);
	}

	bool32 drawComponents(Scene* scene, DefferedRenderer* renderer) {

		Engine::AssetLibrary* assets = LabCore::getAssetLibrary();
		auto* cubemesh = assets->getAssetByReference<Mesh>("cubemesh");
		auto* pipe = assets->getAssetByReference<GraphicsPipeline>("defaultmaterialpipe");

		auto view = scene->registry.view<Transform, PointLight>();

		for (auto ent : view) {
			auto& tf = getComponent<Transform>(scene, ent);

			renderer->drawMesh(cubemesh, pipe, tf.transform, (uint32)ent);

		}

		return true;
	}

	SceneComponentDescription Transform::createDescription() {


		SceneComponentDescription desc{};
		desc.hasComponent_ = hasTransform;
		desc.deserializeComponent_ = deserializeTransform;
		desc.serializeComponent_ = serializeTransform;
		desc.drawComponents_ = drawComponents;
		desc.addComponent = addTransform;
		desc.drawUI_ = drawTransformUI;
		desc.name = Engine::String::create(getComponentName());
		return desc;
	}


}