#pragma once
#include <Platform/PlatformAPI.h>
#include <Base/Log.h>
#include <Engine/cppAPI/Rendering.hpp>

#include <Engine/YamlExtensions.h>

namespace PH::RpGui {

	#define M_PI 3.141592653589793238462643383279502884197169399
	#define RP_FPGA_SAMPLERATE 125000000

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