#pragma once

#include <LabCore/LabCoreAPI.h>
#include <labcore-ssh/assets/Connection.h>

namespace PH::LabCoreSSH {

	struct FilterView {
		Connection* targetconnection;
	};

	LabCore::ViewDescription createFilterViewDescription();
	//bool32 connectionCreatePopupModalCallback(Engine::AssetLibrary* lib);
}