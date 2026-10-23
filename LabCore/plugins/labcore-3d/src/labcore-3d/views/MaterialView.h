#pragma once
#include <LabCore/LabCoreAPI.h>
#include "../assets/GraphicsPipeline.h"

namespace PH::LabCore3D {

	struct MaterialView {

		Engine::AssetIdentifier* currentpipe;
		

	};

	LabCore::ViewDescription createMaterialViewDescription();
}