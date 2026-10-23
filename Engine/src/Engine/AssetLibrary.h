#pragma once
#include "AssetDescription.h"
#include <Base/Datastructures/HashMap.h>

namespace PH::Engine {

	inline uint64 stringHash(const Base::SubString& string) {
		return Base::stringHash(string.getC_Str());
	}

	inline bool32 stringCompare(const Base::SubString& left, const Base::SubString& right) {
		return Base::stringCompare(left.getC_Str(), right.getC_Str());
	}

	enum AssetStatus {
		LOADED,
		LOADING,
		UNLOADED
	};

	//assetidentifier
	//will exist for as long as it is included in the project even if the asset is unloaded
	//holds information about where the asset is on disk and if it is loaded into memory
	struct AssetIdentifier {
		AssetID assetid;
		AssetStatus status;

		//maybe make this an ID if we do not require the asset description to be persistent in memory but for now it is persistent
		AssetDescription* assetdescription;

		Engine::ArrayList<Engine::String> references;

		Engine::String filepath;
		AssetBase* assetdata;
	};

	//creates an asset identifier for the given asset
	PH_ENGINE_API AssetIdentifier createAssetIdentifier(AssetBase* asset, AssetID id, AssetDescription* description);

	//container with all assets of a given type
	struct AssetPool {
		AssetDescription description;

		Base::ChainedHashMap<AssetID, AssetIdentifier, Base::uint64Hash, Base::uint64Compare, Engine::Allocator> assets;
		Base::ChainedHashMap<Base::SubString, AssetIdentifier*, stringHash, stringCompare, Engine::Allocator> assetreferences;
	};

	class PH_ENGINE_API AssetLibrary {
	public:

		static AssetLibrary createAssetLibrary();

		void addAssetType(const AssetDescription& description);

	private:
		//retrieves an asset description based on the asked extension. extensions are the primary keys for asset descriptions
		template<typename Asset>
		AssetDescription* getAssetDescription() {
			AssetDescriptionID descriptionid = Asset::getAssetDescriptionID();
			return assetpools.get_last(descriptionid)->description;
		}

		template<typename Asset>
		AssetPool* getAssetPool() {
			return assetpools.get_last(Asset::getAssetDescriptionID());
		}

	public:
		//retrieves an assetidentifier of type Asset based on the given id, returns nullptr if the asset is not found
		template<typename Asset>
		AssetIdentifier* getAssetIdentifier(AssetID id) {
			AssetPool* pool = getAssetPool<Asset>();

			return pool->assets.get_last(id);
		}

		template<typename Asset>
		Asset* getAsset(AssetID id) {

			AssetIdentifier* identifier = getAssetIdentifier<Asset>(id);
			Asset* asset = (Asset*)identifier->assetdata;

			if (!asset || identifier->status == UNLOADED) {
				Engine::WARN << "trying to retrieve an asset that is not loaded!\n";
			}

			return asset;
		}

		template<typename Asset>
		bool32 addReferenceToAsset(AssetID assetid, const char* reference) {
			AssetIdentifier* identifier = getAssetIdentifier<Asset>(assetid);
			if (identifier == nullptr) {
				Engine::WARN << "trying to add a reference to an asset that does not exist\n";
				return false;
			}

			auto& strref = identifier->references.pushBack(Engine::String::create(reference));

			getAssetPool<Asset>()->assetreferences.add(strref.getSubString(), identifier);
			return true;
		}

		template<typename Asset> 
		bool32 addReferenceToAsset(Asset* asset, const char* reference) {
			AssetBase* base = (AssetBase*)asset;
			return addReferenceToAsset<Asset>(base->identifier->assetid, reference);
		}

		//adds an asset to the library with no disk requirement
		template<typename Asset>
		AssetIdentifier* addAsset(Asset* asset, AssetID id) {
			//asset should be derived from AssetBase, so we can cast it to AssetBase
			AssetPool* pool = getAssetPool<Asset>();
			AssetIdentifier* identifier = pool->assets.add(id, createAssetIdentifier((AssetBase*)asset, id, &pool->description));

			AssetBase* base = (AssetBase*)asset;
			base->identifier = identifier;
			base->id = id;

			return identifier;
		}


		template<typename Asset>
		AssetIdentifier* addAsset(Asset* asset, AssetID id, const char* name) {
			AssetIdentifier* identifier = addAsset<Asset>(asset, id);
			addReferenceToAsset<Asset>(asset, name);
			return identifier;
		}

		template<typename Asset>
		Engine::ArrayList<AssetIdentifier*> getAllIdentifiersOfType() {
			AssetPool* pool = getAssetPool<Asset>();
			return pool->assets.getAll<Engine::Allocator>();
		}
		

		//gets asset by reference, does not guarantee that the asset is of correct type, use with caution
		template<typename Asset>
		Asset* getAssetByReference(const char* reference) {

			AssetPool* pool = getAssetPool<Asset>();

			AssetIdentifier* identifier = *pool->assetreferences.get_last(Base::SubString(reference));
			if(!identifier) {
				return nullptr;
			}

			return (Asset*)identifier->assetdata;
		}

		Base::ChainedHashMap<AssetDescriptionID, AssetPool, Base::uint64Hash, Base::uint64Compare, Engine::Allocator> assetpools;
	};
}