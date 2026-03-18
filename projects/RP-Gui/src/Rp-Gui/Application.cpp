
#include <Platform/PlatformAPI.h>
#include <Platform/PlatformAPI.cpp>

namespace PH::RpGui {

	PH_APPLICATION_INITIALIZE(applicationInitialize) {
		//this is just a stub application that does nothing, the real application is in the editor module
		return true;
	}

	PH_APPLICATION_UPDATE(applicationUpdate) {
		return true;
	}

	PH_APPLICATION_DESTROY(applicationDestroy) {
		return true;
	}

}