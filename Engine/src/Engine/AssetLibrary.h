
#pragma once
#include <typeinfo>
#include "Engine.h"
#include <Base/Datastructures/HashMap.h>
#include <Base/random.h>
#include "AssetStorage.h"
#include "Asset.h"

namespace YAML {
	class Emitter;
	class Node;
}

namespace PH::Engine::Assets {

	template<typename work>
	struct CircularWorkQueue;

	inline uint64 uuidhash(const Engine::UUID& uuid) {
		return uuid;
	}

	inline uint64 stringHash(const char* const& key) {
		return Base::pcg_hash((uint32)Base::stringLength(key));
	}
	
	inline bool32 stringCompare_(const char* const& key1, const char* const& key2) {
		return Base::stringCompare(key1, key2);
	}

	inline bool32 UUIDcompare(const UUID& k1, const UUID& k2) {
		return k1 == k2;
	}

	inline bool32 storage_UUID_compare(UUID k1, UUID k2) {
		return k1 == k2;
	}

	inline uint64 storage_UUID_hash(UUID id) {
		return id;
	}
	
	inline void saveAssetToDisk(const Asset* asset, YAML::Emitter& out) {

		out << YAML::Key << "UUID" << YAML::Value << asset->id;
		out << YAML::Key << "references" << YAML::Value << YAML::BeginSeq; // references
		for (auto ref : asset->references) {
			out << YAML::Value << ref.getC_Str();
		}
		out << YAML::EndSeq; //references;
	}

	using assetreferences = Base::ChainedHashMap<const char*, Asset*, stringHash, stringCompare_, Engine::Allocator>;

	struct ThreadInfo {
		bool32 open;
		Library* lib;
		DWORD threadid;
		HANDLE semaphore;
	};

	struct LoaderThread {
		ThreadInfo info;
		PH::Platform::Thread thread;
	};

	struct LoaderThreadWork {
		Asset* assettobeloaded;
	};

	/// <summary>
	/// work queue capable of multithreaded queue operations
	/// </summary>
	/// <typeparam name="Work"></typeparam>
	template<typename Work>
	struct CircularWorkQueue {

		volatile DWORD writeptr = 0;
		volatile DWORD readptr = 0;

		void push(const Work& work) {

			storage[writeptr] = work;

			_ReadWriteBarrier(); //make sure the compiler doesnt increase the writeptr before the work is submitted to the queue

			writeptr++;
			writeptr %= storage.getCapacity();
		}

		bool32 hasWork() {
			return writeptr != readptr;
		}

		bool32 isFull() {
			return (writeptr + 1) % storage.getCapacity() == readptr;
		}

		Work* pop() {
			while (true) {
				if (!hasWork()) {
					return nullptr;
				}

				DWORD index = readptr;
				DWORD newreadptr = (readptr + 1) % storage.getCapacity();
				Work* w = &storage[index % storage.getCapacity()];

				if (InterlockedCompareExchange(&readptr, newreadptr, index) == index) {
					return w;
				}
			}
		}
		DynamicArray<Work> storage;
	};

	class Library {
	public:
		using AssetStorage = AssetStorage<storage_UUID_hash, storage_UUID_compare, Engine::Allocator>;
		static Library* create();

		static UUID createRandomUUID() {
			return Base::pcg_hash((uint32)(PH::Platform::getTimeMs() * 1000.0f));
		}

		/// <summary>
		/// adds an asset that you already created to the library. making it visible to the get function...
		/// </summary>
		/// <typeparam name="type"></typeparam>
		/// <param name="asset">the asset that is to be added to the library</param>
		/// <returns></returns>
		template<typename type>
		type* add(type asset) {
			static_assert(std::is_base_of<Asset, type>::value, "type not an asset!");

			UUID id = createRandomUUID();
			asset.id = id;

			return assets.add<type>(id, asset);
		}

		/// <summary>
		/// adds an asset to the library that already has an id
		/// </summary>
		/// <typeparam name="type">the type of asset that is to be added</typeparam>
		/// <param name="asset">the actual asset instance that is added</param>
		/// <param name="id">the that the asset is to be found at</param>
		/// <returns></returns>
		template<typename type>
		type* add(const type& asset, UUID id) {
			static_assert(std::is_base_of<Asset, type>::value, "type not an asset!");

			return assets.add<type>(id, asset);
		}

		/// <summary>
		/// returns an asset at the id if it is present in the library... if it is not present it returns a nullptr
		/// </summary>
		/// <typeparam name="type">the type of asset that is to be found</typeparam>
		/// <param name="uuid">the id of the asset</param>
		/// <returns>the asset or nullptr if it is not found</returns>
		template<typename type>
		type* get(UUID uuid) {
			static_assert(std::is_base_of<Asset, type>::value, "type not an asset!");

			return assets.get<type>(uuid);
		}

		/// <summary>
		/// checks weather the asset is loaded, if it is not loaded it loads the asset immediatly in place..
		/// </summary>
		/// <typeparam name="type"></typeparam>
		/// <param name="id"></param>
		/// <returns></returns>
		template<typename type>
		type* getLoaded(Engine::UUID id) {
			type* asset = get<type>(id);

			if (!asset) {
				return nullptr;
			}

			if (asset->status == AssetStatus::NOT_LOADED) {
				load<type>(id);
			}


			//this should be changed to a more elegant synchronization primitive
			if (asset->status == AssetStatus::LOADING) {

				//wait for max 10 seconds
				for (uint32 i = 0; i < 100; i++) {
					if (asset->status != AssetStatus::LOADING) {
						break;
					}
					Sleep(100);
				}
				if (asset->status != AssetStatus::LOADED) {
					return nullptr;
				}
			}

			return asset;
		}

		/// <summary>
		/// returns the asset and if it needs to be loaded it lazy loads it...
		/// </summary>
		/// <typeparam name="type"></typeparam>
		/// <param name="id"></param>
		/// <returns></returns>
		template<typename type>
		type* getLazyLoaded(Engine::UUID id) {
			type* asset = get<type>(id);

			if (!asset) {
				return nullptr;
			}

			if (asset->status == AssetStatus::NOT_LOADED) {
				lazyLoad<type>(id);
			}

			return asset;
		}

		/// <summary>
		/// get an asset by its reference. every asset cant supply string refererces to the library with which they can be found.
		/// </summary>
		/// <typeparam name="type">the type of asset that is to be found</typeparam>
		/// <param name="name">the reference at which the item is to be found</param>
		/// <returns>a pointer to that asset or a nullptr of the asset is not found</returns>
		template<typename type>
		type* getByReference(const char* name) {
			static_assert(std::is_base_of<Asset, type>::value, "type not an asset!");

			Asset** asset = references.get_last(name);
			if (!asset) {
				return nullptr;
			}

			if ((*asset)->type == type::getType()) {
				return (type*)*asset;
			}
			return nullptr;
		}

		template<typename type>
		type* getLoadedByReference(const char* name) {
			type* asset = getByReference<type>(name);
			if (asset) {
				return getLoaded<type>(asset->id);
			}
			return nullptr;
		}
		
		/// <summary>
		/// add a string reference to an asset. when a reference is supplied to an asset the asset can be found at its address
		/// </summary>
		/// <typeparam name="type"></typeparam>
		/// <param name="id">the id of the source asset</param>
		/// <param name="name">the reference that is to be added</param>
		/// <returns>wheather the asset at id was found and if the reference was succesfully added</returns>
		template<typename type>
		bool32 addReference(UUID id, const char* name) {
			static_assert(std::is_base_of<Asset, type>::value, "type not an asset!");

			Asset* asset = (Asset*)assets.get<type>(id);
			if (!asset) {
				Engine::WARN << "failed to add reference: " << name << " to UUID: " << id << "\n";
				return false;
			}

			if (asset->references.getCapacity() == 0) {
				asset->references = ArrayList<String>::create(1);
			}

			const String& ref = asset->references.pushBack(String::create(name));
			references.add(ref.getC_Str(), asset);
			return true;
		}

		/// <summary>
		/// returns all the assets of a given type
		/// </summary>
		/// <typeparam name="type">the type of asset that is returned</typeparam>
		/// <typeparam name="allocator">the allocator with which the resulting arraylist is created</typeparam>
		/// <returns>a list with all the assets of a given type</returns>
		template<typename type, typename allocator = Engine::Allocator>
		Base::ArrayList<type*, allocator> getAll() {
			static_assert(std::is_base_of<Asset, type>::value, "type not an asset!");
			return assets.getAllOfType<type, allocator>();
		}

		/// <summary>
		/// allows you to add an asset with a given name and directory. 
		/// The name is added as a reference to the asset and the final path is the directory with the name plus extension
		/// </summary>
		/// <typeparam name="type"></typeparam>
		/// <param name="name">The name reference that is added to the asset after being added the library</param>
		/// <param name="directory">the file directory that the asset is added to</param>
		/// <param name="asset">the asset that needs te be added to the library</param>
		/// <returns>a pointer to the location that the asset is placed in</returns>
		template<typename type>
		type* add(const char* name, const char* directory, const type& asset) {

			static_assert(std::is_base_of<Asset, type>::value, "type not an asset!");

			type result = asset;
			result.type = type::getType();
			result.filepath = String::create(directory).append("/").append(name).append(type::getExtension());
			result.id = createRandomUUID();
			result.references = Engine::ArrayList<String>::create(1);
			result.status = AssetStatus::LOADED;

			type* resultptr = assets.add<type>(result.id, result);
			addReference<type>(result.id, name);
			return resultptr;
		}

		/// <summary>
		/// saves the asset to disk. is implemented per asset but can only be called when a valid filepath is set for the asset
		/// </summary>
		/// <typeparam name="type"></typeparam>
		/// <param name="asset">the asset that needs to be saved to disk</param>
		template<typename type>
		void saveToDisk(const type* asset) {
			
			//if an asset is not loaded, it should not be saved to disk because it is not complete!
			if (!(asset->status == AssetStatus::LOADED)) {
				return;
			}

			YAML::Emitter out;
			out << YAML::BeginMap;
			saveAssetToDisk(asset, out);
			out << YAML::Key << "Asset" << YAML::Value;

			type::serialize(out, (type*)asset);
			out << YAML::EndMap;

			Platform::FileBuffer file;
			file.data = (void*)out.c_str();
			file.size = out.size();
			
			Platform::writeFile(file, asset->filepath.getC_Str());
		}

		/// <summary>
		/// includes an asset into the project without loading it. as subsqequent call to load(UUID id) or lazyLoad(UUID) loads the asset into memory..
		/// </summary>
		/// <typeparam name="type"></typeparam>
		/// <param name="path">the path to the asset on disk</param>
		/// <returns></returns>
		template<typename type>
		type* include(const char* path) {
			auto root = FileIO::loadYamlfile(path);
			
			auto assetnode = root["Asset"];
			if (!assetnode) {
				INFO << "Failed to include asset\n";
				return nullptr;
			}


			type asset{};
			loadAssetInfo(root, &asset, path);

			if (!type::include(assetnode, this, &asset)) {
				WARN << "asset " << path << " was not included correctly!\n";
				return nullptr;
			}

			asset.status = AssetStatus::NOT_LOADED;
			asset.type = type::getType();

			if (assets.get<type>(asset.id)) {
				WARN << "trying to include an asset that already exists in the library\n";
				return nullptr;
			}

			type* ptr = add(asset, asset.id);
			for (String& str : asset.references) {
				references.add(str.getC_Str(), ptr);
			}
			return ptr;
		}

		/// <summary>
		/// loads an included asset that is not yet loaded
		/// </summary>
		/// <typeparam name="type"></typeparam>
		/// <param name="id"></param>
		/// <returns></returns>
		template<typename type>
		type* load(UUID id) {
			static_assert(std::is_base_of<Asset, type>::value, "type not an asset!");
			type* asset = get<type>(id);
			asset->status = AssetStatus::LOADING;

			if (!asset) {
				return nullptr;
			}

			auto root = FileIO::loadYamlfile(asset->filepath.getC_Str());
			auto assetnode = root["Asset"];
			if (!assetnode) {
				return nullptr;
			}

			INFO << "loading asset: " << asset->filepath.getC_Str() << "\n";

			if (!type::deserialize(assetnode, this, asset, asset->filepath.getC_Str())) {
				return nullptr;
			}

			asset->type = type::getType();
			asset->status = AssetStatus::LOADED;
			return asset;
		}

		/// <summary>
		/// loads an asset asynchronously
		/// </summary>
		/// <typeparam name="type"></typeparam>
		/// <param name="id"></param>
		template<typename type>
		void lazyLoad(UUID id) {
			static_assert(std::is_base_of<Asset, type>::value, "type not an asset!");

			type* asset = get<type>(id);
			asset->status = AssetStatus::LOADING;

			LoaderThreadWork work;
			work.assettobeloaded = (Asset*)asset;

			loadqueue.push(work);

			ReleaseSemaphore(workavailablesemaphore, 1, nullptr);
		}

		/// <summary>
		/// loads a certain asset from disk directly into the library
		/// </summary>
		/// <typeparam name="type"></typeparam>
		/// <param name="path">the path where the asset resides</param>
		/// <returns>returns a pointer to where the asset is stored in memory</returns>
		template<typename type>
		type* load(const char* path) {
			static_assert(std::is_base_of<Asset, type>::value, "type not an asset!");

			auto root = FileIO::loadYamlfile(path);

			auto assetnode = root["Asset"];
			if (!assetnode) {
				return nullptr;
			}

			type asset{};
			loadAssetInfo(root, &asset, path);

			INFO << "loading asset: " << path << "\n";

			if (!type::include(assetnode, this, &asset)) {
				WARN << "asset " << path << " was not included correctly!\n";
				return nullptr;
			}

			if (!type::deserialize(assetnode, this, &asset, path)) {
				WARN << "asset " << path << " was not deserialized correctly!\n";
				return nullptr;
			}

			asset.status = AssetStatus::LOADED;
			asset.type = type::getType();

			type* ptr = add(asset, asset.id);
			for (String& str : asset.references) {
				references.add(str.getC_Str(), ptr);
			}
			return ptr;
		}

		
		/// <summary>
		/// imports an asset from a sourcefile like a texture image.
		/// </summary>
		/// <typeparam name="type"></typeparam>
		/// <param name="sourcefile"></param>
		/// <param name="name"></param>
		/// <returns></returns>
		template<typename type>
		type* import(const char* sourcefile, const char* targetdir, const char* name) {
			
			type newasset{};
			type::loadFromSource(sourcefile, &newasset);
			newasset.status = AssetStatus::LOADED;
			newasset.type = type::getType();
			return add(name, targetdir, newasset);
		}

		/// <summary>
		/// imports an asset from a source file and names it to the same name as the source file
		/// </summary>
		/// <typeparam name="type"></typeparam>
		/// <param name="sourcefile">the source file with which the asset is to be created, like an fbx file directory</param>
		/// <returns>the imorted asset</returns>
		template<typename type>
		type* import(const char* sourcefile) {

			String sourcedir = String::create(sourcefile);
			sourcedir.replace('\\', '/');
			
			if (sourcedir.getSize() <= 1) {
				return nullptr;
			}

			String name = String::create(sourcedir.getFromLast('/').getSubString().moveHead(1).getTillLast('.') - 1);
			String targetdir = String::create(sourcedir.getSubString().getTillLast('/'));

			type* ptr = import<type>(sourcefile, targetdir.getC_Str(), name.getC_Str());
			String::destroy(&name);
			String::destroy(&targetdir);
			String::destroy(&sourcedir);

			return ptr;
		}


		/// <summary>
		/// puts the filepaths of the assets in the 
		/// </summary>
		/// <param name="out"></param>
		void serialize(YAML::Emitter& out);
		bool32 deserialize(YAML::Node& root);
		bool32 deserialize(const char* rootpath);

		bool32 saveAllToDisk();

	private:

		//sets all the asset parameters
		bool32 loadAssetInfo(YAML::Node& root, Asset* asset,const char* path) {

			auto id = root["UUID"];
			if (!id) {
				WARN << "asset at path: " << path << " doesnt have an ID\n";
				return false;
			}

			asset->id = id.as<UUID>();
			if (asset->id == 0) {
				WARN << "asset at path: " << path << "doesnt have a valid ID";
			}

			/*
			auto assettype = root["AssetType"];
			if (assettype) {
				asset->type = (AssetType)assettype.as<uint32>();
			} 
			*/

			asset->references = ArrayList<String>::create(0);
			auto references = root["references"];
			if (references) {
				for (auto ref : references) {
					asset->references.pushBack(ref.as<String>());
				}
			}

			asset->filepath = String::create(path);
			asset->internsource = false;
		}

		friend int32 AssetLoaderThreadProc(
			void* userdata
		);

		template<typename type>
		type* addLoadedAsset(type& asset, const char* path);
		
		CircularWorkQueue<LoaderThreadWork> loadqueue;
		ArrayList<LoaderThread*> loaderthreads;
		HANDLE workavailablesemaphore;

		assetreferences references;
		Library::AssetStorage assets;
	};
}