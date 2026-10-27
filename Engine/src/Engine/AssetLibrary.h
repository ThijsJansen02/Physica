#pragma once
#include "AssetDescription.h"
#include <Base/Datastructures/HashMap.h>
#include <Engine/coreassets/Font.h>

namespace PH::Engine {

	inline uint64 stringHash(const Base::SubString& string) {
		return Base::stringHash(string.getC_Str());
	}

	inline bool32 stringCompare(const Base::SubString& left, const Base::SubString& right) {
		return Base::stringCompare(left.getC_Str(), right.getC_Str());
	}

	typedef enum AssetFlagsBits {
		PH_ASSET_SERIALIZABLE = 1
	};	
	
	typedef uint32 AssetFlags;

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
		volatile AssetStatus status;
		AssetFlags flags;

		//maybe make this an ID if we do not require the asset description to be persistent in memory but for now it is persistent
		AssetDescriptionID assetdescription;

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
		AssetDescription* getAssetDescription(AssetDescriptionID id) {
			return &assetpools.get_last(id)->description;
		}

		//retrieves an asset description based on the asked extension. extensions are the primary keys for asset descriptions
		template<typename Asset>
		AssetDescription* getAssetDescription() {
			AssetDescriptionID descriptionid = Asset::getAssetDescriptionID();
			return getAssetDescription(descriptionid);
		}

		AssetPool* getAssetPool(AssetDescriptionID id) {
			return assetpools.get_last(id);
		}

		template<typename Asset>
		AssetPool* getAssetPool() {
			AssetDescriptionID descriptionid = Asset::getAssetDescriptionID();
			return getAssetPool(descriptionid);
		}

		void unloadAsset(AssetIdentifier* id, AssetPool* pool) {
			AssetDescription* desc = &pool->description;
			if (id->assetdata && desc->destroy_) {
				desc->destroy_(id->assetdata);
			}

			if (id->assetdata) {
				Engine::Allocator::dealloc(id->assetdata);
			}

			id->status = UNLOADED;
		}

		//destroys an asset identifier and any asset that is still attached to it
		void destroyAssetIdentifier(AssetIdentifier* id, AssetPool* pool) {
			
			//if an asset loader thread is loading the asset, wait till it has finished loading
			while (id->status == LOADING) {
				Sleep(100);
			}

			for (auto& ref : id->references) {
				pool->assetreferences.remove(ref.getSubString());
				Engine::String::destroy(&ref);
			}

			Engine::String::destroy(&id->filepath);

			AssetDescription* desc = &pool->description;

			unloadAsset(id, pool);
		}

		void serializeReferences(YAML::Emitter& out, Base::Array<Engine::String> references) {

			out << YAML::BeginSeq;
			for (const auto& ref : references) {
				out << YAML::Value << ref.getC_Str();
			}
			out << YAML::EndSeq;
		}

		Engine::ArrayList<Engine::String> deserializeReferences(YAML::Node in, AssetPool* pool, AssetIdentifier* toref) {
			Engine::ArrayList<Engine::String> result = Engine::ArrayList<Engine::String>::create();
			for (YAML::Node ref : in) {
				result.pushBack(ref.as<Engine::String>());
				pool->assetreferences.add(result[result.getCount() - 1].getSubString(), toref);
			}
			return result;
		}

		void SerializeIdentifier(YAML::Emitter& out, AssetIdentifier* identifier) {
			out << YAML::BeginMap;

			out << YAML::Key << "assetid" << YAML::Value << identifier->assetid;
			out << YAML::Key << "assetdescriptionid" << YAML::Value << identifier->assetdescription;
			out << YAML::Key << "filepath" << YAML::Value << identifier->filepath.getC_Str();
			out << YAML::Key << "flags" << YAML::Value << identifier->flags;
			out << YAML::Key << "references" << YAML::Value;
			serializeReferences(out, identifier->references.getArray());

			out << YAML::EndMap;
		}

		//deserializes asset identifier and adds it to the pool
		AssetIdentifier* deserializeAssetIdentifier(YAML::Node in, AssetPool* pool) {
			
			AssetIdentifier id{};
			id.assetdata = nullptr;

			if (in["assetdescriptionid"]) {
				id.assetdescription = in["assetdescriptionid"].as<AssetDescriptionID>();
				PH_DEBUG_ASSERT(id.assetdescription == pool->description.id, "asset description id does not equal the pool description id")
			}
			else {
				id.assetdescription = pool->description.id;
			}

			id.assetid = in["assetid"].as<AssetID>();
			id.flags = in["flags"].as<AssetFlags>();
			id.filepath = in["filepath"].as<Engine::String>();

			id.status = UNLOADED;
			
			auto* result = pool->assets.add(id.assetid, id);

			result->references = deserializeReferences(in["references"], pool, result);
			
			return result;

		}

		//serialize asset in location of users choice
		void SerializeAsset(AssetIdentifier* identifier, AssetDescription* desc, const char* path) {

			//if the path is null return
			if (!path) {
				return;
			}

			if (!identifier->assetdata || !(identifier->status == LOADED)) {
				return;
			}

			YAML::Emitter out;

			out << YAML::BeginMap;

			out << YAML::Key << "assetidentifier" << YAML::Value;
			SerializeIdentifier(out, identifier);

			out << YAML::Key << "asset";
			if (desc->serialize_) {
				desc->serialize_(out, identifier->assetdata);
			}

			out << YAML::EndMap;

			Engine::FileIO::writeYamlFile(out, path);
		}

		//deserializes an already included asset
		bool32 deserializeAsset(AssetPool* pool, AssetIdentifier* identifier, YAML::Node asset) {
			
			if (!asset) {
				return false;
			}
			
			if (pool->description.deserialize_) {
				if (identifier->assetdata) {
					Engine::WARN << "deserializing an asset that was already loaded!\n";
					unloadAsset(identifier, pool);
				}

				identifier->assetdata = (AssetBase*)Engine::Allocator::alloc(pool->description.localdatasize);
				pool->description.deserialize_(asset, identifier->assetdata);
				identifier->status = LOADED;
			}
			else {
				Engine::WARN << "failed to load asset";
				return false;
			}
			return true;
		}

	public:

		//serialize
		void serializeAssetIdentifiers(YAML::Emitter& out) {
			Engine::ArrayList<AssetPool*> pools = this->assetpools.getAll<Engine::Allocator>();
			
			out << YAML::BeginSeq;
			for (AssetPool* pool: pools) {
				Engine::ArrayList<AssetIdentifier*> assets = pool->assets.getAll<Engine::Allocator>();

				out << YAML::BeginMap;
				out << YAML::Key << "descriptionid" << YAML::Value << pool->description.id;
				
				out << YAML::Key << "includedassets" << YAML::Value << YAML::BeginSeq;
				for (AssetIdentifier* identifier : assets) {
					if (identifier->flags & PH_ASSET_SERIALIZABLE) {
						SerializeIdentifier(out, identifier);
					}
				}
				out << YAML::EndSeq;
				out << YAML::EndMap;
				Engine::ArrayList<AssetIdentifier*>::destroy(&assets);
			}
			out << YAML::EndSeq;
			Engine::ArrayList<AssetPool*>::destroy(&pools);
		}

		void deserializeAssetIdentifiers(YAML::Node in) {
			if (in) {
				for (YAML::Node pool : in) {

					AssetPool* assetpool = nullptr;
					if (!pool["descriptionid"]) {
						continue;
					}

					assetpool = getAssetPool(pool["descriptionid"].as<AssetDescriptionID>());
					if (!assetpool) {
						continue;
					}

					if (!pool["includedassets"]) {
						continue;
					}

					for (YAML::Node asset : pool["includedassets"]) {
						deserializeAssetIdentifier(asset, assetpool);
					}
				}
			}
		}

		//serialzes all assets in memory that are serializable
		void serializeAssets() {

			Engine::ArrayList<AssetPool*> pools = this->assetpools.getAll<Engine::Allocator>();
			for (AssetPool* pool : pools) {
				Engine::ArrayList<AssetIdentifier*> assets = pool->assets.getAll<Engine::Allocator>();

				for (AssetIdentifier* identifier : assets) {
					if (identifier->flags & PH_ASSET_SERIALIZABLE) {
						SerializeAsset(identifier, &pool->description, identifier->filepath.getC_Str());
					}
				}
				
				Engine::ArrayList<AssetIdentifier*>::destroy(&assets);
			}
			Engine::ArrayList<AssetPool*>::destroy(&pools);
		}


		//deletes all serializable assets, the identifier as well as any in memory data
		void removeAllSerializabeAssets() {
			Engine::ArrayList<AssetPool*> pools = this->assetpools.getAll<Engine::Allocator>();

			for (AssetPool* pool : pools) {
				Engine::ArrayList<AssetIdentifier*> assets = pool->assets.getAll<Engine::Allocator>();

				
				for (AssetIdentifier* identifier : assets) {

					//if the asset is currently loading in a different thread, wait till the loading has completed
					if (identifier->flags & PH_ASSET_SERIALIZABLE) {
						destroyAssetIdentifier(identifier, pool);
					}
				}
				Engine::ArrayList<AssetIdentifier*>::destroy(&assets);
			}
			Engine::ArrayList<AssetPool*>::destroy(&pools);
		}


		//retrieves an assetidentifier of type Asset based on the given id, returns nullptr if the asset is not found
		template<typename Asset>
		AssetIdentifier* getAssetIdentifier(AssetID id) {
			auto* pool = getAssetPool<Asset>();
			return pool->assets.get_last(id);
		}

		//retrieves an asset and loads it in when needed, blocks the calling thread until the asset is loaded
		template<typename Asset>
		Asset* getAsset(AssetID id) {

			AssetIdentifier* identifier = getAssetIdentifier<Asset>(id);
			Asset* asset = (Asset*)identifier->assetdata;

			if (!asset || identifier->status == UNLOADED) {
				
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


		/// <summary>
		/// adds a new asset to the project that will be serialized and deserialize automatically to the file path
		/// </summary>
		/// <typeparam name="Asset">the asset type that is added to the project</typeparam>
		/// <param name="asset">the created asset that needs to be added to the project</param>
		/// <param name="filepath">the filepath where the asset needs to be serialized</param>
		/// <param name="name">the name that is attached to the asset, more references can be added</param>
		/// <param name="flags">the assetflags for the asset</param>
		/// <returns></returns>
		template<typename Asset>
		AssetIdentifier* addAsset(Asset* asset, const char* filepath, const char* name, AssetFlags flags) {
			AssetPool* pool = getAssetPool<Asset>();
			
			if (!pool) {
				Engine::WARN << "trying to add an asset that has no pool or description, see if the assettype is actually added to the system\n";
				return nullptr;
			}
			
			AssetIdentifier identifier{};
			identifier.assetid = Engine::createRandomUUID();
			identifier.filepath = String::create(filepath);
			identifier.flags = flags;
			identifier.status = UNLOADED;

			AssetIdentifier* result = pool->assets.add(identifier.assetid, identifier);

			//asset is loaded and not serializable
			if (asset) {
				result->status = AssetStatus::LOADED;
				result->assetdata = (AssetBase*)asset;
				result->assetdata->id = identifier.assetid;
				result->assetdata->identifier = result;
			}

			addReferenceToAsset(result->assetid, name);

			return result;
		}

		//adds an asset to the library with no disk requirement, these assets are not automatically serialized or deserialize and have to be serialized and deserialized manually
		//add a persistent id to the asset to have other assets and views find the asset consistently
		template<typename Asset>
		AssetIdentifier* addPersistentAsset(Asset* asset, AssetID id) {
			//asset should be derived from AssetBase, so we can cast it to AssetBase
			auto* pool = getAssetPool<Asset>();
			AssetIdentifier* identifier = pool->assets.add(id, createAssetIdentifier((AssetBase*)asset, id, &pool->description));

			//asset is loaded and not serializable
			identifier->flags = 0;
			identifier->status = AssetStatus::LOADED;

			AssetBase* base = (AssetBase*)asset;
			base->identifier = identifier;
			base->id = id;

			return identifier;
		}

		//adds an asset to the library with no disk requirement, these assets are not automatically serialized or deserialize and have to be serialized and deserialized manually
		//add a persistent id to the asset to have other assets and views find the asset consistently
		template<typename Asset>
		AssetIdentifier* addPersistentAsset(Asset* asset, AssetID id, const char* name) {
			AssetIdentifier* identifier = addPersistentAsset<Asset>(asset, id);
			addReferenceToAsset<Asset>(asset, name);
			return identifier;
		}

		//serialize asset in location of users choice
		template<typename Asset>
		void SerializeAsset(AssetIdentifier* identifier, const char* path) {
			SerializeAsset(identifier, getAssetDescription<Asset>(), path);
		}

		//deserialize asset in location of users choice
		template<typename Asset>
		AssetIdentifier* deserializeAsset(const char* path) {
			YAML::Node root = Engine::FileIO::loadYamlfile(path);

			AssetPool* pool = getAssetPool<Asset>();

			if (!root) {
				return nullptr;
			}


			if (!root["assetidentifier"]) {
				return nullptr;
			}
				
			AssetIdentifier* id = deserializeAssetIdentifier(root["assetidentifier"], pool);

			if (!root["asset"]) {
				return nullptr;
			}

			if (!deserializeAsset(pool, id, root["asset"])) {
				return nullptr;
			}

			return id;
		}

		template<typename Asset>
		Engine::ArrayList<AssetIdentifier*> getAllIdentifiersOfType() {
			auto* pool = getAssetPool<Asset>();
			return pool->assets.getAll<Engine::Allocator>();
		}
		

		//gets asset by reference, does not guarantee that the asset is of correct type, use with caution
		template<typename Asset>
		Asset* getAssetByReference(const char* reference) {

			auto* pool = getAssetPool<Asset>();

			AssetIdentifier* identifier = *pool->assetreferences.get_last(Base::SubString(reference));
			if(!identifier) {
				return nullptr;
			}

			return (Asset*)identifier->assetdata;
		}

		Base::ChainedHashMap<AssetDescriptionID, AssetPool, Base::uint64Hash, Base::uint64Compare, Engine::Allocator> assetpools;
	};
}