
#include <Engine/AssetLibrary.h>

namespace PH::Engine {

	AssetIdentifier* AssetLibrary::addAsset(AssetBase* asset, UUID id, const char* extension) {
		
		AssetDescription* description = assetdescriptions.get_last(Base::SubString(extension));
		if (description == nullptr) {
			Engine::WARN << "tried to add asset that doesnt have a description yet!\n";
		}


		AssetIdentifier identifier;
		identifier.assetdata = asset;
		identifier.assetdescription = description;
		identifier.assetid = id;
		identifier.filepath = Engine::String::create("");
		identifier.status = LOADED;

		asset->identifier = assets.addDistinct(id, identifier);
		return asset->identifier;
	}

	AssetIdentifier* AssetLibrary::getAssetIdentifier(UUID id) {
		return assets.get_last(id);
	}

	AssetDescription* AssetLibrary::addAssetDescription(const AssetDescription& description) {

		//will still work after the copy hase taken place, substring still points to correct memory location
		return assetdescriptions.addDistinct(description.extension.getSubString(), description);
	}

	AssetDescription* AssetLibrary::getAssetDescription(const char* extension) {
		return assetdescriptions.get_last(PH::Base::SubString(extension));
	}



}