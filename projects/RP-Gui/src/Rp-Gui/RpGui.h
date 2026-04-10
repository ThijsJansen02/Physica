#pragma once
#include <Platform/PlatformAPI.h>
#include <Base/Log.h>
#include <Engine/cppAPI/Rendering.hpp>

#include "PlotViewPanel.h"
#include "Text.h"

namespace PH::RpGui {

	using namespace PH::Platform;

	struct Filter {
		real32 cutoff;
		real32 Qfactor;
		real32 gain;
	};

	struct TransferFunction {
		Engine::String name;
		Engine::ArrayList<Filter> filters;
	};

	inline real32 bandpass(real32 x, void* params_) {

		Filter* params = (Filter*)params_;
		return (x * x + params->cutoff * params->cutoff) / (x * x + ((x * params->cutoff) / params->Qfactor) + params->cutoff * params->cutoff);
	}

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

		Engine::ArrayList<TransferFunction> activetransferfunctions;

		PlotViewPanel magnitudeplot;
		PlotViewPanel phaseplot;

		Engine::ArrayList<glm::vec2> buffer;

		Font font;
	};

	extern Context* context;
	extern Engine::Renderer2D::Wrapper renderer2D;
}