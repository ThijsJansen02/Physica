#ifdef _MSVC_LANG
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <Platform/PlatformAPI.h>
//#include <Platform/PlatformAPI.cpp>
#include <Base/Log.h>

#include <Engine/cppAPI/Rendering.hpp>
#include <Engine/Engine.h>
#include <Engine/Events.h>

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

		Engine::ArrayList<glm::vec2> buffer;
	};

	Context* context;

	Engine::Renderer2D::Wrapper renderer2D;
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

	RpGui::context->buffer = Engine::ArrayList<glm::vec2>::create(10);

	RpGui::renderer2D = Engine::Renderer2D::Wrapper::create(init);
	return true;
}


real32 function(real32 x, real32 freq) {
	return 20 * sin(2.0f * 3.1416f * freq * x);
}

real32 left = -100.0f;
real32 right = 100.0f;
real32 top = 100.0f;
real32 bottom = -100.0f;

PH_DLL_EXPORT PH_APPLICATION_UPDATE(applicationUpdate) {

	PH::Engine::beginNewFrame(&context);
	PH::Engine::Events::startNewFrame();

	//update events;
	for (const auto& event : context.events) {
		PH::Engine::Events::onEvent(event);
	}

	glm::vec2 mousefactor = { 0.1f, 0.1f };

	if (PH::Engine::Events::isMouseButtonPressed(PH_LMBUTTON)) {
		glm::vec2 deltam = PH::Engine::Events::getDeltaMousePos();

		Engine::INFO << "hit middle mouse button\n";

		left -= mousefactor.x * deltam.x;
		right -= mousefactor.x * deltam.x;
		top += mousefactor.y * deltam.y;
		bottom += mousefactor.y * deltam.y;
	}
	

	RpGui::renderer2D.setView(glm::mat4(1.0f));
	RpGui::renderer2D.setProjection(glm::ortho(left, right, top, bottom));
	RpGui::renderer2D.begin();
	
	real32 xrange = right - left;
	real32 dx = xrange / (real32)context.windowwidth;
	real32 yrange = top - bottom;
	real32 dy = yrange / (real32)context.windowheight;

	RpGui::context->buffer.clear();

	static real32 freq = 0.1f;

	for (real32 x = left; x < right; x += dx) {
		glm::vec2 coordinate = { x, function(x, freq) };
		RpGui::context->buffer.pushBack(coordinate);
	}


	PH::Engine::beginRenderPass(*Engine::getParentDisplay());

	glm::vec2 linethickness = {dx*1.0f, dy*1.0f};
	RpGui::renderer2D.drawLine({ left, 0.0f, 0.0f }, { right, 0.0f, 0.0f }, {1.0f, 0.0f, 0.0f, 1.0f}, linethickness);
	RpGui::renderer2D.drawLine({ 0.0f, top, 0.0f }, { 0.0f, bottom, 0.0f }, {1.0f, 0.0f, 0.0f, 1.0f}, linethickness);
	RpGui::renderer2D.drawLineStrip(RpGui::context->buffer.getArray(), linethickness);

	RpGui::renderer2D.end();
	

	RpGui::renderer2D.flush({ nullptr, 0 });


	bool open = true;
	if(ImGui::Begin("function", &open)) {
		ImGui::DragFloat("frequency", &freq, 0.001f,1.0f);
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