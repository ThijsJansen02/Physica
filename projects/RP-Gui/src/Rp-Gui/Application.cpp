#ifdef _MSVC_LANG
#define _CRT_SECURE_NO_WARNINGS
#endif

//this define is needed to use libssh as a static library on windows, because otherwise the functions will be exported with __declspec(dllimport) which will cause linker errors when trying to link against the static library
//libssh should be included as the first header to avoid any issues with the windows.h header that is included by the platform layer, because windows.h defines some macros that can cause issues with the libssh headers if they are included after windows.h
#define LIBSSH_STATIC 1
#include <libssh/libssh.h>

#include "RpGui.h"

#include <Platform/PlatformAPI.h>
#include <Base/Log.h>

#include <Engine/cppAPI/Rendering.hpp>
#include <Engine/Engine.h>
#include <Engine/Events.h>
#include <Engine/imgui/DockSpace.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "Text.h"
#include "Plot.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

using namespace PH;

namespace PH::RpGui {
	Context* context = nullptr;
	Engine::Renderer2D::Wrapper renderer2D;

	PH::Base::LogStream<consoleWrite> INFO;
	PH::Base::LogStream<consoleWrite> WARN;
	PH::Base::LogStream<consoleWrite> ERR;
}



PH_DLL_EXPORT PH_APPLICATION_INITIALIZE(applicationInitialize) {

	//sets up the engine allocators and other systems that rely on the engine allocator, such as the console log stream
	PH::Engine::EngineInitInfo engineinit{};
	engineinit.memory = (PH::uint8*)context.appmemory;
	engineinit.memorysize = context.appmemsize;
	engineinit.platformcontext = &context;
	PH::Engine::init(engineinit);

#if 0 //libssh test
	ssh_init();
	ssh_session my_ssh_session = NULL;
	int rc;

	my_ssh_session = ssh_new();
	if (my_ssh_session == NULL)
		exit(-1);

	ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, "localhost");

	rc = ssh_connect(my_ssh_session);
	if (rc != SSH_OK)
	{
		Engine::ERR << "Error connecting to localhost: " << ssh_get_error(my_ssh_session) << "\n";
		//exit(-1);
	}
	else {
		ssh_disconnect(my_ssh_session);
	}
	ssh_free(my_ssh_session);
#endif


	RpGui::context = (RpGui::Context*)Engine::Allocator::alloc(sizeof(RpGui::Context));

	RpGui::context->plotviewpanel = RpGui::PlotViewPanel::create({ -10.0f, -10.0f, 10.0f, 10.0f });

	RpGui::context->font = RpGui::loadFont("c:/windows/fonts/times.ttf", 512, 32.0f);
	RpGui::context->pipeline2D = Engine::Renderer2D::createGraphicsPipelineFromGLSLSource(&RpGui::context->plotviewpanel.display, "res/shaders/default_quadshader.vert", "res/shaders/default_quadshader.frag", {nullptr, 0});
	RpGui::context->fontpipeline2D = Engine::Renderer2D::createGraphicsPipelineFromGLSLSource(&RpGui::context->plotviewpanel.display, "res/shaders/default_fontshader.vert", "res/shaders/default_fontshader.frag", { &RpGui::fontuserlayout, 1 });
	
	Engine::Renderer2D::InitInfo init{};
	init.currentpipeline = RpGui::context->pipeline2D;
	init.descriptorsetlayouts = { nullptr, 0 };
	init.instancebuffersize = 8 * MEGA_BYTE;
	init.shadowmapdimensions = 0;

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigWindowsMoveFromTitleBarOnly = true;


	RpGui::context->buffer = Engine::ArrayList<glm::vec2>::create(10);

	RpGui::renderer2D = Engine::Renderer2D::Wrapper::create(init);
	return true;
}

PH_DLL_EXPORT PH_APPLICATION_UPDATE(applicationUpdate) {

	PH::Engine::beginNewFrame(&context);
	PH::Engine::Events::startNewFrame();


	real32 scrollspeed = 0.001f;
	static RpGui::Box2D range = { 0.0f, 0.0f, 10.0f, 5.0f };

	//update all events events;
	for (auto& event : context.events) {
		PH::Engine::Events::onEvent(event);
		RpGui::context->plotviewpanel.onEvent(&event);
	}

	auto& io = ImGui::GetIO();


	RpGui::renderer2D.begin();

	PH::Engine::BeginDockspace();

	ImGui::BeginMenuBar();
	ImGui::MenuItem("File");
	ImGui::EndMenuBar();

	RpGui::context->plotviewpanel.draw();
	RpGui::context->plotviewpanel.ImGuiDraw();

	PH::Engine::beginRenderPass(*Engine::getParentDisplay());

	static bool statsopen;
	if (ImGui::Begin("stats")) {
		ImGui::Text("framerate %f", 1.0f / Engine::getTimeStep());
		ImGui::Text("mousepos %f, %f", Engine::Events::getMousePos().x, Engine::getParentDisplay()->viewport.y - Engine::Events::getMousePos().y);
		ImGui::Text("viewpanel topleft: %f, %f", RpGui::context->plotviewpanel.panelregion.left, RpGui::context->plotviewpanel.panelregion.top);
	} ImGui::End();

	PH::Engine::EndDockspace();

	//render the imgui widgetes to the screen
	ImGui::Render();
	PH::Platform::GFX::drawImguiWidgets(ImGui::GetDrawData());



	PH::Engine::endRenderPass(*Engine::getParentDisplay());
	RpGui::renderer2D.end();
	
	return true;
}

PH_DLL_EXPORT PH_APPLICATION_DESTROY(applicationDestroy) {

	PH::RpGui::INFO << "destroying Rp-Gui application...\n";
	return true;
}