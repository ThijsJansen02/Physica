
#include <Engine/AssetLibrary.h>

namespace PH::Engine {

	AssetLibrary AssetLibrary::createAssetLibrary() {

		AssetLibrary lib{};
		lib.assetpools = Base::ChainedHashMap<AssetDescriptionID, AssetPool, Base::uint64Hash, Base::uint64Compare, Engine::Allocator>::create(4);
		return lib;
	}

	void AssetLibrary::addAssetType(const AssetDescription& description) {
		AssetPool pool{};

		pool.assets = Base::ChainedHashMap<AssetID, AssetIdentifier, Base::uint64Hash, Base::uint64Compare, Engine::Allocator>::create(4);
		pool.assetreferences = Base::ChainedHashMap<Base::SubString, AssetIdentifier*, stringHash, stringCompare, Engine::Allocator>::create(4);

		pool.description = description;

		assetpools.add(description.id, pool);

		return;
	}

	AssetIdentifier createAssetIdentifier(AssetBase* asset, AssetID id, AssetDescription* description) {
		
		AssetIdentifier identifier;
		identifier.assetdata = asset;
		identifier.assetdescription = description->id;
		identifier.assetid = id;

		//could optimize this to a string pool or one string seperated by ; 
		identifier.references = Engine::ArrayList<Engine::String>::create();

		identifier.filepath = Engine::String::create("");
		identifier.status = UNLOADED;
		if (asset) {
			identifier.status = LOADED;
		}

		return identifier;
	}
}