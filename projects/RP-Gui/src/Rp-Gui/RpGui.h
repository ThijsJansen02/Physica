#pragma once
#include <Platform/PlatformAPI.h>
#include <Base/Log.h>
#include <Engine/cppAPI/Rendering.hpp>

namespace PH::RpGui {

	#define M_PI 3.141592653589793238462643383279502884197169399
	#define RP_FPGA_SAMPLERATE 125000000

	static real64 targetfs = 125e6 / 128.0;
	static uint32 standard_decimation = 256;

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