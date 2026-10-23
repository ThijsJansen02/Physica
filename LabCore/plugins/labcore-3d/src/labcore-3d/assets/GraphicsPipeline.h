#pragma once
#include <Engine/AssetDescription.h>
namespace PH::LabCore3D {

#define PH_ASSET_GRAPHICS_PIPELINE 983741375477342588

	enum PipelineType {
		LC_PIPELINE_TYPE_3D_DEFFERED_MATERIAL,
		LC_PIPELINE_TYPE_3D_FORWARD_MATERIAL,
		LC_PIPELINE_TYPE_3D_DEFFERED_LIGHTING,
		LC_PIPELINE_TYPE_2D_BATCHED_MATERIAL
	};


	struct ShaderResource {
		PH::Platform::GFX::DescriptorType type;
		uint32 binding;
		uint32 set;

		union {
			Platform::GFX::Buffer buffer;
			Engine::AssetIdentifier* texture;
		};

		Engine::String name;
	};

	struct GraphicsPipeline : public Engine::AssetBase {
		static Engine::AssetDescriptionID getAssetDescriptionID() {
			return PH_ASSET_GRAPHICS_PIPELINE;
		}

		//what kind of graphics pipeline this is
		PipelineType type;

		//the platform pipeline that gets created when creating the asset;
		PH::Platform::GFX::GraphicsPipeline internalpipeline;

		//the resources the shader requirers
		Engine::ArrayList<ShaderResource> resources;
		PH::Platform::GFX::DescriptorSetLayout descriptorsetlayout;
		PH::Platform::GFX::DescriptorSet defaultdescriptorset;

		//path to the source path
		Engine::String srcpath;

		//path to the directory where the binaries are stored
		Engine::String binpath;
	};

	//api should definitly be different in the future
	void updateDescriptorSet(Base::Array<ShaderResource> resources, Platform::GFX::DescriptorSet set, uint32 targetset);


	GraphicsPipeline createGraphicsPipeline(const char* sourcepath, const char* binpath, PipelineType type);

	bool32 reloadGraphicsPipelineFromGLSL(GraphicsPipeline* pipe);

	//GraphicsPipeline createDefferedLightingPipeline(const char* sourcepaht, const char* binpath);

	Engine::AssetDescription createGraphicsPipelineDescription();
}