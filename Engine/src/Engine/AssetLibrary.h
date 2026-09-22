#pragma once
#include "AssetDescription.h"
#include <Base/Datastructures/HashMap.h>

namespace PH::Engine {

	enum AssetStatus {
		LOADED,
		LOADING,
		UNLOADED
	};

	//assetidentifier
	//will exist for as long as it is included in the project even if the asset is unloaded
	//holds information about where the asset is on disk and if it is loaded into memory
	struct AssetIdentifier {
		UUID assetid;
		AssetStatus status;
		AssetDescription* assetdescription;

		Engine::String filepath;
		AssetBase* assetdata;
	};

	uint64 stringHash(const Base::SubString& string) {
		return Base::stringHash(string.getC_Str());
	}

	bool32 stringCompare(const Base::SubString& left, const Base::SubString& right) {
		return Base::stringCompare(left.getC_Str(), right.getC_Str());
	}

	class AssetLibrary {

		Base::ChainedHashMap<UUID, AssetIdentifier, Base::uint64Hash, Base::uint64Compare, Engine::Allocator> assets;
		Base::ChainedHashMap<Base::SubString, AssetDescription, stringHash, stringCompare, Engine::Allocator> assetdescriptions;

		//adds an asset to the library with no disk requirement, extension is required to find the correct description
		AssetIdentifier* addAsset(AssetBase* asset, UUID id, const char* extension);

		//retrieves an asset 
		AssetIdentifier* getAssetIdentifier(UUID id);

		template<typename T>
		T* getAsset(UUID id) {

			AssetIdentifier* identifier = getAssetIdentifier(id);
			T* asset = (T*)identifier->assetdata;

			if (!asset || identifier->status == UNLOADED) {
				Engine::WARN << "trying to retrieve an asset that is not loaded!";
			}

			return asset;
		}

		//adds a new type of asset to the library
		AssetDescription* addAssetDescription(const AssetDescription& description);

		//retrieves an asset description based on the asked extension. extensions are the primary keys for asset descriptions
		AssetDescription* getAssetDescription(const char* extension);
		

	};
}