#pragma once
#include "Plot.h"
#include "Text.h"

#include <Engine/Engine.h>
#include <Engine/Display.h>
#include <Engine/Events.h>
#include <Platform/platformAPI.h>

#include <yaml-cpp/yaml.h>

#include <glm/glm.hpp>

namespace PH::RpGui {

	class PlotViewPanel {
	public:
		Engine::ImGuiDisplay display;
		Box2D range;
		Box2D region;

		Box2D panelregion; //the region of the panel in window coordinates, used for drawing the plot and for transforming mouse coordinates from window coordinates to plot coordinates
		bool32 isfocussed;
		bool32 ishovered;

		Engine::String name;

		static PlotViewPanel create(Box2D range, const char* name);

		void onEvent(PH::Platform::Event* event);

		void setRange(Box2D newrange) {
			range = newrange;
		}

		void beginRenderPass() {
			Engine::beginRenderPass(display);
		}

		void endRenderPass() {
			Engine::endRenderPass(display);
		}

		void ImGuiDraw();
		void draw();

		void serialize(YAML::Emitter& emmiter);
		void deserialize(const YAML::Node& root);


	};



}