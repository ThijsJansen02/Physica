#pragma once
#include <Engine/Engine.h>
#include <Engine/Scene.h>
#include <Engine/Display.h>
#include <Engine/AssetLibrary.h>
#include "Editor/View.h"

using namespace PH::Engine::Assets;

namespace PH::Editor {

#ifdef PH_EDITOR_EXPORT
#define PH_EDITOR_API __declspec(dllexport)
#else 
#define PH_EDITOR_API __declspec(dllimport)
#endif


	struct Instance {
		Engine::Assets::Scene* activescene;
		Engine::Renderer2D::Context* renderer2d;
		Engine::Renderer3D::Context* renderer3d;

		Engine::Assets::Scene::Entity selected;
		Engine::Assets::Library* assets;	

		Engine::ArrayList<View> views;
	};
}