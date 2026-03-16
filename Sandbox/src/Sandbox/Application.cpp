
#include "Platform/platformAPI.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <Engine/Engine.h>
#include <Engine/Rendering.h>
#include <Engine/Scene.h>
#include <Engine/Components.h>

#define EOF (-1)

using namespace PH::Platform;
using namespace PH;

namespace PH::Sandbox {

}

Engine::Renderer2D::Context* rendercontext;
Engine::Scene::Instance* sceneinstance;

struct translation {
	glm::mat3 transform;
};


PH_DLL_EXPORT PH_APPLICATION_INITIALIZE(applicationInitialize)
{

	ImGui::SetCurrentContext(context.ìmguicontext);

	//initing the engine
	Engine::EngineInitInfo engineinit{};
	engineinit.memory = context.appmemory;
	engineinit.memorysize = context.appmemsize;
	Engine::Init(engineinit);

	sceneinstance = Engine::Scene::createInstance();
	PH::Platform::GFX::DescriptorSetLayout dsls[] = { Engine::Scene::getSceneDescriptorSetLayout(sceneinstance) };

	//initing the 2d renderer
	Engine::Renderer2D::InitInfo rendererinit{};
	rendererinit.instancebuffersize = KILO_BYTE * 64;
	rendererinit.defaultfragpath = "res/shaders/default_quadfrag.spv";
	rendererinit.defaultvertpath = "res/shaders/default_quadvert.spv";
	rendererinit.descriptorsetlayouts = { dsls, 1 };
	rendercontext = Engine::Renderer2D::createContext(rendererinit);

	return true;
}

PH_DLL_EXPORT PH_APPLICATION_DESTROY(applicationDestroy)
{
	PH::Platform::consoleWrite("application destroyed!\n");
	return true;
}

bool demowindowopen = true;

void BeginDockspace() {

	static bool open;
	static bool opt_fullscreen = true;
	static bool opt_padding = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}
	else
	{
		dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
	}

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
	// and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	if (!opt_padding)
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace Demo", &open, window_flags);
	if (!opt_padding)
		ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	// Submit the DockSpace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}
}
void EndDockspace() {
	ImGui::End();
}

using namespace PH;

PH_DLL_EXPORT PH_APPLICATION_UPDATE(applicationUpdate)
{
	static real32 rotation = 0.0f;
	static real32 scale = 1.0f;

	//BeginDockspace();

	if (ImGui::Begin("demo window", &demowindowopen)) {
		ImGui::DragFloat("rotation:", &rotation, 1.0f, 0.0f, 360.0f);
		ImGui::DragFloat("scale:", &scale, 0.01f);
		ImGui::End();
	}

	//EndDockspace();

	//ImGui::ShowDemoWindow();

	ImGui::Render();

	GFX::RenderpassbeginInfo begininfo;
	begininfo.description = 0;
	begininfo.framebuffer = 0;

	GFX::beginRenderpass(&begininfo);

	GFX::Viewport viewport;
	viewport.width = (real32)context.windowwidth;
	viewport.height = (real32)context.windowheight;
	viewport.x = 0;
	viewport.y = 0;

	GFX::setViewports(&viewport, 1);	

	GFX::Scissor scissor;
	scissor.width = context.windowwidth;
	scissor.height = context.windowheight;
	scissor.x = 0;
	scissor.y = 0;

	GFX::setScissors(&scissor, 1);


	//maybe change to Engine::renderer::drawScene?? is more in line with function programming
	//Engine::Scene::render(rendercontext, sceneinstance, );

	GFX::drawImguiWidgets(ImGui::GetDrawData());

	GFX::endRenderpass();

	return true;
}