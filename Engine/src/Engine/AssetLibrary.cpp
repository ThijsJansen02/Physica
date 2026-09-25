
#include <Engine/AssetLibrary.h>

namespace PH::Engine {

	AssetLibrary AssetLibrary::createAssetLibrary() {

		AssetLibrary lib{};
		lib.assetdescriptions = Base::ChainedHashMap<Base::SubString, AssetDescription, stringHash, stringCompare, Engine::Allocator>::create(4);
		lib.assets = Base::ChainedHashMap<UUID, AssetIdentifier, Base::uint64Hash, Base::uint64Compare, Engine::Allocator>::create(4);
		lib.assetreferences = Base::ChainedHashMap<Base::SubString, AssetIdentifier*, stringHash, stringCompare, Engine::Allocator>::create(4);

		return lib;
	}

	AssetIdentifier createAssetIdentifier(AssetBase* asset, UUID id, AssetDescription* description) {
		AssetIdentifier identifier;
		identifier.assetdata = asset;
		identifier.assetdescription = description;
		identifier.assetid = id;

		//could optimize this to a string pool or one string seperated by ; 
		identifier.references = Engine::ArrayList<Engine::String>::create();

		identifier.filepath = Engine::String::create("");
		identifier.status = LOADED;
		return identifier;
	}

	AssetIdentifier* AssetLibrary::addAsset(AssetBase* asset, UUID id, const char* extension) {
		
		AssetDescription* description = assetdescriptions.get_last(Base::SubString(extension));
		if (description == nullptr) {
			Engine::WARN << "tried to add asset that doesnt have a description yet!\n";
		}


		AssetIdentifier identifier = createAssetIdentifier(asset, id, description);

		asset->identifier = assets.addDistinct(id, identifier);
		return asset->identifier;
	}

	bool32 AssetLibrary::addReferenceToAsset(UUID assetid, const char* reference) {
		AssetIdentifier* identifier = getAssetIdentifier(assetid);
		if (identifier == nullptr) {
			Engine::WARN << "tried to add reference to asset that doesnt exist!\n";
			return false;
		}

		auto* refstring = &identifier->references.pushBack(Engine::String::create(reference));
		
		assetreferences.add(refstring->getSubString(), identifier);
		return true;
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