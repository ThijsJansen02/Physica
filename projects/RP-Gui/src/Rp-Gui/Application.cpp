
#include <Platform/PlatformAPI.h>
//#include <Platform/PlatformAPI.cpp>
#include <Base/Log.h>

namespace PH::RpGui {

	CONSOLE_WRITE(consoleWrite) {
		Platform::consoleWrite(str);
	}


	PH::Base::LogStream<consoleWrite> INFO;
	PH::Base::LogStream<consoleWrite> WARN;
	PH::Base::LogStream<consoleWrite> ERR;


}

PH_APPLICATION_INITIALIZE(applicationInitialize) {

	PH::RpGui::INFO << "intializing Rp-Gui application...\n";

	//this is just a stub application that does nothing, the real application is in the editor module
	return true;
}

PH_APPLICATION_UPDATE(applicationUpdate) {
	return true;
}

PH_APPLICATION_DESTROY(applicationDestroy) {
	return true;
}