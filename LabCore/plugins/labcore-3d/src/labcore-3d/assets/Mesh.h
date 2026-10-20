#pragma once
#include <Platform/platformAPI.h>
#include <Engine/AssetDescription.h>
#include "../Vertex.h"

namespace PH::LabCore3D {

#define PH_ASSET_MESH 2064075887894144755

	struct Mesh : public Engine::AssetBase {

		static Engine::AssetDescriptionID getAssetDescriptionID() {
			return PH_ASSET_MESH;
		}
		
		sizeptr indexcount;

		Platform::GFX::Buffer indexbuffer;
		Platform::GFX::Buffer vertexbuffer;
	};


	Mesh createCubeMesh();
	Mesh createMesh(Base::Array<Vertex3D> vertices, Base::Array<uint32> indices);
	Engine::AssetDescription createMeshAssetDescription();
}