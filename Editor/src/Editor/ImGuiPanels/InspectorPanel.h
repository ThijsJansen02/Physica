#pragma once
#define PH_SCENE_IMPLEMENTATION

#include "../EditorAPI.h"
#include <Engine/Scene.h>
#include "../Editor.h"
#include <imgui.h>
#include <Engine/Components.h>


namespace PH::Editor {

	inline void drawEntityNode(Scene::Entity entity, Editor::Instance* editor)
	{
		auto* tag = entity.getComponent<Engine::TagComponent>();

		ImGuiTreeNodeFlags flags = ((editor->selected == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)tag->uuid, flags, tag->tag.getC_Str());
		
		if (ImGui::IsItemClicked())
		{
			editor->selected = entity;
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		if (opened)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag->tag.getC_Str());
			if (opened)
				ImGui::TreePop();
			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			Scene::destroyEntity(entity.handle, entity.parent);
			if (editor->selected == entity)
				editor->selected = {};
		}
	}

	inline void drawInspectorPanel(Editor::Instance* editor, Scene::Instance* scene) {

		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));

		if (ImGui::Button("Create entity")) {
			Scene::createEntity(scene, "New Entity", Engine::Assets::Library::createRandomUUID());
		}

		auto view = scene->registry.view<Engine::TagComponent>();

		for (auto ent : view) {
			drawEntityNode({ scene, (Scene::EntityHandle)ent }, editor);
		}

		ImGui::PopStyleVar();
	}

	inline bool32 inspectorView_draw(void* userdata) {
		Editor::Instance* editor = API::getCurrentInstance();

		drawInspectorPanel(editor, editor->activescene->instance);
		return true;
	}

#define INSPECTORVIEW_ID 3

	inline View LoadInspectorView() {
		View inspectorview{};
		inspectorview.draw = inspectorView_draw;
		inspectorview.name = Engine::String::create("InspectorView");
		inspectorview.id = Library::createRandomUUID();
		inspectorview.sourceid = INSPECTORVIEW_ID;
		return inspectorview;
	}
}