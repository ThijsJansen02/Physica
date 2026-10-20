#include "LabCore.h"
#include <Engine/YamlExtensions.h>
#include <yaml-cpp/yaml.h>
#include <LabCore/EmbeddedPython.h>

#include <Engine/coreassets/Font.h>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

namespace py = pybind11;

typedef void(*plugininitfn)();

namespace PH::LabCore {

	void loadShaders();

	void init() {

		appcontext->defaultrenderpassdescription = Engine::getParentDisplay()->renderpass;
		loadShaders();

		Engine::Renderer2D::InitInfo rendererinit{};
		rendererinit.currentpipeline = appcontext->defaultgraphicspipeline2D;
		rendererinit.instancebuffersize = MEGA_BYTE * 16;
		appcontext->renderer2D = Engine::Renderer2D::Wrapper::create(rendererinit);

		renderer2D = &appcontext->renderer2D;

		//create the list of views and viewinstances
		appcontext->views = Engine::ArrayList<ViewDescription>::create(1);
		appcontext->viewinstances = Engine::ArrayList<ViewInstance>::create(1);

		appcontext->assets = Engine::AssetLibrary::createAssetLibrary();
		appcontext->assets.addAssetType(Engine::createFontDescription());

		//statically adding a font to the asset library. application should always have a font available to draw text with, this is going to be used for the python shell and for the view instances
		Engine::Font* font = Engine::Allocator::push(Engine::createFont("c:/windows/fonts/Arial.ttf", 512, 16));
		auto identifier = appcontext->assets.addAsset<Engine::Font>(font, 683249087875, "defaultfont");
		
		//should definitly be a relative path
		initPython("C:\\Users\\Thijs\\OneDrive\\Documenten\\programming\\physica\\dep\\embeddedpython");

		PH::Platform::FileBuffer file;
		if (PH::Platform::loadFile(&file, "res/labcoreinit.py")) {
			try {
				py::exec((char*)file.data);
			}
			catch (py::error_already_set& e) {
				Engine::INFO << e.what();
			}
		}
		
		HMODULE applicationlib = LoadLibraryA("LabCore-3D.dll");
		if (applicationlib == nullptr) {
			Engine::ERR << "failed to load application\n";
			PH_DEBUG_BREAK();
		}

		plugininitfn plugininit_ = (plugininitfn)GetProcAddress(applicationlib, "init");
		if (!plugininit_) { Engine::ERR << "failed to load application update\n"; }

		plugininit_();


	}

	void destroy() {

		serializeProject("project.lcproj");

		//TODO add destruction off all the resources that were created in the application
		PH::Engine::Allocator::dealloc(appcontext);
	}

	void serializeProject(const char* filepath) {
		//TODO implement serialization of the project

		YAML::Emitter out;

		out << YAML::BeginMap;

		//open views sequence
		out << YAML::Key << "openviews" << YAML::Value << YAML::BeginSeq;

		for (auto& viewinstance : appcontext->viewinstances) {
			out << YAML::Value << YAML::BeginMap;

			out << YAML::Key << "viewname" << YAML::Value << viewinstance.viewdescription->name.getC_Str();
			out << YAML::Key << "instanceid" << YAML::Value << viewinstance.instanceid;
			out << YAML::Key << "viewdata" << YAML::Value;
			if (viewinstance.viewdescription->serialize_) {
				viewinstance.viewdescription->serialize_(viewinstance.instancedata, out);
			}
			else {
				out << YAML::Null;
			}
			out << YAML::EndMap;
		}

		//end the open views sequence
		out << YAML::EndSeq;
		out << YAML::EndMap;

		Engine::FileIO::writeYamlFile(out, filepath);
	}

	void deserializeProject(const char* filepath) {
		//TODO implement deserialization of the project

		auto project = Engine::FileIO::loadYamlfile(filepath);
		if (!project) {
			Engine::WARN << "failed to load project file: " << filepath << "\n";
			return;
		}

		auto views = project["openviews"];

		if (!views) {
			Engine::WARN << "project has no views\n";
			return;
		}

		for (auto view : views) {
			Engine::String viewname = view["viewname"].as<Engine::String>();
			Engine::UUID instanceid = view["instanceid"].as<Engine::UUID>();

			for (ViewDescription& viewdescription : appcontext->views) {
				if (viewdescription.name.compare(viewname.getC_Str())) {

					
					ViewInstance instance = createViewInstance(&viewdescription, instanceid);
					if (view["viewdata"] && instance.viewdescription->deserialize_) {
						instance.viewdescription->deserialize_(instance.instancedata, view["viewdata"]);
					}
					auto& inst = appcontext->viewinstances.pushBack(instance);

				}
			}
		}
	}

	void drawMenuBar() {
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("views"))
			{
				for (auto& View : appcontext->views) {
					if (ImGui::MenuItem(View.name.getC_Str())) {
						ViewInstance instance = createViewInstance(&View);
						appcontext->viewinstances.pushBack(instance);
					}
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

	void update() {
		updateViews(appcontext->viewinstances);
	}

	void draw() {

		//rendering should be done within begin and end frame with this renderer
		Engine::Renderer2D::startFrame(appcontext->renderer2D.getContext());

		drawMenuBar();

		drawViews(appcontext->viewinstances);

		//end the frame
		Engine::Renderer2D::endFrame(appcontext->renderer2D.getContext());
	}

	void loadShaders() {

		auto defaultquadvert = Engine::Renderer2D::checkCompileBinaries("res/shaders/default_quadshader.vert", Platform::GFX::SHADER_STAGE_VERTEX_BIT);
		auto defaultquadfrag = Engine::Renderer2D::checkCompileBinaries("res/shaders/default_quadshader.frag", Platform::GFX::SHADER_STAGE_FRAGMENT_BIT);

		appcontext->defaultgraphicspipeline2D = Engine::Renderer2D::createGraphicsPipelineFromBinaries(Engine::Display::defaultrenderpassdescription,
			defaultquadvert.getArray(),
			defaultquadfrag.getArray(),
			{ nullptr, 0 }
		);

		Engine::DynamicArray<uint8>::destroy(&defaultquadvert);
		Engine::DynamicArray<uint8>::destroy(&defaultquadfrag);

		auto defaultfontvert = Engine::Renderer2D::checkCompileBinaries("res/shaders/default_fontshader.vert", Platform::GFX::SHADER_STAGE_VERTEX_BIT);
		auto defaultfontfrag = Engine::Renderer2D::checkCompileBinaries("res/shaders/default_fontshader.frag", Platform::GFX::SHADER_STAGE_FRAGMENT_BIT);

		appcontext->defaultfontpipeline2D = Engine::Renderer2D::createGraphicsPipelineFromBinaries(Engine::Display::defaultrenderpassdescription,
			defaultfontvert.getArray(),
			defaultfontfrag.getArray(),
			{ &Engine::Font::descriptorsetlayout, 1}
		);

		Engine::DynamicArray<uint8>::destroy(&defaultfontvert);
		Engine::DynamicArray<uint8>::destroy(&defaultfontfrag);
	}


}