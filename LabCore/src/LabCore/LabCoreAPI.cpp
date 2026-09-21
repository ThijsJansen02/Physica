#include "LabCore.h"
#include "LabCoreAPI.h"

namespace PH::LabCore {

	void addView(const View& view) {
		appcontext->views.pushBack(view);
	}
}