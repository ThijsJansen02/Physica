#pragma once

#include <LabCore/LabCoreAPI.h>

namespace PH::LabCore {

	struct SceneHierarchyView {
		Engine::AssetIdentifier* scene;
	};

	LabCore::ViewDescription createSceneHierarchyViewDescription();
}