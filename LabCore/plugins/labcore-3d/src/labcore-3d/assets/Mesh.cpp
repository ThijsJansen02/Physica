#include "Mesh.h"


namespace PH::LabCore3D {

	// Generates the full 24-vertex array for a standard 1x1x1 cube centered at the origin
	Vertex3D cubeVertices[] = {
		// === FRONT FACE (Normal: 0, 0, 1) ===
		{ {-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} }, // Bottom-Left
		{ { 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} }, // Bottom-Right
		{ { 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} }, // Top-Right
		{ {-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} }, // Top-Left

		// === BACK FACE (Normal: 0, 0, -1) ===
		{ { 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} },
		{ {-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} },
		{ {-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} },
		{ { 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} },

		// === TOP FACE (Normal: 0, 1, 0) ===
		{ {-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f} },
		{ { 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f} },
		{ { 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f} },
		{ {-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f} },

		// === BOTTOM FACE (Normal: 0, -1, 0) ===
		{ {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f} },
		{ { 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f} },
		{ { 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f} },
		{ {-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f} },

		// === RIGHT FACE (Normal: 1, 0, 0) ===
		{ { 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f} },
		{ { 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f} },
		{ { 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f} },
		{ { 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f} },

		// === LEFT FACE (Normal: -1, 0, 0) ===
		{ {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f} },
		{ {-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f} },
		{ {-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f} },
		{ {-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f} }
	};

	// Counter-Clockwise index buffer specifying 2 triangles per face (12 total triangles)
	uint32 cubeIndices[] = {
		 0,  1,  2,     2,  3,  0,    // Front face
		 4,  5,  6,     6,  7,  4,    // Back face
		 8,  9, 10,    10, 11,  8,    // Top face
		12, 13, 14,    14, 15, 12,    // Bottom face
		16, 17, 18,    18, 19, 16,    // Right face
		20, 21, 22,    22, 23, 20     // Left face
	};

	
	void serializeMesh(YAML::Emitter& out, Mesh* mesh) {

	}

	void deserializeMesh(YAML::Node node, Mesh* mesh) {

	}

	Mesh createCubeMesh() {
		return createMesh({ (Vertex3D*)cubeVertices, 24 }, { cubeIndices, 36 });
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