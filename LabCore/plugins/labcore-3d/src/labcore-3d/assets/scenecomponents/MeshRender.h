#pragma once

#include "../../DefferredRenderer.h"
#include <labcore-scene/assets/scene.h>

#include <Engine/AssetLibrary.h>

namespace PH::LabCore3D {

	struct MeshRender {

		static const char* getComponentName() {
			return "Mesh";
		}

		static LabCore::SceneComponentDescription createDescription();


		//might want to add a system to detect when an asset is certainly loaded and have a direct pointer to the asset
		Engine::AssetIdentifier* mesh;
		Engine::AssetIdentifier* material;
	};



}