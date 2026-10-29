#include "SceneHierarchyView.h"
#include <LabCore/LabCoreAPI.h>
#include <Engine/coreassets/Font.h>
#include <Engine/imgui/extensions.h>
#include "../assets/scenecomponents/NameTag.h"
#include "../DefferredRenderer.h"

namespace PH::LabCore3D {

	Engine::ArrayList<SceneComponentDescription> Scene::scenecomponentdescriptions = Engine::ArrayList<SceneComponentDescription>::create(0);

	PH::bool32 drawSceneHierarchyView(SceneHierarchyView* view) {

		Engine::ArrayList<Engine::AssetIdentifier*> options = LabCore::getAssetLibrary()->getAllIdentifiersOfType<Scene>();

		view->scene = ImGuiext::assetSelecter("scene", view->scene, options.getArray(), "select scene");
		ImGui::SameLine();
		if (ImGui::Button("create scene")) {
			char buffer[256];

			PH::Platform::OpenFileDialogInfo info{};
			info.filter = "*.lcscene";
			info.flags = PH_DIALOG_PATHMUSTEXIST;
			info.resultbuffer = buffer;
			info.resultbuffersize = 256;
			info.read = false;
			info.startingdir = LabCore::getCurrentProjectDir().getC_Str();


			if (PH::Platform::openFileDialog(info)) {
				auto filepath = Engine::String::create(buffer);
				Base::setExtension(&filepath, Scene::getExtension());

				auto filename = Engine::String::create(filepath.getSubString().getFromLast('/').getTillLastExcluding('.').moveHead(1));



				Scene* scene = (Scene*)Engine::Allocator::alloc(sizeof(Scene));
				initScene(scene);

				view->scene = LabCore::getAssetLibrary()->addAsset<Scene>(scene, filepath.getC_Str(), filename.getC_Str(), Engine::PH_ASSET_SERIALIZABLE);

				Engine::String::destroy(&filepath);
				Engine::String::destroy(&filename);
			}
		}

		//if there is no view selected yet, return
		if (!view->scene) {
			return true;
		}

		Scene* scene = LabCore::getAssetLibrary()->getLoadedAsset<Scene>(view->scene->assetid);
		if (view->scene->status == Engine::LOADED) {

			if (ImGui::TreeNodeEx(view->scene->getName(), ImGuiTreeNodeFlags_DefaultOpen)) {
				if (ImGui::Button("add entity")) {
					entt::entity newentity = createEntity(scene);
					addComponent<Tag>(scene, newentity, { Engine::String::create("New Entity") });
				}

				for (entt::entity ent : scene->registry.view<Tag>()) {
					Tag tag = scene->registry.get<Tag>(ent);
					
					char buffer[256];
					sprintf_s<256>(buffer, "%s###%llu", tag.name.getC_Str(), tag.id);

					ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
					if (scene->selected == ent) {
						flags |= ImGuiTreeNodeFlags_Selected;
					}

					if (ImGui::TreeNodeEx(buffer, flags)) {
						if (ImGui::TreeNodeEx("components", ImGuiTreeNodeFlags_DefaultOpen)) {
							if (ImGui::Button("add component")) {
								ImGui::OpenPopup("components");
							}

							if (ImGui::BeginPopup("components")) {
								ImGui::Text("Select component");

								for (const SceneComponentDescription& desc : Scene::scenecomponentdescriptions) {

									if (ImGui::MenuItem(desc.name.getC_Str())) {
										if (desc.addComponent) {
											desc.addComponent(ent, scene);
										}
									}
								}
								ImGui::EndPopup();
							}

							for (const SceneComponentDescription& desc : Scene::scenecomponentdescriptions) {
								if (desc.hasComponent_(ent, scene)) {
									if(ImGui::TreeNodeEx(desc.name.getC_Str(), ImGuiTreeNodeFlags_DefaultOpen)) {
										if (desc.drawUI_) {
											desc.drawUI_(ent, scene);
										}
										ImGui::TreePop();
									}
								}

							}
							ImGui::TreePop();
						}

						ImGui::TreePop();
					}

					if (ImGui::IsItemClicked()) {
						scene->selected = ent;
					}
				}

				ImGui::TreePop();
			}

		}

		

		return true;
	}

	PH::bool32 updateSceneHierarchyView(SceneHierarchyView* view) {
		

		return true;
	}

	PH::bool32 instantiateSceneHierarchyView(SceneHierarchyView* view) {
		view->scene = nullptr;
		return true;
	}

	PH::bool32 destroySceneHierarchyView(SceneHierarchyView* data) {
		return true;
	}

	PH::bool32 SceneHierarchyViewOnEvent(SceneHierarchyView* data, const PH::Platform::Event& e) {
		return true;
	}

	PH::bool32 deserializeSceneHierarchyView(SceneHierarchyView* data, const YAML::Node& node) {

		Engine::AssetID id = node.as<Engine::AssetID>();
		data->scene = LabCore::getAssetLibrary()->getAssetIdentifier<Scene>(id);
		return true;
	}

	PH::bool32 serializeSceneHierarchyView(SceneHierarchyView* data, YAML::Emitter& out) {
		if (data->scene) {
			out << data->scene->assetid;
		}
		else {
			out << 0;
		}
		return true;
	}

	LabCore::ViewDescription createSceneHierarchyViewDescription() {
		LabCore::ViewDescription d{};
		d.draw_ = (LabCore::ViewDrawFunction)drawSceneHierarchyView;
		d.onUpdate_ = (LabCore::ViewOnUpdateFunction)updateSceneHierarchyView;
		d.onEvent_ = (LabCore::ViewOnEventFunction)SceneHierarchyViewOnEvent;

		d.destroy_ = (LabCore::ViewDestroyFunction)destroySceneHierarchyView;
		d.instantiate_ = (LabCore::ViewInstantiateFunction)instantiateSceneHierarchyView;

		d.windowpadding = { 5,5 };
		d.windowflags = 0;

		d.deserialize_ = (LabCore::ViewDeserializeFunction)deserializeSceneHierarchyView;
		d.serialize_ = (LabCore::ViewSerializeFunction)serializeSceneHierarchyView;

		d.name = Engine::String::create("Scene Hierarchy View");
		d.instancedatasize = sizeof(SceneHierarchyView);

		return d;
	}
}