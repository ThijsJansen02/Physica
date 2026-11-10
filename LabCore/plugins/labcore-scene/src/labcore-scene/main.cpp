#include <LabCore/LabCoreAPI.h>

#include "views/SceneHierarchyView.h"

#include "assets/Scene.h"

#include "assets/scenecomponents/NameTag.h"
#include "assets/scenecomponents/Transform.h"


namespace PH::LabCore {
	



}


using namespace PH::LabCore;

extern "C" {
	_declspec(dllexport) void init(const char* plugindir) {

		Scene::scenecomponentdescriptions = PH::Engine::ArrayList<SceneComponentDescription>::create(0);
		
		PH::Engine::AssetLibrary* lib = PH::LabCore::getAssetLibrary();
		lib->addAssetType(createSceneAssetDescription());

		PH::LabCore::Scene::addComponentDescription(Tag::createDescription());
		PH::LabCore::Scene::addComponentDescription(Transform::createDescription());

		PH::LabCore::addView(createSceneHierarchyViewDescription());
	}
}