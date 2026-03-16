#pragma once
#include <Platform/platformAPI.h>
#include <Base/Datastructures/HashMap.h>

#include "Engine/asset.h"
#include "../YamlExtensions.h"

namespace PH::Engine::Assets {

	class Library;

	enum class MemberType {
		REAL32,
		INT32,
		UINT32
	};

	struct Member {
		String name;
		MemberType format;

		uint32 rows;
		uint32 columns;

		uint32 offset;
	};

	enum struct UniformResourceType {
		NONE = 0,
		UNIFORM_BUFFER,
		UNIFORM_SAMPLER
	};

	struct UniformBufferSubResource {
		Platform::GFX::Buffer base;
		sizeptr size;
		sizeptr offset;
	};

	void* mapUniformBufferSubResource(const UniformBufferSubResource& res);
	struct TextureImage;

	struct UniformSamplerDescription {
		Engine::ArrayList<TextureImage*> boundimages;
	};

	struct UniformBufferDescription {
		ArrayList<Member> members;
		uint32 size;
		Engine::ArrayList<UniformBufferSubResource> boundbuffers;
	};

	struct UniformResource {

		String name;
		uint32 binding;
		uint32 arraysize;
		UniformResourceType type;
		Platform::GFX::ShaderStageFlags shaderstages;

		union {
			UniformBufferDescription ubodescription;
			UniformSamplerDescription samplerdescription;
		};
	};

	struct ShaderModule {
		String binarypath;
		String sourcecode;

		void* loadedbinary;
		sizeptr loadedbinarysize;

		Platform::GFX::Shader shader;
	};
	
	struct UniformBuffer {
		PH::Platform::GFX::Buffer base;
		sizeptr size;
	};

	using ResourceMap = Base::ChainedHashMap<uint32, UniformResource, Base::uint32Hash, Base::uint32Compare, Engine::Allocator>;

	struct MaterialInstance;
	struct Material;

	//serialization methods
	bool32 deserializeMaterialInstance(const char* path, Library* lib, MaterialInstance* result);
	bool32 deserializeMaterialInstance(YAML::Node& root, Library* lib, MaterialInstance* result, const char* path);

	bool32 serializeMaterialInstance(const char* path, MaterialInstance* material);
	bool32 serializeMaterialInstance(YAML::Emitter& out, MaterialInstance* material);

	bool32 deserializeMaterial(YAML::Node& root, Library* lib, Material* result, const char* path);
	bool32 deserializeMaterial(const char* path, Library* lib, Material* result);

	bool32 serializeMaterial(YAML::Emitter& out, Material* material);
	bool32 serializeMaterial(const char* path, Material* mat);

	struct Material : public Asset {

		static constexpr const char* getExtension() {
			return ".phmat";
		}

		static constexpr AssetType getType() {
			return AssetType::MATERIAL;
		}

		//the eventual graphics pipeline
		Platform::GFX::GraphicsPipeline pipeline;

		//the layout of the descriptor set that is to be bound to the material
		Platform::GFX::DescriptorSetLayout layout;
		
		//a mapping of all the uniform resources that the shader needs
		ResourceMap resources;

		//the instances that exist for this material
		ArrayList<UUID> instances;

		//the material render settings
		Platform::GFX::CullModeFlags cullmode;
		bool32 depthtest;

		uint32 texturecount;
		ShaderModule fragmentshader;
		ShaderModule vertexshader;

		bool32 compiled;

		friend class Assets::Library;
	private:

		//serializes the asset into the yaml emitter
		static void serialize(YAML::Emitter& out, Material* instance) {
			serializeMaterial(out, instance);
		}

		//deserializes the asset, the path parameter is only there for debug messages...
		static bool32 deserialize(YAML::Node& root, Library* lib, Material* result, const char* path) {
			return deserializeMaterial(root, lib, result, path);
		}
	};

	struct MaterialInstance : public Asset {

		static constexpr const char* getExtension() {
			return ".phmatinst";
		}

		static constexpr AssetType getType() {
			return AssetType::MATERIAL_INSTANCE;
		}

		bool32 Ready() {
			return (status == AssetStatus::LOADED);
		}
		
		Engine::Assets::Material* parent;
		ResourceMap resources;
		Platform::GFX::DescriptorSet descriptorset;

	private:
		friend class Library;

		///the methods are added to make the process of incorporating the asset into the assetlibrary automatic

		//serializes the asset into the yaml emitter
		static void serialize(YAML::Emitter& out, MaterialInstance* instance) {
			serializeMaterialInstance(out, instance);
		}

		//deserializes the asset, the path parameter is only there for debug messages...
		static bool32 deserialize(YAML::Node& root, Library* lib, MaterialInstance* result, const char* path) {
			return deserializeMaterialInstance(root, lib, result, path);
		}
	};

	Material createMaterial(const char* bindir, const char* binname);

	//material compilation
	bool32 compileMaterialFromBinaries(Material* material, Platform::GFX::RenderpassDescription intendedrenderpass, Platform::GFX::DescriptorSetLayout scenedescriptorsetlayout);
	bool32 compileMaterialFromSource(Material* material, Platform::GFX::RenderpassDescription intendedrenderpass, Platform::GFX::DescriptorSetLayout scenedescriptorsetlayout);
	bool32 compileMaterialFromBinaries(Material* material, Platform::GFX::RenderpassDescription intendedrenderpass);
	bool32 compileMaterialFromSource(Material* material, Platform::GFX::RenderpassDescription intendedrenderpass);
	

	//material instance creation
	bool32 createMaterialInstance(Material* sourcematerial, MaterialInstance* instance, TextureImage* defaultimage);
	bool32 setTextureForMaterialInstance(MaterialInstance* instance, uint32 binding, uint32 arrayelement, TextureImage* image);

	//sets texture after checking if the texture is actually loaded, if it is not loaded it loads the image directly
	bool32 setTextureForMaterialInstanceSafe(MaterialInstance* instance, uint32 binding, uint32 arrayelement, TextureImage* image, Library* lib);
	bool32 rebuildMaterialInstance(MaterialInstance* inst, TextureImage* defaultimage);

}