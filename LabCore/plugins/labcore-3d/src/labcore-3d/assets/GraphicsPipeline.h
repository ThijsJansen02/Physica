#pragma once
#include <Engine/AssetDescription.h>
namespace PH::LabCore3D {

#define PH_ASSET_GRAPHICS_PIPELINE 983741375477342588

	enum PipelineType {
		PIPELINE_TYPE_3D_DEFFERED_MATERIAL,
		PIPELINE_TYPE_3D_MATERIAL,
		PIPELINE_TYPE_LIGHT_SHADER
	};

	struct GraphicsPipeline : public Engine::AssetBase {
		static Engine::AssetDescriptionID getAssetDescriptionID() {
			return PH_ASSET_GRAPHICS_PIPELINE;
		}

		//what kind of graphics pipeline this is
		PipelineType type;

		//the platform pipeline that gets created when creating the asset;
		PH::Platform::GFX::GraphicsPipeline internalpipeline;

		//path to the source path
		Engine::String srcpath;

		//path to the directory where the binaries are stored
		Engine::String bindir;
	};

	GraphicsPipeline createGraphicsPipeline(const char* sourcepath, const char* binpath);
	Engine::AssetDescription createGraphicsPipelineDescription();
}