
#pragma once
#include <Engine/Engine.h>
#include <Platform/Events.h>
#include <yaml-cpp/yaml.h>

namespace PH::Editor {

	typedef bool32 drawViewFN(void *userdata);
	typedef bool32 initViewFN(void* userdata);
	typedef bool32 dispatchViewEventFN(void* userdata, const PH::Platform::Event& e);
	typedef bool32 serializeViewFN(void* userdata, YAML::Emitter& out);
	typedef bool32 deserializeViewFN(void* userdata, YAML::Node& root);

	struct View {
		void* userdata;
		Engine::String name;

		drawViewFN* draw;
		initViewFN* init;
		dispatchViewEventFN* dispatchEvent;
		serializeViewFN* serialize;
		deserializeViewFN* deserialize;

		Engine::UUID sourceid;
		Engine::UUID id;
		
		bool32 focussed;
		bool32 hovered;
	};

	inline bool32 drawView(View* view) {

		if (!view->draw) {
			return false; 
		}

		ImGui::PushID(view->id);

		char windownamebuffer[64];
		sprintf_s<64>(windownamebuffer, "%s##%llu", view->name.getC_Str(), (uint64)view->id);

		if (ImGui::Begin(windownamebuffer)) {
			view->focussed = ImGui::IsWindowFocused();

			view->draw(view->userdata);
		}
		ImGui::End();
		ImGui::PopID();
	}
}