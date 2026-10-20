#include "Mesh.h"


namespace PH::LabCore3D {

	real32 cubeVertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  1.0f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  1.0f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  1.0f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  1.0f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  1.0f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  1.0f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f,  1.0f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  1.0f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  1.0f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f
	};

	uint32 indices[]{
		0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35
	};

	
	void serializeMesh(YAML::Emitter& out, Mesh* mesh) {

	}

	void deserializeMesh(YAML::Node node, Mesh* mesh) {

	}

	Mesh createCubeMesh() {
		return createMesh({ (Vertex3D*)cubeVertices, 36 }, { indices, 36 });
	}

	Mesh createMesh(Base::Array<Vertex3D> vertices, Base::Array<uint32> indices) {

		Mesh result;
		PH::Platform::GFX::BufferCreateinfo vci{};
		vci.bufferusage = PH::Platform::GFX::BUFFER_USAGE_VERTEX_BUFFER_BIT;
		vci.data = vertices.data;
		vci.size = vertices.count * sizeof(Vertex3D);
		vci.dynamic = false;
		vci.memoryproperties = PH::Platform::GFX::MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		PH::Platform::GFX::BufferCreateinfo ici{};
		ici.bufferusage = PH::Platform::GFX::BUFFER_USAGE_INDEX_BUFFER_BIT;
		ici.data = indices.data;
		ici.size = indices.count * sizeof(uint32);
		ici.dynamic = false;
		ici.memoryproperties = PH::Platform::GFX::MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		PH::Platform::GFX::BufferCreateinfo infos[] = { vci, ici };
		PH::Platform::GFX::Buffer buffers[2];
		PH::Platform::GFX::createBuffers(infos, buffers, 2);
		
		result.indexcount = indices.count;
		result.vertexbuffer = buffers[0];
		result.indexbuffer = buffers[1];

		return result;
	}

	Engine::AssetDescription createMeshAssetDescription() {

		Engine::AssetDescription d;
		d.localdatasize = sizeof(Mesh);
		d.deserialize_ = (Engine::deserializeAsset)deserializeMesh;
		d.serialize_ = (Engine::SerializeAsset)serializeMesh;	
		d.extension = ".lcmesh";
		d.id = Mesh::getAssetDescriptionID();
		return d;
	}

}