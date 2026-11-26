#pragma once
#ifdef LC_SCENE_EXPORT
#define LC_SCENE_API  __declspec(dllexport)
#else
#define LC_SCENE_API __declspec(dllimport)
#endif
#include <Engine/AssetDescription.h>
#include <entt/entt.hpp>
#include <Engine/RenderPrimitives.h>

namespace PH::LabCore {

#define PH_ASSET_SCENE 6544691651984941516

	struct SceneComponentDescription;


	struct LC_SCENE_API Scene : Engine::AssetBase {

		//properties of the drawing camera that the view that displays the scene should fill in
		struct CameraProperties {
			glm::mat4 view;
			glm::mat4 projection;
			bool32 orthographic;

			union {
				Engine::Box2D domain;
				struct {
					real32 focallength;
				};
			};

			//transforms a worldspace vector to a screenspace vector. does not preserve position, only length and rotation
			glm::mat4 worldtoscreen;

			//inverse of view matrix
			glm::mat4 viewtoworld;
		};

		struct RenderProperties {
			glm::vec2 viewportsize;
			CameraProperties camera;
		};

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

	typedef bool32(EntityHasComponentFN)(entt::entity entity, Scene* reg);
	typedef bool32(DrawComponentUIFN)(entt::entity entity, Scene* reg);

	typedef bool32(AddComponentFN)(entt::entity entity, Scene* reg);
	typedef bool32(RemoveComponentFN)(entt::entity entity, Scene* reg);

	//these functions should act on all the components
	typedef bool32(UpdateComponentsFN)(Scene* reg);
	typedef bool32(DrawComponentsFN)(Scene* reg, Scene::RenderProperties* renderproperties);

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

	Engine::AssetDescription createSceneAssetDescription();
	LC_SCENE_API Scene createScene();
	LC_SCENE_API void initScene(Scene* scene);

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
		entt::entity ent = scene->registry.create();
		return ent;
	}

}
