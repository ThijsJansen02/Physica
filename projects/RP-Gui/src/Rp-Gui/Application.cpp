#ifdef _MSVC_LANG
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <Platform/PlatformAPI.h>
//#include <Platform/PlatformAPI.cpp>
#include <Base/Log.h>

#include <Engine/Rendering.h>
#include <Engine/Engine.h>

namespace PH::RpGui {

	using namespace PH::Platform;

	CONSOLE_WRITE(consoleWrite) {
		Platform::consoleWrite(str);
	}


	PH::Base::LogStream<consoleWrite> INFO;
	PH::Base::LogStream<consoleWrite> WARN;
	PH::Base::LogStream<consoleWrite> ERR;


	void beginFinalRenderpass(uint32 windowwidth, uint32 windowheight) {

		GFX::RenderpassbeginInfo renderpassbegin{};
		renderpassbegin.description = 0;
		renderpassbegin.framebuffer = 0;
		renderpassbegin.renderarea = { windowwidth, windowheight };

		GFX::ClearValue clearcolor = { 0.5f, 0.5f, 0.5f, 1.0f };
		renderpassbegin.clearvalues = { &clearcolor, 1 };

		GFX::beginRenderpass(&renderpassbegin);

		GFX::Viewport viewport;
		viewport.width = (real32)windowwidth;
		viewport.height = (real32)windowheight;
		viewport.x = 0;
		viewport.y = 0;

		GFX::setViewports(&viewport, 1);

		GFX::Scissor scissor;
		scissor.width = windowwidth;
		scissor.height = windowheight;
		scissor.x = 0;
		scissor.y = 0;

		GFX::setScissors(&scissor, 1);
	}

	void endFinalRenderpass() {
		GFX::endRenderpass();
	}

	struct Context {
		Engine::Renderer2D::Context* renderer2D;
	};
}




void* imguiAllocator(size_t sz, void* userdata) {
	return PH::Engine::Allocator::alloc(sz);
}

void imguiDeallocator(void* data, void* userdata) {
	PH::Engine::Allocator::dealloc(data);
}

using namespace PH::RpGui;

extern "C" __declspec(dllexport) PH_APPLICATION_INITIALIZE(applicationInitialize) {

	PH::RpGui::INFO << "intializing Rp-Gui application...\n";

	PH::RpGui::Context* appcontext = (PH::RpGui::Context*)context.appmemory;

	//sets up the engine allocators and other systems that rely on the engine allocator, such as the console log stream
	PH::Engine::EngineInitInfo engineinit{};
	engineinit.memory = (PH::uint8*)context.appmemory + sizeof(PH::RpGui::Context);
	engineinit.memorysize = context.appmemsize - sizeof(PH::RpGui::Context);
	PH::Engine::init(engineinit);
	
	//renderpass is finalrenderpass
	PH::Engine::Renderer2D::InitInfo init{};
	init.renderpass = 0;
	init.defaultpipeline = PH_GFX_NULL;
	init.shadowmapdimensions = { 1024 };
	init.instancebuffersize = MEGA_BYTE;
	appcontext->renderer2D = PH::Engine::Renderer2D::createContext(init);
	
	//set the Imgui context in this translation unit
	ImGui::SetCurrentContext(context.imguicontext);
	ImGui::SetAllocatorFunctions(imguiAllocator, imguiDeallocator, nullptr);

	//this is just a stub application that does nothing, the real application is in the editor module
	return true;
}


PH_DLL_EXPORT PH_APPLICATION_UPDATE(applicationUpdate) {

	PH::Engine::beginNewFrame();	

	PH::RpGui::beginFinalRenderpass(context.windowwidth, context.windowheight);
	bool open = true;
	if(ImGui::Begin("stats", &open)) {
		ImGui::Text("Frame rate: %f", 1.0f / PH::Engine::getTimeStep());
	} ImGui::End();

	ImGui::Render();
	PH::Platform::GFX::drawImguiWidgets(ImGui::GetDrawData());
	PH::RpGui::endFinalRenderpass();
	return true;
}

PH_DLL_EXPORT PH_APPLICATION_DESTROY(applicationDestroy) {

	PH::RpGui::INFO << "destroying Rp-Gui application...\n";
	return true;
}