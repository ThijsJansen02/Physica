#pragma once
#include <Engine/Rendering.h>
#include <Platform/PlatformAPI.h>
#include "PlotViewPanel.h"
#include "Text.h"

namespace PH::RpGui {

	struct Context {
		Engine::Renderer2D::Context* renderer2D;
		PH::Platform::GFX::GraphicsPipeline pipeline2D;
		PH::Platform::GFX::GraphicsPipeline fontpipeline2D;

		Engine::ArrayList<TransferFunction> activetransferfunctions;

		PlotViewPanel magnitudeplot;
		PlotViewPanel phaseplot;

		Engine::ArrayList<glm::vec2> buffer;

		Engine::String openproject;

		Font font;
	};
}