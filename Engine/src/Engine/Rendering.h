#pragma once
#include "Engine/Engine.h"
#include "Engine/assets/Mesh.h"
#include "Engine/assets/Material.h"
#include "Engine/Display.h"

namespace PH::Engine {

	namespace Renderer2D {

		struct QuadVertex {
			glm::vec2 position;
			glm::vec2 uvcoord;
		};

		struct ColoredQuadInstance {
			glm::mat2 scalerot;
			glm::vec3 position;
			glm::vec4 color;
			uint32 textureindex;
		};

		static PH::Platform::GFX::VertexInputBindingDescription quadvertexbindingdescription[] = {
			//vertexbuffer
			{0, sizeof(QuadVertex), Platform::GFX::INPUT_RATE_PER_VERTEX},

			//instancebuffer
			{1, sizeof(ColoredQuadInstance), Platform::GFX::INPUT_RATE_PER_INSTANCE}
		};

		static PH::Platform::GFX::VertexInputAttributeDescription quadvertexinputattributes[] = {

			//vertexinput
			{0, 0, Platform::GFX::FORMAT_R32G32_SFLOAT, offsetof(QuadVertex, position)},
			{0, 1, Platform::GFX::FORMAT_R32G32_SFLOAT, offsetof(QuadVertex, uvcoord)},

			//instanceinput
			{1, 2, Platform::GFX::FORMAT_R32G32B32A32_SFLOAT, offsetof(ColoredQuadInstance, scalerot)},
			{1, 3, Platform::GFX::FORMAT_R32G32B32_SFLOAT, offsetof(ColoredQuadInstance, position)},
			{1, 4, Platform::GFX::FORMAT_R32G32B32A32_SFLOAT, offsetof(ColoredQuadInstance, color)},
			{1, 5, Platform::GFX::FORMAT_R32_UINT, offsetof(ColoredQuadInstance, textureindex)}
		};

		const uint32 SCENE_UBO_BINDING = 1;
		const uint32 SCENE_UBO_SET = 0;
		const uint32 SCENE_SKYBOX_BINDING = 1;

		const uint32 TEXTURESAMPLER_BINDING = 0;
		const uint32 TEXTURESAMPLER_SET = 0;
		const uint32 MAX_TEXTURES = 32;

		struct Context;

		struct InitInfo {
			
			//currently used pipeline
			Platform::GFX::GraphicsPipeline currentpipeline;
			Base::Array<Platform::GFX::DescriptorSetLayout> descriptorsetlayouts;

			uint32 shadowmapdimensions;

			//gpu buffer
			sizeptr instancebuffersize;
		};

		bool32 begin(Context* context);
		bool32 end(Context* context);
		bool32 flush(Context* context, Base::Array<Platform::GFX::DescriptorSet> globaldescriptorsets);

		bool32 drawTexturedQuad(const glm::mat4& transform, Platform::GFX::Texture texture, Renderer2D::Context* context);

		Platform::GFX::GraphicsPipeline createGraphicsPipelineFromGLSLSource(const Engine::Display* target, const char* vertpath, const char* fragpath);

		bool32 drawColoredQuad(const glm::mat4& transform, glm::vec4 color, Renderer2D::Context* context);
		bool32 drawColoredQuad(glm::vec3 position, glm::vec2 size, real32 rotation, glm::vec4 color, Renderer2D::Context* context);
		bool32 drawColoredQuad(glm::vec3 position, glm::vec2 size, glm::vec4 color, Renderer2D::Context* context);

		bool32 drawLine(glm::vec3 v1, glm::vec3 v2, glm::vec2 thickness, Renderer2D::Context* context);
		bool32 drawLineStrip(Base::Array<glm::vec2> points, glm::vec2 thickness, Renderer2D::Context* context);

		bool32 drawLine(glm::vec3 v1, glm::vec3 v2, glm::vec4 color, glm::vec2 thickness, Renderer2D::Context* context);
		bool32 drawLineStrip(Base::Array<glm::vec2> points, glm::vec4 color, glm::vec2 thickness, Renderer2D::Context* context);


		bool32 setView(Context* context, const glm::mat4& view);
		bool32 setProjection(Context* context, const glm::mat4& projection);
		Context* createContext(const InitInfo& initinfo);
	}

	namespace Renderer3D {

		struct RenderStats {
			sizeptr trianglecount;
			sizeptr drawcalls;
		};

		extern Engine::Assets::Mesh cube;
		extern Engine::Assets::Mesh sphere;
		extern Engine::Assets::Mesh arrowshaft;
		extern Engine::Assets::Mesh arrowpoint;

		struct Context;

		Engine::Assets::Mesh createCube();

		typedef Assets::Vertex Vertex;

		struct DrawInstance {
			union {
				glm::mat3x4 transform;
				struct {
					glm::vec4 tv1;
					glm::vec4 tv2;
					glm::vec4 tv3;
				};
			};
			glm::vec4 color;
			uint32 objectid;
		};

		static PH::Platform::GFX::VertexInputBindingDescription vertexbindingdescription[] = {
			//vertexbuffer
			{0, sizeof(Vertex), Platform::GFX::INPUT_RATE_PER_VERTEX},

			//instancebuffer
			{1, sizeof(DrawInstance), Platform::GFX::INPUT_RATE_PER_INSTANCE}
		};

		static PH::Platform::GFX::VertexInputAttributeDescription vertexinputattributes[] = {

			//vertexinput
			{0, 0, Platform::GFX::FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)},
			{0, 1, Platform::GFX::FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)},
			{0, 2, Platform::GFX::FORMAT_R32G32_SFLOAT, offsetof(Vertex, UV)},

			//the transform matrix is transferred as 3 vec4s because there is only room for 4 real32s in a binding.
			//the last 
			{1, 3, Platform::GFX::FORMAT_R32G32B32A32_SFLOAT, offsetof(DrawInstance, tv1)},
			{1, 4, Platform::GFX::FORMAT_R32G32B32A32_SFLOAT, offsetof(DrawInstance, tv2)},
			{1, 5, Platform::GFX::FORMAT_R32G32B32A32_SFLOAT, offsetof(DrawInstance, tv3)},
			{1, 6, Platform::GFX::FORMAT_R32G32B32A32_SFLOAT, offsetof(DrawInstance, color)}, //color information
			{1, 7, Platform::GFX::FORMAT_R32_UINT, offsetof(DrawInstance, objectid)}
		};

		void begin(Renderer3D::Context* context);

		//draws a cube with the specified material...
		void drawCube(glm::mat4 transform, Engine::Assets::MaterialInstance* material, Renderer3D::Context* context);

		//draws a cube with the specified color
		void drawColoredCube(glm::mat4 transform, glm::vec4 color, Renderer3D::Context* context);
		void drawFlatColoredCube(glm::mat4 transform, glm::vec4 color, Renderer3D::Context* context);
		
		//draws mesh under the presumption that the mesh is loaded and all materials are loaded and ready to be rendered
		void drawMesh(
			glm::mat4 transform,
			Engine::Assets::Mesh* mesh,
			Base::Array<Engine::Assets::MaterialInstance*> materials,
			Renderer3D::Context* context
		);

		//draw a mesh without the presumption that it is loaded and all materials are loaded, when the mesh or a material is not yet loaded it just discards it.
		void drawMeshSafe(
			glm::mat4 transform,
			Assets::Mesh* mesh,
			Base::Array<Engine::Assets::MaterialInstance*> materials,
			Renderer3D::Context* context
		);


		void resetStats(Renderer3D::Context* context);
		RenderStats getStats(Renderer3D::Context* context);


		void end(Renderer3D::Context* context);
		void flush(Renderer3D::Context* context, Platform::GFX::DescriptorSet scenedescriptorset);

		struct InitInfo {
			Engine::Assets::MaterialInstance* colorshader;
			Engine::Assets::MaterialInstance* flatcolorshader;
		};

		Context* createContext(const InitInfo& initinfo);
	}
}