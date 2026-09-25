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

		Engine::ArrayList<Engine::String> references;

		Engine::String filepath;
		AssetBase* assetdata;
	};

	inline uint64 stringHash(const Base::SubString& string) {
		return Base::stringHash(string.getC_Str());
	}

	inline bool32 stringCompare(const Base::SubString& left, const Base::SubString& right) {
		return Base::stringCompare(left.getC_Str(), right.getC_Str());
	}

	class AssetLibrary {
	public:
		Base::ChainedHashMap<UUID, AssetIdentifier, Base::uint64Hash, Base::uint64Compare, Engine::Allocator> assets;
		Base::ChainedHashMap<Base::SubString, AssetIdentifier*, stringHash, stringCompare, Engine::Allocator> assetreferences;

		Base::ChainedHashMap<Base::SubString, AssetDescription, stringHash, stringCompare, Engine::Allocator> assetdescriptions;

		static AssetLibrary createAssetLibrary();

		//adds an asset to the library with no disk requirement, extension is required to find the correct description
		AssetIdentifier* addAsset(AssetBase* asset, UUID id, const char* extension);

		bool32 addReferenceToAsset(UUID assetid, const char* reference);

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
		

		//gets asset by reference, does not guarantee that the asset is of correct type, use with caution
		template<typename T>
		T* getAssetByReference(const char* reference) {
			AssetIdentifier** identifier = assetreferences.get_last(Base::SubString(reference));
			if (!identifier) {
				Engine::WARN << "trying to retrieve an asset that is not loaded!";
				return nullptr;
			}

			//should add a check to see if the asset is the correct type
			T* asset = (T*)(*identifier)->assetdata;
			if (!asset || (*identifier)->status == UNLOADED) {
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