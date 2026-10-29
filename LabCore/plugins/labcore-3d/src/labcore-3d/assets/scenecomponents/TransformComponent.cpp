#include "TransformComponent.h"
#include <Engine/imgui/extensions.h>
#include "../../DefferredRenderer.h"
#include <LabCore/LabCoreAPI.h>

namespace PH::LabCore3D {

	bool32 serializeTransform(YAML::Emitter& out, entt::entity entity, Scene* scene) {
		
		const Transform& tr = getComponent<Transform>(scene, entity);
		
		out << YAML::BeginMap;
		out << YAML::Key << "rotXYZ" << YAML::Value << tr.rotXYZ;
		out << YAML::Key << "scale" << YAML::Value << tr.scale;
		out << YAML::Key << "translation" << YAML::Value << tr.translation;
		out << YAML::EndMap;

		return true;
	}

	glm::mat4 calcTransform(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale) {

		return glm::translate(glm::mat4(1.0f), translation)
			* glm::scale(glm::mat4(1.0f), scale)
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), { 0.0f, 0.0f, 1.0f })
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), { 0.0f, 1.0f, 0.0f })
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), { 1.0f, 0.0f, 0.0f });

	}

	bool32 deserializeTransform(YAML::Node in, entt::entity entity, Scene* scene) {
		Transform tc{};
		if (in["rotXYZ"]) {
			tc.rotXYZ = in["rotXYZ"].as<glm::vec3>();
		}

		if (in["scale"]) {
			tc.scale = in["scale"].as<glm::vec3>();
		}

		if (in["translation"]) {
			tc.translation = in["translation"].as<glm::vec3>();
		}

		tc.transform = calcTransform(tc.translation, tc.rotXYZ, tc.scale);

		addComponent<Transform>(scene, entity, tc);
		return true;
	}

	bool32 addTransform(entt::entity entity, Scene* scene) {
		Transform r{};
		r.transform = glm::mat4(1.0f);
		scene->registry.emplace<Transform>(entity, r);
		return true;
	}



	bool32 drawTransformUI(entt::entity entity, Scene* scene) {
		Transform& tc = getComponent<Transform>(scene, entity);
		bool dirty = false;

		dirty |= ImGui::DragFloat3("translation", (float*)&tc.translation, 0.01);
		dirty |= ImGui::DragFloat3("rotXYZ", (float*)&tc.rotXYZ, 0.01);
		dirty |= ImGui::DragFloat3("scale", (float*)&tc.scale, 0.01);
		
		if (dirty) {
			tc.transform = calcTransform(tc.translation, tc.rotXYZ, tc.scale);
		}
		
		return true;
	}

	bool32 hasTransform(entt::entity entity, Scene* scene) {
		return scene->registry.all_of<Transform>(entity);
	}

	bool32 drawComponents(Scene* scene, DefferedRenderer* renderer) {

		Engine::AssetLibrary* assets = LabCore::getAssetLibrary();
		auto* cubemesh = assets->getAssetByReference<Mesh>("cubemesh");
		auto* pipe = assets->getAssetByReference<GraphicsPipeline>("defaultmaterialpipe");

		for (auto ent : scene->registry.view<Transform>()) {
			auto& tf = getComponent<Transform>(scene, ent);

			renderer->drawMesh(cubemesh, pipe, tf.transform);

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