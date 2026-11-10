#pragma once
#ifdef LC_SCENE_EXPORT
#define LC_SCENE_API  __declspec(dllexport)
#else
#define LC_SCENE_API __declspec(dllimport)
#endif
#include <Engine/AssetDescription.h>
#include <entt/entt.hpp>


namespace PH::LabCore {

#define PH_ASSET_SCENE 6544691651984941516

	struct Scene;

	typedef bool32(EntityHasComponentFN)(entt::entity entity, Scene* reg);
	typedef bool32(DrawComponentUIFN)(entt::entity entity, Scene* reg);

	typedef bool32(AddComponentFN)(entt::entity entity, Scene* reg);
	typedef bool32(RemoveComponentFN)(entt::entity entity, Scene* reg);

	//these functions should act on all the components
	typedef bool32(UpdateComponentsFN)(Scene* reg);
	typedef bool32(DrawComponentsFN)(Scene* reg);

	typedef bool32(serializeComponentFN)(YAML::Emitter& out, entt::entity entity, Scene* reg);
	typedef bool32(deserializeComponentFN)(YAML::Node in, entt::entity entity, Scene* reg);

	struct SceneComponentDescription {

		EntityHasComponentFN* hasComponent_;
		DrawComponentUIFN* drawUI_;
		AddComponentFN* addComponent;
		RemoveComponentFN* removeComponent_;
		UpdateComponentsFN* updateComponents_;
		DrawComponentsFN* drawComponents_;

		serializeComponentFN* serializeComponent_;
		deserializeComponentFN* deserializeComponent_;

		//name is a primary key for the component;
		Engine::String name;

		sizeptr localdatasize;
	};

	struct LC_SCENE_API Scene : Engine::AssetBase {

		static Engine::AssetDescriptionID getAssetDescriptionID() {
			return PH_ASSET_SCENE;
		}

		static const char* getExtension() {
			return ".lcscene";
		}

		static void addComponentDescription(const SceneComponentDescription& desc);

		//the currently selected entity for this scene
		entt::entity selected;
		entt::registry registry;
		static Engine::ArrayList<SceneComponentDescription> scenecomponentdescriptions;
	};

	Engine::AssetDescription createSceneAssetDescription();
	Scene createScene();
	void initScene(Scene* scene);

	template<typename comp>
	void addComponent(Scene* scene, entt::entity entity, const comp& component) {
		scene->registry.emplace<comp>(entity, component);
	}

	template<typename comp>
	bool32 hasComponent(Scene* scene, entt::entity entity) {
		return scene->registry.all_of<comp>(entity);
	}

	template<typename comp>
	comp& getComponent(Scene* scene, entt::entity entity) {
		return scene->registry.get<comp>(entity);
	}

	template<typename comp>
	void removeComponent(Scene* scene, entt::entity entity) {
		scene->registry.remove<comp>(entity);
	}

	inline entt::entity createEntity(Scene* scene) {
		return scene->registry.create();
	}

}
