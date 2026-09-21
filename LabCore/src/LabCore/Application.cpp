
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
	PH::Engine::Events::startNewFrame();
	
	auto parentdisplay = PH::Engine::getParentDisplay();
	
	PH::Engine::beginRenderPass(*parentdisplay);
	
	/*
	renderer2D->begin();

	renderer2D->pushProjection(glm::mat4(1.0f));
	renderer2D->pushView(glm::mat4(1.0f));

	renderer2D->drawColoredQuad(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

	renderer2D->end();
	renderer2D->flush({nullptr, 0});
	*/

	PH::Engine::BeginDockspace();

	PH::LabCore::update();
	PH::LabCore::draw();
	
	PH::Engine::EndDockspace();

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


