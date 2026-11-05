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

	Mesh generateSphereMesh(
		float radius,
		uint32_t longitudeSegments,
		uint32_t latitudeSegments)
	{
		Engine::ArrayList<Vertex3D> vertices = Engine::ArrayList<Vertex3D>::create();
		Engine::ArrayList<uint32> indices = Engine::ArrayList<uint32>::create();

		if (longitudeSegments < 3 || latitudeSegments < 2 || radius <= 0.0f)
			return {};

		constexpr float PI = glm::pi<float>();

		for (uint32_t y = 0; y <= latitudeSegments; ++y)
		{
			float v = static_cast<float>(y) / latitudeSegments;
			float theta = v * PI;

			for (uint32_t x = 0; x <= longitudeSegments; ++x)
			{
				float u = static_cast<float>(x) / longitudeSegments;
				float phi = u * 2.0f * PI;

				float sinTheta = sin(theta);
				float cosTheta = cos(theta);

				float sinPhi = sin(phi);
				float cosPhi = cos(phi);

				glm::vec3 normal(
					sinTheta * cosPhi,
					cosTheta,
					sinTheta * sinPhi
				);

				glm::vec3 tangent(
					-sinPhi,
					0.0f,
					cosPhi
				);

				glm::vec3 bitangent = glm::cross(normal, tangent);

				Vertex3D vertex;
				vertex.position = normal * radius;
				vertex.normal = normal;
				vertex.texcoord = glm::vec2(u, v);
				vertex.tangent = tangent;
				vertex.bitangent = bitangent;

				vertices.pushBack(vertex);
			}
		}

		for (uint32_t y = 0; y < latitudeSegments; ++y)
		{
			for (uint32_t x = 0; x < longitudeSegments; ++x)
			{
				uint32_t current =
					y * (longitudeSegments + 1) + x;

				uint32_t next = current + longitudeSegments + 1;

				indices.pushBack(current);
				indices.pushBack(next);
				indices.pushBack(current + 1);

				indices.pushBack(current + 1);
				indices.pushBack(next);
				indices.pushBack(next + 1);
			}
		}

		return createMesh(vertices.getArray(), indices.getArray());
	}

	
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