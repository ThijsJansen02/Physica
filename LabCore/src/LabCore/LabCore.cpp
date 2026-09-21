#include "LabCore.h"
#include <yaml-cpp/yaml.h>


namespace PH::LabCore {

	void update() {
		
	}

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
		appcontext->views = Engine::ArrayList<View>::create(1);
		appcontext->viewinstances = Engine::ArrayList<ViewInstance>::create(1);
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
		out << YAML::Key << "openviews" << YAML::Value << YAML::BeginSeq;

		for (auto& viewinstance : appcontext->viewinstances) {
			out << YAML::Value << YAML::BeginMap;
			out << YAML::Key << "viewname" << YAML::Value << viewinstance.view->name.getC_Str();
			out << YAML::Key << "instanceid" << YAML::Value << viewinstance.instanceid;
			out << YAML::EndMap;
		}

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

			for (View& viewdescription : appcontext->views) {
				if (viewdescription.name.compare(viewname.getC_Str())) {

					ViewInstance instance = createViewInstance(&viewdescription, instanceid);
					appcontext->viewinstances.pushBack(instance);
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

	void draw() {
		drawMenuBar();

		drawViews(appcontext->viewinstances);
		updateViews(appcontext->viewinstances);
	}

	void loadShaders() {

		auto defaultquadvert = Engine::Renderer2D::checkCompileBinaries("res/shaders/default_quadshader.vert", Platform::GFX::SHADER_STAGE_VERTEX_BIT);
		auto defaultquadfrag = Engine::Renderer2D::checkCompileBinaries("res/shaders/default_quadshader.frag", Platform::GFX::SHADER_STAGE_FRAGMENT_BIT);

		appcontext->defaultgraphicspipeline2D = Engine::Renderer2D::createGraphicsPipelineFromBinaries(appcontext->defaultrenderpassdescription,
			defaultquadvert.getArray(),
			defaultquadfrag.getArray(),
			{ nullptr, 0 }
		);

		Engine::DynamicArray<uint8>::destroy(&defaultquadvert);
		Engine::DynamicArray<uint8>::destroy(&defaultquadfrag);
	}


}