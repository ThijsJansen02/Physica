#include "LabCore.h"
#include "LabCoreAPI.h"

namespace PH::LabCore {

	PH_LABCORE_API void addView(const View& view) {
		appcontext->views.pushBack(view);
	}

	PH_LABCORE_API glm::vec2 getViewSize() {

		Engine::Box2D region = LabCore::openview->region;
		return { region.right - region.left, region.top - region.bottom };
	}
}