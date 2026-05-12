#pragma once
#include <Platform/PlatformAPI.h>
#include <Base/Log.h>
#include <Engine/cppAPI/Rendering.hpp>

namespace PH::RpGui {

	#define M_PI 3.14159265359

	static real64 targetfs = 125e6 / 256.0;

	using namespace PH::Platform;
	struct Context;

	inline CONSOLE_WRITE(consoleWrite) {
		Platform::consoleWrite(str);
	}

	extern PH::Base::LogStream<consoleWrite> INFO;
	extern PH::Base::LogStream<consoleWrite> WARN;
	extern PH::Base::LogStream<consoleWrite> ERR;

	extern Context* context;
	extern Engine::Renderer2D::Wrapper renderer2D;
}