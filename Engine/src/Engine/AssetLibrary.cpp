#pragma once
#include "AssetLibrary.h"
#include "assets/Material.h"
#include "assets/Mesh.h"
#include "assets/Texture.h"
#include "Assets/Script.h"
#include "Scene.h"

#include <yaml-cpp/yaml.h>
#include "Display.h"

#include "YamlExtensions.h"

#include <intrin.h>


#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace PH::Engine::Assets {

	using namespace Platform;

	int32 AssetLoaderThreadProc(
		void* userdata
	) {

		//every thread needs its own arenaallocator because arena allocators are not threadsafe
		ArenaAllocator::arena = Base::createMemoryArena(Engine::Allocator::alloc(1 * KILO_BYTE), 1 * KILO_BYTE);

		ThreadInfo* info = (ThreadInfo*)userdata;
		while (true && info->open) {
			ArenaScope scope;

			LoaderThreadWork* work = info->lib->loadqueue.pop();
			if (work) {

				//INFO << "Arena resetpoint: " << ArenaAllocator::getResetPoint() << "\n";

				Asset* assettobeloaded = work->assettobeloaded;
				INFO << "Thread: " << (uint32)info->threadid << " is loading asset: " << assettobeloaded->references[0].getC_Str() << "\n";

				if (assettobeloaded->type == AssetType::MESH) {
					info->lib->load<Mesh>(assettobeloaded->id);
				} else if (assettobeloaded->type == AssetType::MATERIAL_INSTANCE) {
					info->lib->load<MaterialInstance>(assettobeloaded->id);
				} else {
					INFO << "cant load an asset of this type!\n";
				}
			} else {
				INFO << "thread: " << (uint32)info->threadid << " is going to sleep\n";
				WaitForSingleObjectEx(info->semaphore, INFINITE, FALSE);
				INFO << "thread: " << (uint32)info->threadid << " woke up\n";
			}
		}
		return 0;
	}

	Library* Library::create() {

		//now uses the win32 API directly, subsequent calls to the win32 api should be abstracted into the platform api
		Library* lib = (Library*)Engine::Allocator::alloc(sizeof(Library));
		*lib = {};

		lib->assets = Library::AssetStorage::create(100);
		lib->references = assetreferences::create(10);

		lib->loadqueue.storage = Engine::DynamicArray<LoaderThreadWork>::create(50);
		lib->loadqueue.readptr = 0;
		lib->loadqueue.writeptr = 0;

		uint32 nthreads = 4;

		HANDLE semaphore = CreateSemaphoreEx(0, 0, nthreads, nullptr, 0, SEMAPHORE_ALL_ACCESS);
		lib->workavailablesemaphore = semaphore;

		lib->loaderthreads = Engine::ArrayList<Assets::LoaderThread*>::create(nthreads);

		for (uint32 i = 0; i < nthreads; i++) {
			
			LoaderThread* thread = (LoaderThread*)Engine::Allocator::alloc(sizeof LoaderThread);
			thread->info.open = true;
			thread->info.lib = lib;
			thread->info.semaphore = semaphore;

			_ReadWriteBarrier();

			ThreadCreateInfo threadcreate{};
			threadcreate.threadproc = AssetLoaderThreadProc;
			threadcreate.threadworkmemorysize = KILO_BYTE;
			threadcreate.usegfx = true;
			threadcreate.userdata = &thread->info;

			PH::Platform::createThread(threadcreate, &thread->thread);
			thread->info.threadid = thread->thread.id;

			lib->loaderthreads.pushBack(thread);
		}
		return lib;
	}

	using SubString = Base::SubString;

	bool32 Library::saveAllToDisk() {
		Engine::ArenaScope s;

		for (auto asset : assets.getAllOfType<MaterialInstance, Engine::ArenaAllocator>()) {
			saveToDisk<MaterialInstance>(asset);
		}

		for (auto asset : assets.getAllOfType<Material, Engine::ArenaAllocator>()) {
			saveToDisk<Material>(asset);
		}

		for (auto asset : assets.getAllOfType<Mesh, Engine::ArenaAllocator>()) {
			saveToDisk<Mesh>(asset);
		}

		for (auto asset : assets.getAllOfType<TextureImage, Engine::ArenaAllocator>()) {
			saveToDisk<TextureImage>(asset);
		}

		for (auto asset : assets.getAllOfType<Scene, Engine::ArenaAllocator>()) {
			saveToDisk<Scene>(asset);
		}

		for (auto asset : assets.getAllOfType<Script, Engine::ArenaAllocator>()) {
			saveToDisk<Script>(asset);
		}

		return true;
	}

	void* mapUniformBufferSubResource(const UniformBufferSubResource& res) {
		void* result;
		GFX::mapBuffer(&result, res.base);
		return (uint8*)result + res.offset;
	}

	template<typename type>
	void serializeAssetType(YAML::Emitter& out, const char* key, Library* lib) {
		out << YAML::Key << key << YAML::Value << YAML::BeginSeq; //materials;

		Base::ArrayList<type*, Engine::ArenaAllocator> assets = lib->getAll<type, Engine::ArenaAllocator>();
		for (auto a : assets) {
			out << a->filepath.getC_Str();
		}
		out << YAML::EndSeq;// materials;

	}

	void Library::serialize(YAML::Emitter& out) {

		auto reset = Engine::ArenaAllocator::getResetPoint();

		out << YAML::BeginMap; //assets
		serializeAssetType<Material>(out, "Materials", this);
		serializeAssetType<TextureImage>(out, "Textures", this);
		serializeAssetType<MaterialInstance>(out, "MaterialInstances", this);
		serializeAssetType<Mesh>(out, "Meshes", this);
		serializeAssetType<Scene>(out, "Scenes", this);
		out << YAML::EndMap; //assets;
	}

	template<typename type>
	void includeAssetType(const YAML::Node& r, Library* lib) {
		for (auto a : r) {
			String filepath = a.as<String>();
			lib->include<type>(filepath.getC_Str());
		}
	}

	template<typename type>
	void loadAssetType(YAML::Node& r, Library* lib) {
		for (auto a : r) {
			String filepath = a.as<String>();
			lib->load<type>(filepath.getC_Str());
		}
	}

	char* copyNextItem(const char* src, char* dst, sizeptr size) {
		size--;
		while (*src != '\0' && *src != ';' && size > 0) {
			
			*dst = *src;
			dst++;
			src++;
			size--;
		}
		*dst = '\0';
		if (*src == ';') {
			return (char*)++src;
		}
		return nullptr;
	}

	bool32 Library::deserialize(const char* rootpath) {

		Engine::ArenaScope s;

		auto root = Base::String<ArenaAllocator>::create(rootpath);

		char buffer[2048] = {};
		PH::Platform::listFiles(buffer, 2048, rootpath);

		char* readptr = buffer;
		
		auto filename = Base::String<ArenaAllocator>::create(128);
		while (readptr) {

			Engine::ArenaScope s2;

			readptr = copyNextItem(readptr, (char*)filename.getC_Str(), 128);
			SubString ext = filename.getSubString().getFromLast('.');

			if (filename == ".") {
				continue;
			}

			if (filename == "..") {
				continue;
			}

			auto filepath = Base::String<ArenaAllocator>::create(rootpath).append("/").append(filename.getC_Str());

			if (ext.getLength() == 0) {
				deserialize(filepath.getC_Str());
				continue;
			}


			if (ext == Mesh::getExtension()) {
				Engine::INFO << "including Mesh: " << filename.getC_Str() << "\n";
				include<Mesh>(filepath.getC_Str());
				continue;
			}

			if (ext == MaterialInstance::getExtension()) {
				Engine::INFO << "including Materialinstance: " << filename.getC_Str() << "\n";
				include<MaterialInstance>(filepath.getC_Str());
				continue;
			}

			if (ext == Material::getExtension()) {
				Engine::INFO << "including Material: " << filename.getC_Str() << "\n";
				include<Material>(filepath.getC_Str());
				continue;
			}

			if (ext == TextureImage::getExtension()) {
				Engine::INFO << "including TextureImage: " << filename.getC_Str() << "\n";
				include<TextureImage>(filepath.getC_Str());
				continue;
			}

			if (ext == Scene::getExtension()) {
				Engine::INFO << "including Scene: " << filename.getC_Str() << "\n";
				include<Scene>(filepath.getC_Str());
				continue;
			}

			if (ext == Script::getExtension()) {
				Engine::INFO << "including Script: " << filename.getC_Str() << "\n";
				include<Script>(filepath.getC_Str());
				continue;
			}
		}
		return true;
	}

	bool32 Library::deserialize(YAML::Node& root) {
		
		includeAssetType<TextureImage>(root["Textures"], this);
		includeAssetType<Material>(root["Materials"], this);
		includeAssetType<MaterialInstance>(root["MaterialInstances"], this);
		includeAssetType<Mesh>(root["Meshes"], this);
		includeAssetType<Scene>(root["Scenes"], this);

		return true;
	}
}