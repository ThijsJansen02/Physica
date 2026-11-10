#pragma once

#include <LabCore/LabCoreAPI.h>
#include "../assets/Connection.h"


namespace PH::LabCoreSSH {

	struct ConnectionView {
		 Connection* current;
		 Engine::String currentcommand;
	};

	LabCore::ViewDescription createConnectionViewDescription();
	bool32 connectionCreatePopupModalCallback(Engine::AssetLibrary* lib);
}