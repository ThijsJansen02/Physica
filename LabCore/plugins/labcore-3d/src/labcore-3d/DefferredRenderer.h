#pragma once
#include <glm/glm.hpp>
#include <Platform/platformAPI.h>
#include <Engine/GPUBuffer.h>
#include "assets/Mesh.h"
#include "assets/GraphicsPipeline.h"

namespace PH::LabCore3D {

	struct MeshInstance {
		uint32 materialindex;
		glm::vec4 trow0;
		glm::vec4 trow1;
		glm::vec4 trow2;
	};

	static PH::Platform::GFX::VertexInputBindingDescription mesh3Dvertexbindingdescription[] = {
		//vertexbuffer
		{0, sizeof(Vertex3D), Platform::GFX::INPUT_RATE_PER_VERTEX},

		//instancebuffer
		{1, sizeof(MeshInstance), Platform::GFX::INPUT_RATE_PER_INSTANCE}
	};

	static PH::Platform::GFX::VertexInputAttributeDescription mesh3Dvertexinputattributes[] = {

		//vertexinput
		{0, 0, Platform::GFX::FORMAT_R32G32B32_SFLOAT, offsetof(Vertex3D, position)},
		{0, 1, Platform::GFX::FORMAT_R32G32_SFLOAT, offsetof(Vertex3D, texcoord)},
		{0, 2, Platform::GFX::FORMAT_R32G32B32_SFLOAT, offsetof(Vertex3D, normal)},

		//instanceinput
		{1, 4, Platform::GFX::FORMAT_R8G8B8A8_UINT, offsetof(MeshInstance, materialindex)},
		{1, 5, Platform::GFX::FORMAT_R32G32B32A32_SFLOAT, offsetof(MeshInstance, trow0)},
		{1, 6, Platform::GFX::FORMAT_R32G32B32A32_SFLOAT, offsetof(MeshInstance, trow1)},
		{1, 7, Platform::GFX::FORMAT_R32G32B32A32_SFLOAT, offsetof(MeshInstance, trow2)}
	};

	struct CameraVariables {
		glm::mat4 projection;
		glm::mat4 view;
	};

	struct DefferedRenderer {

		static DefferedRenderer create();
		static DefferedRenderer create(sizeptr maxinstancecount);

		void setProjection(glm::mat4 projection);
		void setView(glm::mat4 view);

		void begin();
		void end();

		void drawMesh(Mesh* mesh, GraphicsPipeline* pipeline);

		Engine::DynamicGPUBuffer<MeshInstance> instancebuffer;
		Engine::DynamicGPUBuffer<CameraVariables> cameravariables;
		
		sizeptr instanceptr;

		static PH::Platform::GFX::DescriptorSetLayout createSceneDescriptorLayout();
		static PH::Platform::GFX::DescriptorSetLayout scenedescriptorlayout;
		PH::Platform::GFX::DescriptorSet scenedescriptorset;
	};
	
	extern DefferedRenderer renderer;
}