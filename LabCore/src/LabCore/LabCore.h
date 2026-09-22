#pragma once
#include <Engine/Engine.h>
#include <Engine/Rendering.h>
#include <Engine/cppAPI/Rendering.hpp>
#include <Engine/Events.h>
#include "View.h"

namespace PH::LabCore {
	struct AppContext {
		PH::Platform::GFX::GraphicsPipeline defaultgraphicspipeline2D;
		PH::Platform::GFX::GraphicsPipeline defaultfontpipeline2D;


		PH::Platform::GFX::RenderpassDescription defaultrenderpassdescription;

		PH::Engine::Renderer2D::Wrapper renderer2D;

		//the available views in the application, these are registered by the application and can be used to create view instances
		PH::Engine::ArrayList<View> views;

		//the active instances of views in the application, these are created by the application and can be used to draw and update the views
		PH::Engine::ArrayList<ViewInstance> viewinstances;
	};

	extern AppContext* appcontext;
	extern PH::Engine::Renderer2D::Wrapper* renderer2D;

	void update();
	void draw();
	void loadCorePlugins();

	void serializeProject(const char* filepath);
	void deserializeProject(const char* filepath);

	void init();
	void destroy();
}