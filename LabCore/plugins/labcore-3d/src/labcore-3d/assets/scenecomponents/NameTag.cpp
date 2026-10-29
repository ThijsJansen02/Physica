#include "NameTag.h"
#include <Engine/imgui/extensions.h>
namespace PH::LabCore3D {

	bool32 serializeTag(YAML::Emitter& out, entt::entity entity, Scene* scene) {
		
		const Tag& tag = getComponent<Tag>(scene, entity);
		
		out << YAML::BeginMap;
		out << YAML::Key << "name" << YAML::Value << tag.name.getC_Str();
		out << YAML::Key << "id" << YAML::Value << tag.id;
		out << YAML::EndMap;

		return true;
	}

	bool32 deserializeTag(YAML::Node in, entt::entity entity, Scene* scene) {
		Tag tag{};
		if (!in["id"]) {
			tag.id = Engine::createRandomUUID();
		}
		else {
			tag.id = in["id"].as<Engine::UUID>();
		}

		if (!in["name"]) {
			tag.name = Engine::String::create("unnamed");
		}
		else {
			tag.name = in["name"].as<Engine::String>();
		}

		addComponent<Tag>(scene, entity, tag);
		return true;
	}

	bool32 drawTagUI(entt::entity entity, Scene* scene) {
		Tag& tag = getComponent<Tag>(scene, entity);
		ImGuiext::Input("Tag", &tag.name);
		return true;
	}

	bool32 hasTag(entt::entity entity, Scene* scene) {
		return scene->registry.all_of<Tag>(entity);
	}

	SceneComponentDescription Tag::createDescription() {


		SceneComponentDescription desc{};
		desc.hasComponent_ = hasTag;
		desc.deserializeComponent_ = deserializeTag;
		desc.serializeComponent_ = serializeTag;
		desc.drawUI_ = drawTagUI;
		desc.name = Engine::String::create(getComponentName()); 
		return desc;
	}


}