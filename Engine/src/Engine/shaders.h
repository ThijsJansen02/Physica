#pragma once
#include <Engine/Engine.h>

namespace PH::Engine {

	struct ShaderBinariesHeader {
		sizeptr vertbegin;
		sizeptr vertend;
		sizeptr fragbegin;
		sizeptr fragend;
	};

	struct Binaries {
		Engine::DynamicArray<uint8> vertsrc;
		Engine::DynamicArray<uint8> fragsrc;
	};

	Engine::DynamicArray<uint8> compileGLSLSourceToVulkanBinary(const char* source, PH::Platform::GFX::ShaderStageFlags shaderstage);
	bool32 writeBinariesToDisk(const Binaries& b, const char* binpath);
	//reloads the graphics pipeline from glsl source
	bool32 compileBinariesFromGLSL(const char* path, Binaries* b);
	bool32 loadBinariesFromDisk(const char* path, Binaries* binaries);
	bool32 loadBinaries(Binaries* b, const char* srcpath, const char* binpath);

	Platform::GFX::GraphicsPipeline createGraphicsPipelineFromBinaries(
		Platform::GFX::RenderpassDescription renderpass,
		const Binaries& b,
		Base::Array<PH::Platform::GFX::DescriptorSetLayout> userlayouts,
		Base::Array<PH::Platform::GFX::VertexInputAttributeDescription> vertexinputattributes,
		Base::Array<PH::Platform::GFX::VertexInputBindingDescription> vertexinputbindings
	);

	Platform::GFX::GraphicsPipeline createGraphicsPipelineFromBinaries(
		Platform::GFX::RenderpassDescription renderpass,
		const Binaries& b,
		Base::Array<PH::Platform::GFX::DescriptorSetLayout> userlayouts,
		Base::Array<PH::Platform::GFX::VertexInputAttributeDescription> vertexinputattributes,
		Base::Array<PH::Platform::GFX::VertexInputBindingDescription> vertexinputbindings,
		Base::Array<PH::Platform::GFX::PushConstantRange> pushconstants,
		bool32 depthtest,
		bool32 blend
	);
}