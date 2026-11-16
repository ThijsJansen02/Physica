#pragma once
#include <Platform/platformAPI.h>
#include <Engine/Display.h>
#include <Engine/GPUBuffer.h>
#include <glm/glm.hpp>

namespace PH::Engine {

	//batch renderer to draw simple primitives as quads, circles, lines, and text
	struct PH_ENGINE_API BatchRenderer2D {
		struct QuadVertex {
			glm::vec2 position;
			glm::vec2 uvcoord;
		};

		struct ColoredQuadInstance {
			glm::mat2 scalerot;
			glm::vec3 position;
			glm::vec4 color;
			uint32 textureindex;
			uint32 objectid;
		};

		struct Camera {
			glm::mat4 view;
			glm::mat4 projection;
		};

		static constexpr PH::Platform::GFX::VertexInputBindingDescription quadvertexbindingdescription[] = {
			//vertexbuffer
			{0, sizeof(QuadVertex), Platform::GFX::INPUT_RATE_PER_VERTEX},

			//instancebuffer
			{1, sizeof(ColoredQuadInstance), Platform::GFX::INPUT_RATE_PER_INSTANCE}
		};

		static constexpr PH::Platform::GFX::VertexInputAttributeDescription quadvertexinputattributes[] = {

			//vertexinput
			{0, 0, Platform::GFX::FORMAT_R32G32_SFLOAT, offsetof(QuadVertex, position)},
			{0, 1, Platform::GFX::FORMAT_R32G32_SFLOAT, offsetof(QuadVertex, uvcoord)},

			//instanceinput
			{1, 2, Platform::GFX::FORMAT_R32G32B32A32_SFLOAT, offsetof(ColoredQuadInstance, scalerot)},
			{1, 3, Platform::GFX::FORMAT_R32G32B32_SFLOAT, offsetof(ColoredQuadInstance, position)},
			{1, 4, Platform::GFX::FORMAT_R32G32B32A32_SFLOAT, offsetof(ColoredQuadInstance, color)},
			{1, 5, Platform::GFX::FORMAT_R32_UINT, offsetof(ColoredQuadInstance, textureindex)}
		};


		//internal framebuffer, used when 2d renderer is used as main renderer. is a simple ldr framebuffer with depth/stencil attachment
		struct Framebuffer {
			Platform::GFX::Framebuffer fb;
			Platform::GFX::Texture colorattachment;
			Platform::GFX::Texture depthattchment;

			sizeptr width;
			sizeptr height;
		};

		static Platform::GFX::RenderpassDescription geometryrenderpass;
		static Platform::GFX::RenderpassDescription createGeometryRenderpass();

		//have to be called at the begin of the frame and at the end of the frame
		void beginFrame();
		void endFrame();

		void begin();
		void end();
		
		void pushView(glm::mat4 view);
		void pushProjection(glm::mat4 proj);

		//if framebuffer is specified, begingeometrypass uses that framebuffer otherwise it will use the internal fb.
		void beginGeometryPass(bool32 clearcolor, bool32 cleardepth, Framebuffer* frambuffer = nullptr);

		void drawQuad(const glm::vec3& position, const glm::mat2& rotscale, const glm::vec4& color, uint32 matindex, uint32 objectid);
		void drawRectangle(const glm::vec3& position, real32 rotation, const glm::vec2& scale, const glm::vec4 color = glm::vec4(1.0f));
		void drawCircle(const glm::vec3& position, real32 rotation, const glm::vec2& radius, const glm::vec4 color = glm::vec4(1.0f));
		void drawLine(const glm::vec2& v1, const glm::vec2& v2, real32 depth, real32 thickness, const glm::vec4& color = glm::vec4(1.0f));

		void pushView(const glm::mat4& view);
		void pushPojection(const glm::mat4& projection);

		//can be called manually when the user wants to flush the geometry to the fb, is called in endgeometrypass
		void flush();
		void endGeometryPass();

		//outputs the internal framebuffer to a display
		void output(Engine::Display* output);

		//create a batch graphics pipe from the binaries if they exist and otherwise from the src. userlayouts are layouts the pipeline uses next to the set 0, which holds the texture array
		static PH::Platform::GFX::GraphicsPipeline createBatchPipeline(const char* srcpath, const char* binpath, Base::Array<PH::Platform::GFX::DescriptorSetLayout> userlayouts, bool32 blend, bool32 depthtest);
		//PH::Platform::GFX::GraphicsPipeline createBatchPipeline();

		PH::Platform::GFX::GraphicsPipeline quadshader;
		PH::Platform::GFX::GraphicsPipeline circleshader;
		PH::Platform::GFX::GraphicsPipeline lineshader;
		PH::Platform::GFX::GraphicsPipeline fontshader;

		PH::Platform::GFX::GraphicsPipeline currentpipeline;

		Camera camera;

		struct CreateInfo {
			PH::Platform::GFX::GraphicsPipeline quadshader;
			PH::Platform::GFX::GraphicsPipeline lineshader;
			PH::Platform::GFX::GraphicsPipeline fontshader;
			PH::Platform::GFX::GraphicsPipeline circleshader;

			sizeptr internalwidth;
			sizeptr internalheight;

			//if the internalfb is flagged, an internal framebuffer is created to draw to. if not a framebuffer has to be supplied to the begin geometry pass
			bool32 internalfb;

			sizeptr instancebuffersize;
		};

		static BatchRenderer2D create(const CreateInfo& create);

		PH::Platform::GFX::Viewport viewport;

		Engine::StaticGPUBuffer<QuadVertex> quadvertexbuffer;
		Engine::StaticGPUBuffer<uint32> quadindexbuffer;

		Framebuffer internalframebuffer;
		Engine::DynamicGPUBuffer<ColoredQuadInstance> instancebuffer;
		sizeptr instance_end;
		sizeptr instance_begin;
		
	};
}