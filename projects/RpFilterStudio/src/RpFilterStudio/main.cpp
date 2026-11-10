#include <LabCore/LabCoreAPI.h>

using namespace PH::LabCore;

extern "C" {
	_declspec(dllexport) void init(const char* plugindir) {
		PH::Engine::INFO << "load rp filter studio\n";
	}
}