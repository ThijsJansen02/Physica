#pragma once
#include <Engine/Engine.h>


namespace PH::Engine {

	struct AssetIdentifier;

	//base class for any asset that is 
	struct AssetBase {
		//unique key
		UUID assetid;
		AssetIdentifier* identifier;
	};

	typedef void (*SerializeAsset)(const char* filepath, AssetBase* asset);
	typedef void (*deserializeAsset)(const char* filepath, AssetBase* asset);
	//typedef void (*createAsset)(void* assetinitinfo);

	//holds the description for an asset 
	struct AssetDescription {

		//unique key
		Engine::String extension;

		SerializeAsset serialize_;
		deserializeAsset deserialize_;

		sizeptr localdatasize;

		Engine::String name;

	};

}