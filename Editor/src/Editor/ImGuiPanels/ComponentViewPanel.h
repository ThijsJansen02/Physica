#pragma once
#define PH_SCENE_IMPLEMENTATION
#include <imgui_internal.h>
#include "../EditorAPI.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ImGuizmo.h"
#include "ImGuiHelpFunctions.h"

namespace PH::Editor {

	inline void drawTransformComponent(Engine::TransformComponent* comp, Editor::Instance* editor) {

		glm::vec3 rotation;
		glm::vec3 scale;
		glm::vec3 translation;

		ImGuizmo::DecomposeMatrixToComponents(
			glm::value_ptr(comp->global),
			glm::value_ptr(translation),
			glm::value_ptr(rotation),
			glm::value_ptr(scale)
		);

		ImGui::DragFloat3("translation",(real32*)&translation, 0.01f);
		ImGui::DragFloat3("rotation", (real32*)&rotation, 0.01f);
		ImGui::DragFloat3("Scale", (real32*)&scale, 0.01f);

		ImGuizmo::RecomposeMatrixFromComponents((real32*)&translation, (real32*)&rotation, (real32*)&scale, (real32*)&comp->global);
	}

	inline void drawQuadComponent(Engine::QuadComponent* comp, Editor::Instance* editor) {

		ImGui::ColorEdit4("Quad color", (float*) & comp->color);
		selectAssetCombo<Engine::Assets::TextureImage>(&comp->texture, "texture", editor);
	}

	inline void drawMeshComponent(Engine::MeshComponent* comp, Editor::Instance* editor) {

		Engine::Assets::Mesh* selected = comp->mesh;
		if (selectAssetCombo<Engine::Assets::Mesh>(&comp->mesh, "Mesh", editor, 0, true)) {
			//new mesh selected!
			Engine::Assets::Mesh* selected = comp->mesh;

			if (selected) {
				comp->materials.resize(comp->mesh->defaultmaterials.getCount());

				uint32 index = 0;
				for (Engine::UUID matid : selected->defaultmaterials) {
					comp->materials[index] = editor->assets->getLazyLoaded<MaterialInstance>(matid);
					index++;
				}
			}
		}

		if (selected) {

			uint32 index = 0;
			for (auto& mat : comp->materials) {
				char buffer[64];
				sprintf_s<64>(buffer, "material [%u]", index);
				selectAssetCombo<MaterialInstance>(&mat, buffer, editor, index, true);
				index++;
			}
		}
	}

	inline void drawCameraComponent(Engine::CameraComponent* camera, Editor::Instance* instance) {

		ImGui::DragFloat("zoom", &camera->zoom, 0.01f);
	}

	inline void drawLightComponent(Engine::LightComponent* light, Editor::Instance* instance) {
		ImGui::ColorEdit3("tint", (real32*)&light->tint);
		ImGui::DragFloat("brightness", (real32*)&light->brightness, 0.1f);
		ImGui::DragFloat("linear", (real32*)&light->linear, 0.1f);
		ImGui::DragFloat("quadratic", (real32*)&light->quadratic, 0.1f);
		if(ImGui::BeginCombo("type", "...")) {
			if (ImGui::Selectable("point", light->type == Engine::LightType::POINT)) {
				light->type = Engine::LightType::POINT;
			}

			if (ImGui::Selectable("directional", light->type == Engine::LightType::DIRECTIONAL)) {
				light->type = Engine::LightType::DIRECTIONAL;
			}
			ImGui::EndCombo();
		}
	}

	template<typename T, typename UIfunc>
	inline void drawComponent(const std::string& name, Editor::Instance* editor, UIfunc func)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
		T* component = editor->selected.getComponent<T>();
		
		if (component)
		{
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar(
			);
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove component"))
					removeComponent = true;

				ImGui::EndPopup();
			}

			if (open)
			{
				func(component, editor);
				ImGui::TreePop();
			}

			if (removeComponent) {
				editor->selected.removeComponent<T>();
			}
		}
	}

	template<typename T>
	inline void DisplayAddComponentEntry(const char* name, Scene::Entity selected) {

		if (!selected.getComponent<T>()) {
			if (ImGui::MenuItem(name)) {
				T comp{};
				selected.addComponent<T>(comp);
				ImGui::CloseCurrentPopup();
			}
		}
	}

	inline void drawComponentView(Editor::Instance* editor, Scene::Instance* scene) {

		if (!editor->selected.valid()) {
			return;
		}

		Scene::Entity selectedentity = editor->selected;

		auto* tag = selectedentity.getComponent<Engine::TagComponent>();
		if (tag)
		{
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strncpy_s(buffer, sizeof(buffer), tag->tag.getC_Str(), sizeof(buffer));
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
			{
				tag->tag.set(buffer);
			}
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);


		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent"))
		{
			DisplayAddComponentEntry<Engine::MeshComponent>("MeshComponent", selectedentity);
			DisplayAddComponentEntry<Engine::QuadComponent>("QuadComponent", selectedentity);
			DisplayAddComponentEntry<Engine::LightComponent>("LightComponent", selectedentity);
			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		drawComponent<Engine::TransformComponent>("TransformCompoment", editor, drawTransformComponent);
		drawComponent<Engine::QuadComponent>("QuadComponent", editor, drawQuadComponent);
		drawComponent<Engine::MeshComponent>("MeshComponent", editor, drawMeshComponent);
		drawComponent<Engine::LightComponent>("Light", editor, drawLightComponent);
		drawComponent<Engine::CameraComponent>("CameraComponent", editor, drawCameraComponent);
	}

	inline bool32 componentView_draw(void* userdata) {

		Editor::Instance* editor = Editor::API::getCurrentInstance();
		drawComponentView(editor, editor->activescene->instance);
		return true;
	}

#define COMPONENT_VIEW_ID 1

	inline View loadComponentView() {
		Editor::View componentview{};
		componentview.id = Engine::Assets::Library::createRandomUUID();
		componentview.sourceid = COMPONENT_VIEW_ID;
		componentview.draw = Editor::componentView_draw;
		componentview.name = Engine::String::create("ComponentView");
		return componentview;
	}

}