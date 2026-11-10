#pragma once
#include <Engine/Engine.h>
#include "../Scene.h"

namespace PH::LabCore {

	struct Tag {
		Engine::String name;
		Engine::UUID id;


		static const char* getComponentName() {
			return "Tag";
		}

		static SceneComponentDescription createDescription();
	};
}
