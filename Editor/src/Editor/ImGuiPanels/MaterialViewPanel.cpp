
#include "MaterialViewPanel.h"
#include "../Editor.h"
#include <Engine/assets/Material.h>
#include "ImGuiHelpFunctions.h"
#include <Engine/assets/Material.h>
#include <Engine/Events.h>

using namespace PH::Engine;

namespace PH::Editor {

	MaterialView createMaterialView() {

		MaterialView result{};
		return result;
	}

//might move this to a heap alloted buffer
#define INPUTBUFFER_SIZE (8 * KILO_BYTE)
	char fragmentsrcbuffer[INPUTBUFFER_SIZE];
	char vertexsrcbuffer[INPUTBUFFER_SIZE];
	char layoutyaml[INPUTBUFFER_SIZE];

	void saveMaterial(Engine::Assets::Material* material, Editor::Instance* instance) {
		material->fragmentshader.sourcecode.set(fragmentsrcbuffer);
		material->vertexshader.sourcecode.set(vertexsrcbuffer);
		instance->assets->saveToDisk(material);
	}

	void materialViewOnEvent(PH::Platform::Event e, Editor::Instance* instance, MaterialView* view) {

		if (e.type == PH_EVENT_TYPE_KEY_PRESSED) {
			if (e.lparam == PH_KEY_S && Engine::Events::isKeyPressed(PH_CONTROL)) {
				Engine::INFO << "saved material!\n";
				saveMaterial(view->selectedmaterial, instance);
			}
		}
	}

	char namebuffer[256] = "New Material";
	char dirbuffer[256] = "res/materials/";

	void drawMaterialView(Editor::Instance* editor, MaterialView* view) {

		Engine::ArenaScope s;

		selectAssetCombo<Engine::Assets::Material>(&view->selectedmaterial, "Material", editor);
		ImGui::SameLine();
		if (ImGui::Button("New Material")) {
			ImGui::OpenPopup("NewMaterialPopUp");
		}

		if (ImGui::BeginPopup("NewMaterialPopUp")) {

			ImGui::InputText("name", namebuffer, 256);
			ImGui::InputText("directory", dirbuffer, 256);

			if (ImGui::Button("Create")) {
				Engine::Assets::Material mat = Engine::Assets::createMaterial("res/materialbinaries", namebuffer);
				editor->assets->add(namebuffer, dirbuffer, mat);
			}
			ImGui::EndPopup();
		}
			

		view->focussed = ImGui::IsWindowFocused();
			 
		if (ImGui::Button("Save & Compile")) {
			saveMaterial(view->selectedmaterial, editor);

			if (Engine::Assets::compileMaterialFromSource(view->selectedmaterial, Engine::Display::renderpass)) {
				for (Engine::UUID id : view->selectedmaterial->instances) {
					auto* instance = editor->assets->get<Engine::Assets::MaterialInstance>(id);
					if (!instance) {
						continue;
					}

					if (instance->status != AssetStatus::LOADED) {
						continue;
					}
					Engine::Assets::rebuildMaterialInstance(
						instance,
						editor->assets->getLoadedByReference<Engine::Assets::TextureImage>("default_texture")
					);
				}
			}
		}

		if (view->selectedmaterial) {
			if (view->selectedmaterial != view->lastselectedmaterial) {
				if (view->lastselectedmaterial != nullptr) {
					view->lastselectedmaterial->fragmentshader.sourcecode.set(fragmentsrcbuffer);
					view->lastselectedmaterial->vertexshader.sourcecode.set(vertexsrcbuffer);
				}

				Base::stringCopy(view->selectedmaterial->fragmentshader.sourcecode.getC_Str(), fragmentsrcbuffer, INPUTBUFFER_SIZE);
				Base::stringCopy(view->selectedmaterial->vertexshader.sourcecode.getC_Str(), vertexsrcbuffer, INPUTBUFFER_SIZE);
				view->lastselectedmaterial = view->selectedmaterial;
			}

			if (ImGui::BeginTabBar("sources")) {
				if (ImGui::BeginTabItem("Fragement")) {
					ImVec2 contentregion = ImGui::GetContentRegionAvail();
					ImGui::InputTextMultiline(
						"Fragment source", 
						fragmentsrcbuffer, 
						INPUTBUFFER_SIZE, 
						{ contentregion.x, contentregion.y }, 
						ImGuiInputTextFlags_AllowTabInput,
						nullptr,
						nullptr
					);


					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("vertex")) {
					ImVec2 contentregion = ImGui::GetContentRegionAvail();
					ImGui::InputTextMultiline(
						"Vertex source",
						vertexsrcbuffer, 
						INPUTBUFFER_SIZE, 
						{ contentregion.x, contentregion.y },
						ImGuiInputTextFlags_AllowTabInput,
						nullptr,
						nullptr
					);


					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Instances")) {
					if (view->selectedmaterial->compiled) {

						selectAssetCombo<Engine::Assets::MaterialInstance>(&view->instance, "Instance", editor);

						ImGui::SameLine();
						if (ImGui::Button("New Instance")) {
							ImGui::OpenPopup("NewMaterialinstancePopUp");
						}

						if (ImGui::BeginPopup("NewMaterialinstancePopUp")) {

							ImGui::InputText("name", namebuffer, 256);
							ImGui::InputText("directory", dirbuffer, 256);

							if (ImGui::Button("Create")) {
								if (view->selectedmaterial) {
									Engine::Assets::MaterialInstance mat{};
									Engine::Assets::createMaterialInstance(view->selectedmaterial, &mat, editor->assets->getByReference<Assets::TextureImage>("default_texture"));
									view->instance = editor->assets->add<Assets::MaterialInstance>(namebuffer, dirbuffer, mat);
									view->selectedmaterial->instances.pushBack(view->instance->id);
								}
								else {
									WARN << "No Material was selected!\n";
								}
							}
							ImGui::EndPopup();
						}

						if (view->instance) {

							ImGui::Text("Textures");
							auto resources = view->instance->resources.getAll<Engine::ArenaAllocator>();

							for (auto* res : resources) {

								if(res->type == Engine::Assets::UniformResourceType::UNIFORM_SAMPLER) { 
									char texturename[256];
									if (res->arraysize == 1) {
										if(selectAssetCombo<Engine::Assets::TextureImage>(&res->samplerdescription.boundimages[0], res->name.getC_Str(), editor)) {
											if (res->samplerdescription.boundimages[0]) {
												Engine::Assets::setTextureForMaterialInstance(view->instance, res->binding, 0, res->samplerdescription.boundimages[0]);
											}
											editor->assets->saveToDisk<Engine::Assets::MaterialInstance>(view->instance);
										}
									}
								}

								if (res->type == Engine::Assets::UniformResourceType::UNIFORM_BUFFER) {
									ImGui::Text(res->name.getC_Str());
									void* data = Assets::mapUniformBufferSubResource(res->ubodescription.boundbuffers[0]);
									for (auto& mem : res->ubodescription.members) {

										if (mem.format == Assets::MemberType::REAL32) {
											if (mem.rows == 4) {
												ImGui::DragFloat4(mem.name.getC_Str(), (float*)((uint8*)data + mem.offset), 0.1f);
											}
											if (mem.rows == 3) {
												ImGui::DragFloat3(mem.name.getC_Str(), (float*)((uint8*)data + mem.offset), 0.1f);
											}
											if (mem.rows == 2) {
												ImGui::DragFloat2(mem.name.getC_Str(), (float*)((uint8*)data + mem.offset), 0.1f);
											}
											if (mem.rows == 1) {
												ImGui::DragFloat(mem.name.getC_Str(), (float*)((uint8*)data + mem.offset), 0.1f);
											}
										}

									}
								}
 
							}

						}
						
					}
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
		}
	}

}