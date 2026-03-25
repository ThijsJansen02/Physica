#ifdef _MSVC_LANG
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <Platform/PlatformAPI.h>
//#include <Platform/PlatformAPI.cpp>
#include <Base/Log.h>

#include <Engine/Rendering.h>
#include <Engine/Engine.h>
#include <Engine/Rendering.h>

namespace PH::RpGui {

	using namespace PH::Platform;

	CONSOLE_WRITE(consoleWrite) {
		Platform::consoleWrite(str);
	}


	PH::Base::LogStream<consoleWrite> INFO;
	PH::Base::LogStream<consoleWrite> WARN;
	PH::Base::LogStream<consoleWrite> ERR;

	struct Context {
		Engine::Renderer2D::Context* renderer2D;
		PH::Platform::GFX::GraphicsPipeline pipeline2D;
	};

	Context* context;
}

using namespace PH;

PH_DLL_EXPORT PH_APPLICATION_INITIALIZE(applicationInitialize) {

	//sets up the engine allocators and other systems that rely on the engine allocator, such as the console log stream
	PH::Engine::EngineInitInfo engineinit{};
	engineinit.memory = (PH::uint8*)context.appmemory;
	engineinit.memorysize = context.appmemsize;
	engineinit.platformcontext = &context;
	PH::Engine::init(engineinit);

	RpGui::context = (RpGui::Context*)Engine::Allocator::alloc(sizeof(RpGui::Context));

	RpGui::context->pipeline2D = Engine::Renderer2D::createGraphicsPipelineFromGLSLSource(Engine::getParentDisplay(), "res/shaders/default_quadshader.vert", "res/shaders/default_quadshader.frag");
	
	Engine::Renderer2D::InitInfo init{};
	init.currentpipeline = RpGui::context->pipeline2D;
	init.descriptorsetlayouts = { nullptr, 0 };
	init.instancebuffersize = 1 * MEGA_BYTE;
	init.shadowmapdimensions = 0;

	RpGui::context->renderer2D = Engine::Renderer2D::createContext(init);
	return true;
}


PH_DLL_EXPORT PH_APPLICATION_UPDATE(applicationUpdate) {

	PH::Engine::beginNewFrame(&context);
	PH::Engine::beginRenderPass(*Engine::getParentDisplay());

	Engine::Renderer2D::setView(RpGui::context->renderer2D, glm::mat4(1.0f));
	Engine::Renderer2D::setProjection(RpGui::context->renderer2D, glm::mat4(1.0f));

	Engine::Renderer2D::begin(RpGui::context->renderer2D);
	Engine::Renderer2D::drawColoredQuad({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f, 1.0f, 1.0f }, RpGui::context->renderer2D);
	Engine::Renderer2D::end(RpGui::context->renderer2D);

	//Engine::Renderer2D::end(RpGui::context->renderer2D);

	Engine::Renderer2D::flush(RpGui::context->renderer2D, { nullptr, 0 });

	bool open = true;
	if(ImGui::Begin("renderstats", &open)) {
		ImGui::Text("Frame rate: %f", 1.0f / PH::Engine::getTimeStep());
	} ImGui::End();

	ImGui::Render();
	PH::Platform::GFX::drawImguiWidgets(ImGui::GetDrawData());
	PH::Engine::endRenderPass(*Engine::getParentDisplay());
	return true;
}

PH_DLL_EXPORT PH_APPLICATION_DESTROY(applicationDestroy) {

	PH::RpGui::INFO << "destroying Rp-Gui application...\n";
	return true;
}