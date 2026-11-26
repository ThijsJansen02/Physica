#pragma once
#include <LabCore/LabCoreAPI.h>
#include <Engine/coreassets/Texture.h>
#include <labcore-scene/assets/scene.h>
#include "PlotCamera.h"

#ifdef LC_PLT_EXPORT
#define LC_PLT_API __declspec(dllexport)
#else
#define LC_PLT_API __declspec(dllimport)
#endif

using namespace PH;
using namespace PH::LabCore;

namespace PH::LabCorePLT {

	struct PlotView {
		struct Settings {
			const char* x_unit = "";
			const char* y_unit = "";

			bool32 logx;
			bool32 logy;
		};

		PlotCamera camera;

		Settings settings;

		bool32 dragging;
		Scene::RenderProperties renderprops;
		Scene scene;
		Engine::ImGuiDisplay display;
	};

	//returns the plotviewsettings of the plot that is currently drawing
	LC_PLT_API PlotView::Settings* getCurrentPlotViewSettings();

	LabCore::ViewDescription createPlotViewDescription();
}