	#define PH_SCENE_IMPLEMENTATION

#include "Editor.h"
#include "Engine/Components.h"
#include <Engine/YamlExtensions.h>
#include <Platform/platformAPI.h>
#include <Engine/Scene.h>

#include "ImGuiPanels/InspectorPanel.h"
#include "ImGuiPanels/MaterialViewPanel.h"
#include "ImGuiPanels/SceneViewPanel.h"
#include "ImGuiPanels/ComponentViewPanel.h"

namespace PH::Editor {

	using namespace PH::Platform;

	Editor::Instance instance;

	bool32 initRendererContext(Editor::Instance* instance) {

		//init the 2d renderer
		Engine::Renderer2D::InitInfo renderer2dInit{};

		//might want to make the dsl a global object from engine
		PH::Platform::GFX::DescriptorSetLayout dsls[] = { Scene::getSceneDescriptorSetLayout(nullptr) };
		renderer2dInit.instancebuffersize = KILO_BYTE * 64;
		renderer2dInit.defaultfragpath = "res/shaders/default_quadfrag.spv";
		renderer2dInit.defaultvertpath = "res/shaders/default_quadvert.spv";
		renderer2dInit.descriptorsetlayouts = { dsls, 1 };
		renderer2dInit.renderpass = Engine::Display::renderpass;

		instance->renderer2d = Engine::Renderer2D::createContext(renderer2dInit);


		Engine::Renderer3D::InitInfo renderer3dinit{};
		renderer3dinit.colorshader = instance->assets->getByReference<Engine::Assets::MaterialInstance>("color_shader_3d");
		if (renderer3dinit.colorshader) {
			instance->assets->lazyLoad<Engine::Assets::MaterialInstance>(renderer3dinit.colorshader->id);
		}

		renderer3dinit.flatcolorshader = instance->assets->getByReference<Engine::Assets::MaterialInstance>("flat_color_shader_3d");
		if (renderer3dinit.flatcolorshader) {
			instance->assets->lazyLoad<Engine::Assets::MaterialInstance>(renderer3dinit.flatcolorshader->id);
		}

		instance->renderer3d = Engine::Renderer3D::createContext(renderer3dinit);

		return true;
	}

#define FRAMEBUFFER_WIDTH 1920
#define FRAMEBUFFER_HEIGHT 1080

	bool32 deserialize(Instance* instance, const char* filepath);

	Instance createInstance(const char* projectpath) {

		Instance result{};

		result.assets = Engine::Assets::Library::create();

		//deserialize the editor
		Editor::deserialize(&result, projectpath);

		//initialize the renderer
		Editor::initRendererContext(&result);
		return result;
	}

	void beginFinalRenderpass(uint32 windowwidth, uint32 windowheight) {

		GFX::RenderpassbeginInfo renderpassbegin{};
		renderpassbegin.description = 0;
		renderpassbegin.framebuffer = 0;
		renderpassbegin.renderarea = { windowwidth, windowheight };

		GFX::ClearValue clearcolor = { 0.1f, 0.1f, 0.1f, 1.0f };
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

	void serialize(Editor::Instance* instance, const char* filepath) {

		YAML::Emitter out;
		out << YAML::BeginMap; //editor context
		out << YAML::Key << "Views" << YAML::Value << YAML::BeginSeq; //open views


		for (auto& view : instance->views) {
			out << YAML::BeginMap; //view
			out << YAML::Key << "Title" << YAML::Value << view.name.getC_Str();
			out << YAML::Key << "InstanceID" << YAML::Value << view.id;
			out << YAML::Key << "SourceID" << YAML::Value << view.sourceid;
			out << YAML::Key << "ViewData" << YAML::Value; //the data of the view

			if (view.serialize) {
				view.serialize(view.userdata, out);
			}
			else {
				out << false;
			}

			out << YAML::EndMap;
		}

		out << YAML::EndSeq; //open views

		out << YAML::Key << "Assets" << YAML::Value;
		instance->assets->serialize(out);

		if (instance->activescene) {
			out << YAML::Key << "ActiveScene" << YAML::Value << instance->activescene->id;
		}

		out << YAML::EndMap; //editor context

		Platform::FileBuffer file{};
		file.data = (void*)out.c_str();
		file.size = out.size();
		PH::Platform::writeFile(file, filepath);
	}

	bool32 deserialize(Instance* instance, const char* filepath) {

		Platform::FileBuffer file;
		if (!PH::Platform::loadFile(&file, filepath)) {
			Scene scene = Engine::Assets::createScene();
			instance->activescene = instance->assets->add<Engine::Assets::Scene>("newScene", "res/assets/scenes", scene);
			return false;
		}

		YAML::Node root = YAML::Load((char*)file.data);

		instance->views = Engine::ArrayList<Editor::View>::create(1);

		auto assets = root["Assets"];
		if (assets) {
			instance->assets->deserialize("res");
		}

		auto views = root["Views"];
		if (!views) {
			return false;
		}

		for (auto view : views) {
			View newview{};

			auto sourceid = view["SourceID"];
			if (sourceid) {
				newview.sourceid = sourceid.as<Engine::UUID>();

				if (newview.sourceid == SCENEVIEW_ID) {
					newview = loadSceneView();
				}

				if (newview.sourceid == COMPONENT_VIEW_ID) {
					newview = loadComponentView();
				}

				if (newview.sourceid == INSPECTORVIEW_ID) {
					newview = LoadInspectorView();
				}

				if (newview.sourceid == MATERIALVIEW_ID) {
					newview = loadMaterialView();
				}
			}

			auto instanceid = view["InstanceID"];
			if (instanceid) {
				newview.id = instanceid.as<Engine::UUID>();
			}

			auto title = view["Title"];
			if (title) {
				newview.name = title.as<Engine::String>();
			}

			auto viewdata = view["ViewData"];
			if (viewdata && newview.deserialize) {
				newview.deserialize(newview.userdata, viewdata);
			}

			instance->views.pushBack(newview);
		}

		auto activescene = root["ActiveScene"];
		if (activescene) {
			instance->activescene = instance->assets->getLoaded<Engine::Assets::Scene>(activescene.as<Engine::UUID>());
		}


		if (!instance->activescene) {
			Scene scene = Engine::Assets::createScene();
			instance->activescene = instance->assets->add<Engine::Assets::Scene>("newScene", "res/assets/scenes", scene);
		}

		PH::Platform::unloadFile(&file);
		return true;
	}
}