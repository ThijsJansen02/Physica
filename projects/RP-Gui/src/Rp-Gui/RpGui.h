#pragma once
#include <Platform/PlatformAPI.h>
#include <Base/Log.h>
#include <Engine/cppAPI/Rendering.hpp>

#include "PlotViewPanel.h"
#include "Text.h"

namespace PH::RpGui {

	using namespace PH::Platform;

	inline CONSOLE_WRITE(consoleWrite) {
		Platform::consoleWrite(str);
	}

	extern PH::Base::LogStream<consoleWrite> INFO;
	extern PH::Base::LogStream<consoleWrite> WARN;
	extern PH::Base::LogStream<consoleWrite> ERR;

	struct Context {
		Engine::Renderer2D::Context* renderer2D;
		PH::Platform::GFX::GraphicsPipeline pipeline2D;
		PH::Platform::GFX::GraphicsPipeline fontpipeline2D;

		PlotViewPanel plotviewpanel;

		Engine::ArrayList<glm::vec2> buffer;

		Font font;
	};

	extern Context* context;
	extern Engine::Renderer2D::Wrapper renderer2D;
}