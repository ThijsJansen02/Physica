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
		{0, 3, Platform::GFX::FORMAT_R32G32B32_SFLOAT, offsetof(Vertex3D, tangent)},
		{0, 4, Platform::GFX::FORMAT_R32G32B32_SFLOAT, offsetof(Vertex3D, bitangent)},


		//instanceinput
		{1, 5, Platform::GFX::FORMAT_R8G8B8A8_UINT, offsetof(MeshInstance, materialindex)},
		{1, 6, Platform::GFX::FORMAT_R32G32B32A32_SFLOAT, offsetof(MeshInstance, trow0)},
		{1, 7, Platform::GFX::FORMAT_R32G32B32A32_SFLOAT, offsetof(MeshInstance, trow1)},
		{1, 8, Platform::GFX::FORMAT_R32G32B32A32_SFLOAT, offsetof(MeshInstance, trow2)}
	};

	struct CameraVariables {
		glm::mat4 projection;
		glm::mat4 view;
	};

	struct GBuffer {
		sizeptr widhth;
		sizeptr height;

		PH::Platform::GFX::Framebuffer fb;
		PH::Platform::GFX::Texture albedo;
		PH::Platform::GFX::Texture normal;
		PH::Platform::GFX::Texture depth;
	};

	struct DefferedRendererCreateInfo {
		sizeptr maxinstancecount;
		sizeptr gbufferwidth;
		sizeptr gbufferheight;


		//initial values, can be changed on the fly
		PH::Platform::GFX::Viewport viewport;
		glm::vec4 clearcolor;
	};


	struct DefferedRenderer {

		enum OutputSource {
			DEFFERED_ALBEDO = 0,
			DEFFERED_NORMAL,
			DEFFERED_LIGHTING
		};

		static DefferedRenderer create();
		static DefferedRenderer create(const DefferedRendererCreateInfo& info);

		static PH::Platform::GFX::RenderpassDescription createGeometryRenderpass();
		static PH::Platform::GFX::RenderpassDescription geometryrenderpass;

		static PH::Platform::GFX::DescriptorSetLayout createSceneDescriptorLayout();
		static PH::Platform::GFX::DescriptorSetLayout scenedescriptorlayout;

		static PH::Platform::GFX::RenderpassDescription createLightingRendererpass();
		static PH::Platform::GFX::RenderpassDescription lightingrenderpass;

		void beginGeometryPass();
		void setProjection(glm::mat4 projection);
		void setView(glm::mat4 view);
		void drawMesh(Mesh* mesh, GraphicsPipeline* pipeline);
		void drawMesh(Mesh* mesh, GraphicsPipeline* pipeline, const glm::mat4& transform);
		void endGeometryPass();

		void applyLightpass(GraphicsPipeline* pipe);
		void applyPostProcess();

		void output(Engine::Display* target, OutputSource source);

		PH::Platform::GFX::Viewport viewport;
		glm::vec4 albedoclearcolor;

		GBuffer gbuffer;

		Platform::GFX::Framebuffer lightpassbuffer;
		Platform::GFX::Texture lightpassattachment;

		Engine::DynamicGPUBuffer<MeshInstance> instancebuffer;
		Engine::DynamicGPUBuffer<CameraVariables> cameravariables;

		sizeptr instanceptr;

		PH::Platform::GFX::DescriptorSet scenedescriptorset;

	private:
		void begin();
		void end();
	};
	
	extern DefferedRenderer renderer;
}