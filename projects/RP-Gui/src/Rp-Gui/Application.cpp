#ifdef _MSVC_LANG
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <Platform/PlatformAPI.h>
//#include <Platform/PlatformAPI.cpp>
#include <Base/Log.h>

#include <Engine/cppAPI/Rendering.hpp>
#include <Engine/Engine.h>

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


void drawLine(glm::vec3 v1, glm::vec3 v2, real32 thickness) {

	glm::vec3 midpoint = (v1 + v2) * 0.5f;

	glm::vec3 up = {0.0f, 0.0f, 1.0f};
	glm::vec3 tangent = (v2 - v1) + glm::normalize(v2-v1) * thickness * 0.5f;
	glm::vec3 bitangent = glm::cross(glm::normalize(tangent), up) * thickness;

	glm::mat3 rotscale = glm::mat3(tangent, bitangent, up);

	glm::mat4 transform = glm::mat4(rotscale);
	transform[3] = { midpoint, 1.0f };

	RpGui::renderer2D.drawColoredQuad(transform, { 1.0f, 1.0f, 1.0f, 1.0f });
}

void drawLineStrip(Base::Array<glm::vec2> points, real32 thickness) {

	for (uint32 i = 1; i < points.count; i++) {
		drawLine({ points[i - 1], 0.0f }, { points[i], 0.0f }, thickness);
	}
}

real32 function(real32 x, real32 freq) {
	return 20 * sin(2.0f * 3.1416f * freq * x);
}


PH_DLL_EXPORT PH_APPLICATION_UPDATE(applicationUpdate) {

	PH::Engine::beginNewFrame(&context);
	PH::Engine::beginRenderPass(*Engine::getParentDisplay());

	real32 left = -100.0f;
	real32 right = 100.0f;
	real32 top = 100.0f;
	real32 bottom = -100.0f;

	RpGui::renderer2D.setView(glm::mat4(1.0f));
	RpGui::renderer2D.setProjection(glm::ortho(left, right, top, bottom));
	RpGui::renderer2D.begin();
	
	real32 dx = 0.1f;

	RpGui::context->buffer.clear();

	static real32 freq = 0.1f;

	for (real32 x = left; x < right; x += dx) {
		glm::vec2 coordinate = { x, function(x, freq) };
		RpGui::context->buffer.pushBack(coordinate);
	}

	static real32 linethickness = 0.5f;
	drawLine({ left, 0.0f, 0.0f }, { right, 0.0f, 0.0f }, linethickness);
	drawLine({ 0.0f, top, 0.0f }, { 0.0f, bottom, 0.0f }, linethickness);
	drawLineStrip(RpGui::context->buffer.getArray(), linethickness);
	//drawLine({ 10.0f, 10.0f, 0.0f }, { 500.0f, 500.0f, 0.0f }, 2.0f);

	RpGui::renderer2D.end();
	

	RpGui::renderer2D.flush({ nullptr, 0 });


	bool open = true;
	if(ImGui::Begin("function", &open)) {
		ImGui::DragFloat("frequency", &freq, 0.001f,1.0f);
		ImGui::DragFloat("linethickness", &linethickness, 0.01, 1.0f);
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