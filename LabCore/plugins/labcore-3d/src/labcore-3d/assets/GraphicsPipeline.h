#pragma once
#include <Engine/AssetDescription.h>
#include <Engine/GPUBuffer.h>

namespace PH::LabCore3D {

#define PH_ASSET_GRAPHICS_PIPELINE 983741375477342588

	enum PipelineType {
		LC_PIPELINE_TYPE_3D_DEFFERED_MATERIAL,
		LC_PIPELINE_TYPE_3D_FORWARD_MATERIAL,
		LC_PIPELINE_TYPE_3D_DEFFERED_LIGHTING,
		LC_PIPELINE_TYPE_2D_BATCHED_MATERIAL
	};

	enum BufferMemberBaseType {
		LC_BUFFER_MEMBER_UNDEFINED,
		LC_BUFFER_MEMBER_FLOAT,
		LC_BUFFER_MEMBER_UINT,
		LC_BUFFER_MEMBER_STRUCT
	};

	struct BufferMember {
		uint32 vecsize;
		uint32 columsize;
		uint32 arraysize;
		uint32 arraystride;
		uint32 offset;
		uint32 userbufferoffset;

		Engine::String name;

		BufferMemberBaseType type;
	};

	struct UniformBufferResource {
		uint32 buffersize;

		//offset in the uniform buffer that belongs to the pipeline, pipeline or derivatives have one large buffer that holds all user uniformbuffer resources
		uint32 userbufferoffset;
		Engine::ArrayList<BufferMember> members;
	};

	struct ShaderResource {
		PH::Platform::GFX::DescriptorType type;
		uint32 binding;
		uint32 set;

		union {
			UniformBufferResource* uniformbufferresource;
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
		PH::Platform::GFX::DescriptorSetLayout userlayout;
		PH::Platform::GFX::DescriptorSet defaultuserdescriptorset;

		//the user buffer with all user uniform buffer information
		Engine::StaticGPUBuffer<uint8> userbuffer;
		void* userbufferlocaldata;


		//path to the source path
		Engine::String srcpath;

		//path to the directory where the binaries are stored	
		Engine::String binpath;

		bool32 isvalid;
	};

	GraphicsPipeline createGraphicsPipeline(const char* sourcepath, const char* binpath, PipelineType type);

	bool32 reloadGraphicsPipelineFromGLSL(GraphicsPipeline* pipe);

	bool32 revalidateTextures(GraphicsPipeline* pipe);

	//revalidates the uniform buffers using the staging buffer, if the staging buffer is nullptr it will create one and delete it afterwards
	bool32 revalidateUniformBuffers(GraphicsPipeline* pipe, Engine::DynamicGPUBuffer<uint8>* stagingbuffer = nullptr);


	//bool32 revalidateDescriptorSet(GraphicsPipeline* pipe);

	//GraphicsPipeline createDefferedLightingPipeline(const char* sourcepaht, const char* binpath);

	Engine::AssetDescription createGraphicsPipelineDescription();
}