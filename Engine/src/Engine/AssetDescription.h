#pragma once
#include <Engine/Engine.h>
#include <yaml-cpp/yaml.h>
#include <Engine/YamlExtensions.h>

namespace PH::Engine {

	struct AssetIdentifier;

	typedef UUID AssetID;
	typedef UUID AssetDescriptionID;

	//base class for any asset that is 
	struct AssetBase {
		//unique key
		AssetID id;
		AssetIdentifier* identifier;
	};

	typedef void (*SerializeAsset)(YAML::Emitter& out, AssetBase* asset);
	typedef void (*deserializeAsset)(YAML::Node node, AssetBase* asset);
	//typedef void (*createAsset)(void* assetinitinfo);

	//holds the description for an asset 
	struct AssetDescription {

		//unique key for the asset description
		AssetDescriptionID id;
		const char* extension;
		
		SerializeAsset serialize_;
		deserializeAsset deserialize_;

		sizeptr localdatasize;
	};

}