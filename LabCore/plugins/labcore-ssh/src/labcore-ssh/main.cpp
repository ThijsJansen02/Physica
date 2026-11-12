#include <libssh/libssh.h>

#include <LabCore/LabCoreAPI.h>
#include "assets/Connection.h"
#include "views/ConnectionView.h"


namespace PH::LabCoreSSH {



}

using namespace PH::LabCore;

extern "C" {
	_declspec(dllexport) void init(const char* plugindir) {

		ssh_init();

		auto* assets = PH::LabCore::getAssetLibrary();
		assets->addAssetType(PH::LabCoreSSH::createConnectionAssetdescription());


		PH::LabCore::addView(PH::LabCoreSSH::createConnectionViewDescription());
		addOnRenderCallback(PH::LabCoreSSH::connectionCreatePopupModalCallback);
		addOnRenderCallback(PH::LabCoreSSH::FileTransferPopupCallBack);
	}
}