#include "LabCore.h"
#include <Engine/YamlExtensions.h>
#include <yaml-cpp/yaml.h>
#include <LabCore/EmbeddedPython.h>

#include <Engine/coreassets/Font.h>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

namespace py = pybind11;

typedef void(*plugininitfn)(const char* plugindir);

#define RENDERER_2D_INSTANCE_BUFFER_SIZE MEGA_BYTE * 16

namespace PH::LabCore {

	void loadShaders();

	void loadIni() {
		appcontext->pythonhome = Engine::String::create("C:\\Users\\Thijs\\OneDrive\\Documenten\\programming\\physica\\dep\\embeddedpython");
		appcontext->projectpath = Engine::String::create("example_project.lcproj");
		appcontext->projectdir = Engine::String::create("");

		YAML::Node ini = Engine::FileIO::loadYamlfile("labcore.ini");
		if (ini) {
			if (ini["projectpath"]) {
				appcontext->projectpath.set(ini["projectpath"].as<Engine::String>());
				appcontext->projectdir.set(appcontext->projectpath.getSubString().getTillLast('/'));
			}

			if (ini["pythonhome"]) {
				appcontext->pythonhome.set(ini["pythonhome"].as<Engine::String>());
			}
		}
	}

	void writeIni() {

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "pythonhome" << YAML::Value << appcontext->pythonhome.getC_Str();
		out << YAML::Key << "projectpath" << YAML::Value << appcontext->projectpath.getC_Str();
		out << YAML::EndMap;
		Engine::FileIO::writeYamlFile(out, "labcore.ini");
	}

	void initRenderer() {
		Engine::Renderer2D::InitInfo rendererinit{};
		rendererinit.currentpipeline = appcontext->defaultgraphicspipeline2D;
		rendererinit.instancebuffersize = RENDERER_2D_INSTANCE_BUFFER_SIZE;
		appcontext->renderer2D = Engine::Renderer2D::Wrapper::create(rendererinit);

		renderer2D = &appcontext->renderer2D;
	}

	void initViews() {
		//create the list of views and viewinstances
		appcontext->views = Engine::ArrayList<ViewDescription>::create(1);
		appcontext->viewinstances = Engine::ArrayList<ViewInstance>::create(1);
	}

	void initAssetLibrary() {
		//create asset library and add font asset
		appcontext->assets = Engine::AssetLibrary::createAssetLibrary();
		appcontext->assets.addAssetType(Engine::createFontDescription());

		//statically adding a font to the asset library. application should always have a font available to draw text with, this is going to be used for the python shell and for the view instances
		Engine::Font* font = Engine::Allocator::push(Engine::createFont("c:/windows/fonts/Arial.ttf", 512, 16));
		auto identifier = appcontext->assets.addPersistentAsset<Engine::Font>(font, 683249087875, "defaultfont");
	}

	void initPyLabCore() {
		PH::Platform::FileBuffer file;
		if (PH::Platform::loadFile(&file, "res/labcoreinit.py")) {
			try {
				py::exec((char*)file.data);
			}
			catch (py::error_already_set& e) {
				Engine::INFO << e.what();
			}
		}
	}

	//inits the plugins that are part of the project, in the future the project file should determine the installed plugins
	void initPlugins() {

		plugininitfn plugininit_ = nullptr;

		//load labcore-3D plugin
		HMODULE applicationlib = LoadLibraryA("LabCore-3D.dll");
		if (applicationlib == nullptr) {
			Engine::ERR << "failed to load application\n";
			PH_DEBUG_BREAK();
		}
		else {
			plugininit_ = (plugininitfn)GetProcAddress(applicationlib, "init");
			if (!plugininit_) {
				Engine::ERR << "failed to load application update\n";
			}
			else {
				Engine::String pluginhome = Engine::String::create(appcontext->labcorehome.getC_Str());
				pluginhome.append("/plugins/labcore-3d/");
				plugininit_(pluginhome.getC_Str());
				Engine::String::destroy(&pluginhome);
			}
		}

		//set the working directoy back to labcore home
		Platform::setCurrentDirectory(appcontext->labcorehome.getSubString());
	}

	void init() {

		char buffer[256];
		PH::Platform::getCurrentDirectory(buffer, 256);
		appcontext->labcorehome = Engine::String::create(buffer);
		appcontext->labcorehome.replace('\\', '/');


		loadShaders();
		loadIni();
		initRenderer();
		initViews();
		initAssetLibrary();

		//should definitly be a relative path
		initPython(appcontext->pythonhome.getC_Str());

		initPyLabCore();
		initPlugins();

	}

	

	void destroy() {

		writeIni();
		serializeProject(appcontext->projectpath.getC_Str());

		//TODO add destruction off all the resources that were created in the application
		PH::Engine::Allocator::dealloc(appcontext);
	}

	void serializeProject(const char* filepath) {
		//TODO implement serialization of the project


		//serialize project happens in the home working directory
		PH::Platform::setCurrentDirectory(appcontext->labcorehome.getSubString());


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


		out << YAML::Key << "includedassets" << YAML::Value;
		appcontext->assets.serializeAssetIdentifiers(out);

		out << YAML::EndMap;
		Engine::FileIO::writeYamlFile(out, filepath);

		//now serialize all serializable assets
		PH::Platform::setCurrentDirectory(appcontext->projectdir.getSubString());
		appcontext->assets.serializeAssets();

		ImGui::SaveIniSettingsToDisk("imgui.ini");
	}

	void deserializeProject(const char* filepath) {
		//TODO implement deserialization of the project

		//deserialize project must 

		Engine::INFO << "opening project: " << filepath << "\n";

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

		appcontext->projectpath.set(filepath);
		appcontext->projectpath.replace('\\', '/');
		appcontext->projectdir.set(appcontext->projectpath.getSubString().getTillLast('/'));

		PH::Platform::setCurrentDirectory(appcontext->projectdir.getSubString());

		auto includedassets = project["includedassets"];

		if (includedassets) {
			appcontext->assets.deserializeAssetIdentifiers(includedassets);
		}

		ImGui::LoadIniSettingsFromDisk("imgui.ini");
	}

	void drawMenuBar() {
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Open Project")) {

				}

				if (ImGui::MenuItem("New Project")) {

					char buffer[256];

					PH::Platform::OpenFileDialogInfo info;
					info.filter = LabCore::projectextension;
					info.read = true;
					info.resultbuffer = buffer;
					info.resultbuffersize = 256;
					info.flags = PH_DIALOG_PATHMUSTEXIST;

					if (PH::Platform::openFileDialog(info)) {

						serializeProject(appcontext->projectpath.getC_Str());

						destroyAllViewInstances(&appcontext->viewinstances);

						//remove all assets from the previous project
						appcontext->assets.removeAllSerializabeAssets();

						Engine::String newprojectpath = Engine::String::create(buffer);
						newprojectpath.replace('\\', '/');

						Base::setExtension(&newprojectpath, LabCore::projectextension);
						
						appcontext->projectpath.set(newprojectpath);
						appcontext->projectdir.set(newprojectpath.getSubString().getTillLast('/'));

					}
				}

				ImGui::EndMenu();
			}

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