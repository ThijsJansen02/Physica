#include "Scene.h"

namespace PH::LabCore3D {

	void serializeScene(YAML::Emitter& out, Scene* scene) {
		out << YAML::BeginMap;
		out << YAML::Key << "entities" << YAML::Value << YAML::BeginSeq;
		for (entt::entity ent : scene->registry.view<entt::entity>()) {
			
			//entity map
			out << YAML::BeginMap;
			out << YAML::Key << "components" << YAML::Value << YAML::BeginMap;
			for (const SceneComponentDescription& desc : Scene::scenecomponentdescriptions) {

				PH_DEBUG_ASSERT(desc.hasComponent_, "component description doesnt have a hascomponentFN");
				if (desc.hasComponent_(ent, scene)) {
					PH_DEBUG_ASSERT(desc.serializeComponent_, "component description doesnt have a serializeFN");

					//description name is primary key to find the component
					out << YAML::Key << desc.name.getC_Str() << YAML::Value;
					desc.serializeComponent_(out, ent, scene);
				}
			}

			//end component map
			out << YAML::EndMap;

			//end entity map
			out << YAML::EndMap;
		}

		//end entity list
		out << YAML::EndSeq;

		//end scenemap
		out << YAML::EndMap;
	}

	void deserializeScene(YAML::Node node, Scene* scene) {
		initScene(scene);
		
		if (!node["entities"]) {
			return;
		}

		for (YAML::Node entitynode : node["entities"]) {
			entt::entity entity = createEntity(scene);
			
			if (!entitynode["components"]) {
				continue;
			}

			YAML::Node componentsnode = entitynode["components"];

			for (const SceneComponentDescription& desc : Scene::scenecomponentdescriptions) {
				if (componentsnode[desc.name.getC_Str()]) {
					PH_DEBUG_ASSERT(desc.deserializeComponent_, "component description doesnt have a deserializeFN");
					desc.deserializeComponent_(componentsnode[desc.name.getC_Str()], entity, scene);
				}
			}
		}
	}

	Scene createScene() {
		Scene scene;
		scene.registry = entt::registry();
		scene.selected = entt::null;
		return scene;
	}

	void initScene(Scene* scene) {
		new (&scene->registry) entt::registry{};
		scene->selected = entt::null;
	}

	Engine::AssetDescription createSceneAssetDescription() {

		Engine::AssetDescription d;
		d.localdatasize = sizeof(Scene);
		d.deserialize_ = (Engine::deserializeAsset)deserializeScene;
		d.serialize_ = (Engine::SerializeAsset)serializeScene;
		d.extension = ".lcscene";
		d.id = Scene::getAssetDescriptionID();
		return d;
	}


}