#ifdef _MSVC_LANG
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <Platform/platformAPI.h>
#include "ImGuiPanels/ComponentViewPanel.h"
#include "ImGuiPanels/InspectorPanel.h"
#include "ImGuiPanels/SceneViewPanel.h"
#include "Editor.h"
#include "ImGuiUtils.h"
#include <Engine/Engine.h>
#include <Engine/Scene.h>
#include <Engine/Components.h>
#include "yaml-cpp/yaml.h"
#include <Engine/Events.h>
#include <Engine/YamlExtensions.h>
#include <Engine/AssetLibrary.h>
#include <Engine/assets/Material.h>
#include <Engine/assets/Texture.h>
#include "ImGuiPanels/ComponentViewPanel.h"
#include "ImGuiPanels/MaterialViewPanel.h"

#include <Windows.h>

using namespace PH;
using namespace PH::Platform;

namespace PH::Editor {

	void serialize(Instance* instance, const char* filepath);
	bool32 deserialize(Instance* instance, const char* filepath);

	void beginFinalRenderpass(uint32 windowwidth, uint32 windowheight);
	void endFinalRenderpass();

	Instance createInstance(const char* projectpath);


	void drawMenuBar(Editor::Instance* instance) {

		static char pathbuffer[256];
		static char namebuffer[256];
		static char targetdir[256];

		ImGuiID popupid = ImGui::GetID("importpopup");
		if (ImGui::BeginPopup("importpopup")) {

			ImGui::Text("Importing asset: %s", pathbuffer);
			ImGui::InputText("name", namebuffer, 256);
			ImGui::InputText("directory", targetdir, 256);

			if (ImGui::Button("import")) {
				Base::SubString path = pathbuffer;
				Base::SubString extension = path.getFromLast('.');

				if (extension == ".fbx" || extension == ".blend") {
					Mesh mesh{};
					Material* mastermaterial = instance->assets->getLoadedByReference<Material>("default_material");
					if (!mastermaterial) {
						Engine::WARN << "no defaultmaterial available!\n";
					}


					if (mastermaterial) {
						importMesh(path.getC_Str(), targetdir, &mesh, mastermaterial, instance->assets);
					}
					instance->assets->add<Mesh>(namebuffer, targetdir, mesh);
				}

				if (extension == ".cpp") {

					Script script{};
					if (!Engine::Assets::importScriptFromCppFile(path.getC_Str(), "res\\scriptbinaries", &script)) {
						ImGui::EndPopup();
						return;
					}

					instance->assets->add<Script>(namebuffer, targetdir, script);

				}

				if (extension == ".jpg" || extension == ".png") {
					instance->assets->import<Engine::Assets::TextureImage>(pathbuffer, targetdir, namebuffer);
				}

				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}


		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open Project...", "Ctrl+O")) {

				}


				if (ImGui::MenuItem("Import Asset", "Ctrl+O")) {


					PH::Platform::OpenFileDialogInfo info{};
					info.filter = "*.png;*.jpg;*.fbx;*.blend";
					info.read = true;
					info.resultbuffer = pathbuffer;
					info.resultbuffersize = 256;

					if (PH::Platform::openFileDialog(info)) {

						ImGui::OpenPopup(popupid);

						Base::SubString path = pathbuffer;
						Base::SubString extension = path.getFromLast('.');
						Base::SubString name = path.getFromLast('/').moveHead(1).getTill('.') - 1;
						Base::SubString dir = path.getTillLast('/');

						//this should become safer for working with large paths
						Base::subStringCopy(name, namebuffer, 256);
						Base::subStringCopy(dir, targetdir, 256);
					}
				}

				if (ImGui::MenuItem("Add Asset", "Ctrl+O")) {

					char buffer[1024];

					PH::Platform::OpenFileDialogInfo info{};
					info.filter = "*.phmesh;*.phmat;*.phmatinst;*.phtex";
					info.read = true;
					info.resultbuffer = buffer;
					info.resultbuffersize = 1024;


					if (PH::Platform::openFileDialog(info)) {
						Base::SubString path = buffer;
						Base::SubString extension = path.getFromLast('.');

						if (extension == ".phmesh") {
							instance->assets->load<Engine::Assets::Mesh>(buffer);
						}

						if (extension == ".phtex") {
							instance->assets->load<Engine::Assets::TextureImage>(buffer);
						}

						if (extension == ".phmat") {
							instance->assets->load<Engine::Assets::Material>(buffer);
						}

						if (extension == ".phmatinst") {
							instance->assets->load<Engine::Assets::MaterialInstance>(buffer);
						}

					}

				}

				ImGui::Separator();

				if (ImGui::MenuItem("New Scene", "Ctrl+N")) {

				}

				if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {

				}

				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S")) {

				}

				ImGui::Separator();

				if (ImGui::MenuItem("Exit")) {

				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Script"))
			{
				if (ImGui::MenuItem("Reload assembly", "Ctrl+R")) {

					for (auto script : instance->assets->getAll<Script>()) {
						if (script->status == Engine::Assets::AssetStatus::LOADED) {
							Engine::Assets::unloadScript(script);
							Engine::Assets::compileCPPscript(script);
							Engine::Assets::loadScript(script);
						}
					}
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
	}


	namespace API {
		void setCurrentInstance(Instance* instance);
	}
}

void* imguiAllocator(size_t sz, void* userdata) {
	return Engine::Allocator::alloc(sz);
}

void imguiDeallocator(void* data, void* userdata) {
	Engine::Allocator::dealloc(data);
}

PH_DLL_EXPORT PH_APPLICATION_INITIALIZE(applicationInitialize) {

	//the only reference I plan to use because i might convert this to a stack variable
	Engine::Events::startNewFrame();
	for (const auto& e : context.events) {
		Engine::Events::onEvent(e);
	}

	Editor::Instance& instance = *(Editor::Instance*)context.appmemory;
	instance = Editor::Instance{};

	Editor::API::setCurrentInstance(&instance);

	//init the engine
	Engine::EngineInitInfo engineinit{};
	engineinit.memory = (uint8*)context.appmemory + sizeof(Editor::Instance);
	engineinit.memorysize = context.appmemsize - sizeof(Editor::Instance);
	Engine::init(engineinit);

	Engine::ArenaScope s;

	//set the Imgui context in this translation unit
	ImGui::SetCurrentContext(context.imguicontext);
	ImGui::SetAllocatorFunctions(imguiAllocator, imguiDeallocator, nullptr);

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 8.0f;
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.01f, 0.01f, 0.01f, 1.0f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.01f, 0.01f, 0.01f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);

	const char* filepaths[6] = {
		"res/assets/textures/skybox/right.jpg",
		"res/assets/textures/skybox/left.jpg",
		"res/assets/textures/skybox/top.jpg",
		"res/assets/textures/skybox/bottom.jpg",
		"res/assets/textures/skybox/front.jpg",
		"res/assets/textures/skybox/back.jpg"
	};


	//init the renderer
	instance = Editor::createInstance("test.phproj");
	
	//instance.views = Engine::ArrayList<Editor::View>::create(4);
	//materialview.init(materialview.userdata);
	/*
	for (auto& view : instance.views) {
		if (view.init) {
			view.init(view.userdata);
		}
	}
	*/

	Engine::Assets::CubeMap* cubemap = (Engine::Assets::CubeMap*)Engine::Allocator::alloc(sizeof(Engine::Assets::CubeMap));
	Engine::Assets::loadCubeMapSource(filepaths, cubemap);

	instance.assets->getLoadedByReference<Script>("TextureView");

	//setup the skybox
	auto* skyboxshader = instance.assets->getLoadedByReference<Engine::Assets::MaterialInstance>("SkyBoxShader_base");

	if (cubemap && skyboxshader) {
		Scene::setSkyBox(cubemap, skyboxshader, instance.activescene->instance);
	}

	return true;
}

real64 lastframetime = 0.0f;

PH_DLL_EXPORT PH_APPLICATION_UPDATE(applicationUpdate) {
	
	//start a new frame
	Engine::beginNewFrame();

	//scope the arena allocator
	Engine::ArenaScope s;

	Editor::Instance& instance = *(Editor::Instance*)context.appmemory;

	Engine::Renderer3D::RenderStats stats = Engine::Renderer3D::getStats(instance.renderer3d);
	Engine::Renderer3D::resetStats(instance.renderer3d);

	for (const auto& e : context.events) {
		Engine::Events::onEvent(e);

		if (e.type == PH_EVENT_TYPE_KEY_PRESSED) {
			if (e.lparam == PH_KEY_S && Engine::Events::isKeyPressed(PH_LSHIFT)) {
				instance.assets->saveAllToDisk();
			}
		}

		for (auto& view : instance.views) {
			if (view.dispatchEvent && view.focussed) {
				view.dispatchEvent(view.userdata, e);
			}
		}
	}

	//calculate the time difference between this frame and last frame
	real32 deltatime = context.ellapsedtimeseconds - lastframetime;
	lastframetime = context.ellapsedtimeseconds;

	//update the editor camera to take in events;
	ImGuizmo::BeginFrame();

	Editor::BeginDockspace();
	Editor::drawMenuBar(&instance);
	
	if(ImGui::Begin("RenderStats")) {
		ImGui::Text("triangle count: %u", stats.trianglecount);
		ImGui::Text("Drawcalls: %u", stats.drawcalls);

		ImGui::Text("Frame rate: %f", 1 / deltatime);
	}
	ImGui::End();

	for (auto& view : instance.views) {
		Editor::drawView(&view);
	}

	Editor::EndDockspace();
	static real32 size = 1.0f;

	//render the composite image of composed from imgui and the scene in the display	
	Editor::beginFinalRenderpass(context.windowwidth, context.windowheight);
	ImGui::Render();
	GFX::drawImguiWidgets(ImGui::GetDrawData());
	Editor::endFinalRenderpass();
	return true;
}

PH_DLL_EXPORT PH_APPLICATION_DESTROY(applicationDestroy) {


	Editor::Instance& instance = *(Editor::Instance*)context.appmemory;
	instance.assets->saveAllToDisk();

	Editor::serialize(&instance, "test.phproj");
	return true;
}