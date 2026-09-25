
#include "LabCore.h"
#include <Engine/imgui/DockSpace.h>
#include <Platform/platformAPI.h>

#include "coreplugins/initcoreplugins.h"

namespace PH::LabCore {

	
	/*
	PH::Base::LogStream<consoleWrite> INFO;
	PH::Base::LogStream<consoleWrite> WARN;
	PH::Base::LogStream<consoleWrite> ERR;
	*/


	//global context for the application
	AppContext* appcontext = nullptr;

	//global renderer for the application
	PH::Engine::Renderer2D::Wrapper* renderer2D = nullptr;
	
}

using namespace PH::LabCore;

PH_DLL_EXPORT PH_APPLICATION_INITIALIZE(applicationInitialize)
{
	//init the render engine
	PH::Engine::EngineInitInfo engineinit{};
	engineinit.memory = (PH::uint8*)context.appmemory;
	engineinit.memorysize = context.appmemsize;
	engineinit.platformcontext = &context;

	PH::Engine::init(engineinit);

	PH::LabCore::appcontext = (AppContext*)PH::Engine::Allocator::alloc(sizeof(AppContext));
	PH::LabCore::init();
	PH::LabCore::initCorePlugins();

	deserializeProject("project.lcproj");

	return true;
}

PH_DLL_EXPORT PH_APPLICATION_UPDATE(applicationUpdate)
{
	PH::Engine::beginNewFrame(&context);

	for (auto e : context.events) {
		PH::Engine::Events::onEvent(e);
		PH::LabCore::interpretEventForViews(PH::LabCore::appcontext->viewinstances, e);
	}
	
	auto parentdisplay = PH::Engine::getParentDisplay();
	
	PH::Engine::BeginDockspace();
	PH::LabCore::draw();
	PH::LabCore::update();
	PH::Engine::EndDockspace();

	
	//final renderpass, this is where the imgui draw data is rendered to the screen
	PH::Engine::beginRenderPass(*parentdisplay);

	//imgui render stuff
	ImGui::Render();
	PH::Platform::GFX::drawImguiWidgets(ImGui::GetDrawData());

	PH::Engine::endRenderPass(*parentdisplay);

	return true;
}



PH_DLL_EXPORT PH_APPLICATION_DESTROY(applicationDestroy)
{
	PH::LabCore::destroy();
	return true;
}


