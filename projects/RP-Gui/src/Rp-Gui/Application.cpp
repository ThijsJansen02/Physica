#ifdef _MSVC_LANG
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <Platform/PlatformAPI.h>
#include <Base/Log.h>

#include <Engine/cppAPI/Rendering.hpp>
#include <Engine/Engine.h>
#include <Engine/Events.h>

#include "Text.h"
#include "Plot.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>


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
		PH::Platform::GFX::GraphicsPipeline fontpipeline2D;

		Engine::ArrayList<glm::vec2> buffer;

		Font font;
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

	char psBuffer[128];
	FILE* file;
	
	Engine::INFO << "starting ssh process...\n";
	if ((file = _popen("ssh root@rp-f083c2.local", "rt")) == NULL) {
		Engine::ERR << "could not open process!\n";
		exit(1);
	}
	Engine::INFO << "process opened successfully, reading output...\n";
	
	while (fgets(psBuffer, 128, file))
	{
		Engine::INFO << psBuffer;
	}



	int endOfFileVal = feof(file);
	int closeReturnVal = _pclose(file);

	if (endOfFileVal)
	{
		printf("\nProcess returned %d\n", closeReturnVal);
	}
	else
	{
		printf("Error: Failed to read the pipe to the end.\n");
	}


	RpGui::context->font = RpGui::loadFont("c:/windows/fonts/times.ttf", 512, 32.0f);

	RpGui::context->pipeline2D = Engine::Renderer2D::createGraphicsPipelineFromGLSLSource(Engine::getParentDisplay(), "res/shaders/default_quadshader.vert", "res/shaders/default_quadshader.frag", {nullptr, 0});
	RpGui::context->fontpipeline2D = Engine::Renderer2D::createGraphicsPipelineFromGLSLSource(Engine::getParentDisplay(), "res/shaders/default_fontshader.vert", "res/shaders/default_fontshader.frag", { &RpGui::fontuserlayout, 1 });
	

	Engine::Renderer2D::InitInfo init{};
	init.currentpipeline = RpGui::context->pipeline2D;
	init.descriptorsetlayouts = { nullptr, 0 };
	init.instancebuffersize = 1 * MEGA_BYTE;
	init.shadowmapdimensions = 0;

	RpGui::context->buffer = Engine::ArrayList<glm::vec2>::create(10);

	RpGui::renderer2D = Engine::Renderer2D::Wrapper::create(init);
	return true;
}


struct FunctionParams {
	real32 cutoff;
	real32 Qfactor;
	real32 gain;
};

real32 bandpass(real32 x, void* params_) {

	FunctionParams* params = (FunctionParams*)params_;
	return (x * x + params->cutoff * params->cutoff) / (x * x + ((x * params->cutoff) / params->Qfactor) + params->cutoff * params->cutoff);
}


PH_DLL_EXPORT PH_APPLICATION_UPDATE(applicationUpdate) {

	PH::Engine::beginNewFrame(&context);
	PH::Engine::Events::startNewFrame();

	real32 scrollspeed = 0.001f;
	static RpGui::Box2D range = { 0.0f, 0.0f, 10.0f, 5.0f };

	//update all events events;
	for (const auto& event : context.events) {
		PH::Engine::Events::onEvent(event);

		if (event.type == PH_EVENT_TYPE_MOUSE_SCROLLED) {
			int16 delta = (int16)(void*)event.lparam;

			real32 zoom = 1.0f + (real32)delta * scrollspeed;

			glm::vec2 mousepos = Engine::Events::getMousePos();
			mousepos.y = context.windowheight - mousepos.y; //flip y coordinate because the window coordinate system has y going down and the plot coordinate system has y going up

			//Engine::INFO << "mouse pos: " << mousepos.x << ", " << mousepos.y << "\n";	

			if (PH::Engine::Events::isKeyPressed(PH_CONTROL)) {
				range.right *= zoom;
				range.left *= zoom;
			}
			else {
				range.top *= zoom;
				range.bottom *= zoom;
			}
		}
	}

	RpGui::Box2D region;
	region.left = 0.0f;
	region.right = (real32)context.windowwidth;
	region.bottom = 0.0f;
	region.top = (real32)context.windowheight;
	
	auto& io = ImGui::GetIO();

	glm::vec2 mousefactor = { (range.right - range.left) / (region.right - region.left), (range.top - range.bottom) / (region.top - region.bottom)};
	if (!io.WantCaptureMouse && PH::Engine::Events::isMouseButtonPressed(PH_LMBUTTON)) {

		glm::vec2 deltam = PH::Engine::Events::getDeltaMousePos();

		range.left -= mousefactor.x * deltam.x;
		range.right -= mousefactor.x * deltam.x;
		range.top += mousefactor.y * deltam.y;
		range.bottom += mousefactor.y * deltam.y;
	}
	

	
	RpGui::context->buffer.clear();

	static int32 nsamples = 1000;
	
	real32 xrange = range.right - range.left;
	real32 dx = xrange / (real32)nsamples;

	static FunctionParams params = { 100.0f, 4.0f, 10.0f };
	
	for (real32 x = range.left; x <= range.right; x += dx) {

		glm::vec2 coordinate = { x, 20 * log10(bandpass(powf(10.0f, x), (void*)&params))};
		RpGui::context->buffer.pushBack(coordinate);
	}
	

	PH::Engine::beginRenderPass(*Engine::getParentDisplay());

	glm::vec2 linethickness = {1.0f, 1.0f};

	RpGui::renderer2D.begin();
	
	//start drawing the plot, first set the pipeline and the view and projection matrices, then draw the background and the plot itself, then end the renderer and flush it to the GPU
	RpGui::renderer2D.pushGraphicsPipeline(RpGui::context->pipeline2D);
	RpGui::renderer2D.setView(glm::mat4(1.0f));
	RpGui::renderer2D.setProjection(glm::ortho(0.0f, (real32)context.windowwidth, (real32)context.windowheight, 0.0f));
	
	drawPlotScaleLines(range, region);
	drawPlot(RpGui::context->buffer.getArray(), range, region);
	//RpGui::renderer2D.drawTexturedQuad({ 500.0f, 500.0f, 0.0f }, { 1000.0f, 1000.0f }, RpGui::context->font.atlas);
	
	static real32 textscale = 0.5f;

	//start drawing the text
	RpGui::renderer2D.pushGraphicsPipeline(RpGui::context->fontpipeline2D, { &RpGui::context->font.cdata, 1 });
	RpGui::renderer2D.pushTexture(RpGui::context->font.atlas);
	RpGui::drawPlotScaleValues(range, region, &RpGui::context->font, textscale);


	RpGui::renderer2D.end();
	
	RpGui::renderer2D.flush({ nullptr, 0 });

	real32 speed = 0.01;

	static bool open = true;
	if(ImGui::Begin("function", &open)) {
		ImGui::DragFloat("cutoff", &params.cutoff, params.cutoff * speed);
		ImGui::DragFloat("Qfactor", &params.Qfactor, params.Qfactor * speed);
		ImGui::DragFloat("gain", &params.gain, params.gain * speed);
		ImGui::DragInt("nsamples", &nsamples, 1.0f, 0, 2000);
	} ImGui::End();

	static bool statsopen;
	if (ImGui::Begin("stats")) {
		ImGui::Text("framerate %f", 1.0f / Engine::getTimeStep());
		ImGui::DragFloat("text scale", &textscale, textscale * speed);
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