#pragma once

#include <Engine/Rendering.h>

namespace PH::Engine {

	namespace Renderer2D {

		class Wrapper {
		public:
			bool32 begin() {
				return Renderer2D::begin(m_Context);
			}
			bool32 end() {
				return Renderer2D::end(m_Context);
			}
			bool32 flush(Base::Array<Platform::GFX::DescriptorSet> globaldescriptorsets) {
				return Renderer2D::flush(m_Context, globaldescriptorsets);
			}
			bool32 drawTexturedQuad(const glm::mat4& transform, Platform::GFX::Texture texture) {
				return Renderer2D::drawTexturedQuad(transform, texture, m_Context);
			}

			bool32 drawTexturedQuad(glm::vec3 position, glm::vec2 size, Platform::GFX::Texture texture) {
				return Renderer2D::drawTexturedQuad(position, size, texture, m_Context);
			}

			static Platform::GFX::GraphicsPipeline createGraphicsPipelineFromGLSLSource(const Engine::Display* target, const char* vertpath, const char* fragpath, Base::Array<Platform::GFX::DescriptorSetLayout> userlayout) {
				return Renderer2D::createGraphicsPipelineFromGLSLSource(target, vertpath, fragpath, userlayout);
			}

			bool32 drawQuadWithID(const glm::vec3& position, const glm::vec2& size, glm::vec4 color, uint32 objectid) {
				return Renderer2D::drawQuadWithID(position, size, color, objectid, m_Context);
			}

			bool32 drawColoredQuad(const glm::mat4& transform, glm::vec4 color) {
				return Renderer2D::drawColoredQuad(transform, color, m_Context);
			}
			bool32 drawColoredQuad(glm::vec3 position, glm::vec2 size, real32 rotation, glm::vec4 color) {
				return Renderer2D::drawColoredQuad(position, size, rotation, color, m_Context);
			}
			bool32 drawColoredQuad(glm::vec3 position, glm::vec2 size, glm::vec4 color) {
				return Renderer2D::drawColoredQuad(position, size, color, m_Context);
			}
			bool32 setView(const glm::mat4& view) {
				return Renderer2D::setView(m_Context, view);
			}
			bool32 setProjection(const glm::mat4& projection) {
				return Renderer2D::setProjection(m_Context, projection);
			}
			bool32 drawLine(glm::vec3 v1, glm::vec3 v2, glm::vec2 thickness) {
				return Renderer2D::drawLine(v1, v2, thickness, m_Context);
			}
			bool32 drawLineStrip(Base::Array<glm::vec2> points, glm::vec2 thickness) {
				return Renderer2D::drawLineStrip(points, thickness, m_Context);
			}	
			bool32 drawLine(glm::vec3 v1, glm::vec3 v2, glm::vec4 color, glm::vec2 thickness) {
				return Renderer2D::drawLine(v1, v2, color, thickness, m_Context);
			}
			bool32 drawLineStrip(Base::Array<glm::vec2> points, glm::vec4 color, glm::vec2 thickness) {
				return Renderer2D::drawLineStrip(points, color, thickness, m_Context);	
			}
			bool32 drawLineStrip(Base::Array<glm::vec2> points, glm::vec4 color, glm::vec2 thickness, real32 depth) {
				return Renderer2D::drawLineStrip(points, color, thickness, depth, m_Context);
			}

			bool32 pushGraphicsPipeline(Platform::GFX::GraphicsPipeline pipeline, Base::Array<Platform::GFX::DescriptorSet> userdescriptors = {nullptr, 0}) {
				return Renderer2D::pushGraphicsPipeline(pipeline, userdescriptors, m_Context);
			}

			bool32 pushTexture(Platform::GFX::Texture texture) {
				return Renderer2D::pushTexture(texture, m_Context);
			}

			static Wrapper create(const Renderer2D::InitInfo& info) {

				Wrapper result;
				result.m_Context = createContext(info);
				return result;
			}

			static Wrapper create(PH::Platform::GFX::GraphicsPipeline pipeline) {

				Renderer2D::InitInfo info{};
				info.currentpipeline;
				info.instancebuffersize = MEGA_BYTE;

				Wrapper result{};
				result.m_Context = Renderer2D::createContext(info);
				return result;
			}

		private:
		
			Renderer2D::Context* m_Context;
		};
	
	}
}
