#pragma once

#include <LabCore/LabCoreAPI.h>
#include "../assets/Connection.h"


namespace PH::LabCoreSSH {

	struct ConnectionView {
		 Connection* current;
		 Engine::String currentcommand;
	};

	//opens a popup window to allow the user to create a new connection
	LC_SSH_API bool32 openCreateConnectionPopup();

	LabCore::ViewDescription createConnectionViewDescription();
	bool32 connectionCreatePopupModalCallback(Engine::AssetLibrary* lib);
}