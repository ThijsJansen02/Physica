
#include "Rendering.h"
#include "Platform/platformAPI.h"
#include "assets/Mesh.h"

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>

using namespace PH::Platform;


namespace PH::Engine {

	namespace Renderer2D {

		QuadVertex quadvertices[] = {
			{{-0.5f, -0.5f}, {0.0f, 0.0f}},
			{{ 0.5f, -0.5f}, {1.0f, 0.0f}},
			{{ 0.5f,  0.5f}, {1.0f, 1.0f}},
			{{-0.5f,  0.5f}, {0.0f, 1.0f}}
		};

		uint32 quadindices[] = {
			0, 1, 2,
			0, 2, 3
		};

		struct Buffers {
			PH::Platform::GFX::Buffer instance;
			PH::Platform::GFX::Buffer vertex;
			PH::Platform::GFX::Buffer index;
		};

		struct SceneBuffer {
			glm::mat4 projection;
			glm::mat4 view;
		};

		struct Context {

			Buffers buffers;
			PH::Platform::GFX::Texture nulltexture;

			PH::Platform::GFX::GraphicsPipeline currentpipeline;

			ArrayList<ColoredQuadInstance> quads;

			//holds a list of the descriptors that the user has set for the current draw call, these will be bound before drawing and cleared after drawing
			ArrayList<PH::Platform::GFX::DescriptorSet> userdescriptors;

			//holds the texture array and scene uniform buffer for every draw call will be incremented and bound to the pipeline before drawing
			ArrayList<PH::Platform::GFX::DescriptorSet> globaldescriptors;
			
			//pointer to the current position in the global descriptor set array, will be incremented for every draw call
			sizeptr globaldescriptorpointer;

			//holds the uniform buffer for the current scene
			PH::Platform::GFX::Buffer scenebuffer;

			//holds the size of the instance buffer, will be used to check if the buffer needs to be resized before drawing
			sizeptr instancebuffersize;
			
			//holds the number of instances that have been drawn before this draw call
			sizeptr drawninstances;

			//local copy of the scene buffer data, will be updated by the user and copied to the GPU buffer before drawing
			SceneBuffer localscenebuffer;

			//holds the textures that have been used in the current draw call
			ArrayList<GFX::Texture> textures;
		};

		bool32 drawColoredQuad(glm::vec3 position, glm::vec2 size, glm::vec4 color, Renderer2D::Context* context) {
			ColoredQuadInstance instance{};
			instance.color = color;
			instance.position = position;
			instance.scalerot = glm::mat2{ {size.x, 0.0f}, {0.0f, size.y} };
			
			context->quads.pushBack(instance);
			return true;
		}

		bool32 drawQuadWithID(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, uint32 objectid, Renderer2D::Context* context) {
			ColoredQuadInstance instance{};
			instance.color = color;
			instance.position = position;
			instance.scalerot = glm::mat2{ {size.x, 0.0f}, {0.0f, size.y} };
			instance.textureindex = objectid;

			context->quads.pushBack(instance);
			return true;
		}

		bool32 drawColoredQuad(glm::vec3 position, glm::vec2 size, real32 rotation, glm::vec4 color, Renderer2D::Context* context) {
			real32 cosp = cos(rotation);
			real32 sinp = sin(rotation);
			
			ColoredQuadInstance instance{};
			instance.color = color;
			instance.position = position;
			instance.scalerot = glm::mat2{ {size.x, 0.0f}, {0.0f, size.y} } * glm::mat2({cosp, sinp}, {-sinp, cosp});

			context->quads.pushBack(instance);
			return true;
		}

		bool32 drawColoredQuad(const glm::mat4& transform, glm::vec4 color, Renderer2D::Context* context) {
			
			ColoredQuadInstance instance{};
			instance.scalerot = {transform[0][0],transform[0][1],transform[1][0],transform[1][1] };
			instance.position = transform[3];
			instance.color = color;
			instance.textureindex = 0;
			
			context->quads.push(instance);
			return true;
		}

		bool32 drawTexturedQuad(glm::vec3 position, glm::vec2 size, GFX::Texture texture, Renderer2D::Context* context) {
			
			ColoredQuadInstance instance{};
			instance.color = { 1.0f, 1.0f, 1.0f, 1.0f };
			instance.position = position;
			instance.scalerot = glm::mat2{ {size.x, 0.0f}, {0.0f, size.y} };
			if (context->textures[context->textures.getCount() - 1] != texture) {
				context->textures.pushBack(texture);
			}

			instance.textureindex = context->textures.getCount() - 1;
			context->quads.pushBack(instance);
			return true;
		}

		bool32 drawTexturedQuad(const glm::mat4& transform, GFX::Texture texture, Renderer2D::Context* context) {

			ColoredQuadInstance instance{};
			instance.scalerot = { transform[0][0],transform[0][1],transform[1][0],transform[1][1] };
			instance.position = transform[3];
			instance.color = {1.0f, 1.0f, 1.0f, 1.0f};

			if (context->textures[context->textures.getCount() - 1] != texture) {
				context->textures.pushBack(texture);
			}
			instance.textureindex = context->textures.getCount() - 1;
			
			context->quads.push(instance);
			return true;
		}

		bool32 drawLine(glm::vec3 v1, glm::vec3 v2, glm::vec2 thickness, Renderer2D::Context* context) {
			return drawLine(v1, v2, { 1.0f, 1.0f, 1.0f, 1.0f }, thickness, context);
		}

		bool32 drawLineStrip(Base::Array<glm::vec2> points, glm::vec2 thickness, Renderer2D::Context* context) {
			return drawLineStrip(points, { 1.0f, 1.0f, 1.0f, 1.0f }, thickness, context);
		}

		real32 blend(real32 a, real32 b, real32 blendfactor) {
			return (1.0f - blendfactor) * a + blendfactor * b;
		}

		bool32 drawLine(glm::vec3 v1, glm::vec3 v2, glm::vec4 color, glm::vec2 thickness, Renderer2D::Context* context) {

			glm::vec3 midpoint = (v1 + v2) * 0.5f;

			glm::vec3 up = { 0.0f, 0.0f, 1.0f };
			glm::vec3 tangent = glm::normalize(v2 - v1);
			glm::vec3 bitangent = glm::cross(tangent, up);

			real32 length = glm::length(v2 - v1);
			real32 bithickness = blend(thickness.y, thickness.x, glm::abs(bitangent.x));
			real32 tanthickness = blend(thickness.y, thickness.x, glm::abs(tangent.x));

			glm::mat3 rotscale = glm::mat3(tangent * (length + tanthickness), bitangent * bithickness, up);

			glm::mat4 transform = glm::mat4(rotscale);
			transform[3] = { midpoint, 1.0f };

			return drawColoredQuad(transform, color, context);
		}

		bool32 drawLineStrip(Base::Array<glm::vec2> points, glm::vec4 color, glm::vec2 thickness, real32 depth, Renderer2D::Context* context) {
			for (uint32 i = 1; i < points.count; i++) {
				drawLine({ points[i - 1], depth }, { points[i], depth }, color, thickness, context);
			}
			return true;
		}

		bool32 drawLineStrip(Base::Array<glm::vec2> points, glm::vec4 color, glm::vec2 thickness, Renderer2D::Context* context) {
			return drawLineStrip(points, color, thickness, 0.0f, context);
		}

		bool32 pushTexture(GFX::Texture texture, Renderer2D::Context* context) {
			if (context->textures.getCount() < Renderer2D::MAX_TEXTURES) {
				context->textures.pushBack(texture);
				return true;
			}
			else {
				Engine::WARN << "Max texture count reached for this draw call, texture will not be drawn\n";
				return false;
			}
		}

		Platform::GFX::DescriptorSetLayout globalDescriptorSetLayout = PH_GFX_NULL;

		Platform::GFX::DescriptorSetLayout createGlobalDescriptorSetLayout() {

			GFX::DescriptorBinding texturebinding;
			texturebinding.binding = TEXTURESAMPLER_BINDING;
			texturebinding.descriptorcount = MAX_TEXTURES;
			texturebinding.shaderstages = GFX::SHADER_STAGE_FRAGMENT_BIT;
			texturebinding.type = GFX::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			
			GFX::DescriptorBinding transformbinding;
			transformbinding.binding = SCENE_UBO_BINDING;
			transformbinding.descriptorcount = 1;
			transformbinding.shaderstages = GFX::SHADER_STAGE_VERTEX_BIT;
			transformbinding.type = GFX::DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				
			GFX::DescriptorBinding bindings[2] = { texturebinding, transformbinding };

			GFX::DescriptorSetLayoutCreateinfo createinfo{};
			createinfo.bindings = { bindings, 2 };
			GFX::DescriptorSetLayout layout;

			GFX::createDescriptorSetLayouts(&createinfo, &layout, 1);
			return layout;
		}

		Platform::GFX::Buffer createScenebuffer() {

			SceneBuffer localbuffer = { glm::mat4(1.0f), glm::mat4(1.0f) };

			Platform::GFX::BufferCreateinfo info{};
			info.bufferusage = Platform::GFX::BufferUsageFlagBits::BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			info.data = &localbuffer;
			info.dynamic = true;
			info.memoryproperties = Platform::GFX::MEMORY_PROPERTY_HOST_VISIBLE_BIT | Platform::GFX::MEMORY_PROPERTY_HOST_COHERENT_BIT;
			info.size = sizeof(localbuffer);
			
			Platform::GFX::Buffer result;
			if (!Platform::GFX::createBuffers(&info, &result, 1)) {
				Engine::WARN << "Failed to create buffer\n";
			}

			return result;
		}

		//creates a 1x1 texture with a white color to be set when no texture is specified
		Platform::GFX::Texture createNullTexture() {

			uint32 col = 0xFFFFFFFF;

			GFX::TextureCreateInfo createinfo;
			createinfo.data = &col;
			createinfo.format = GFX::FORMAT_R8G8B8A8_SRGB;
			createinfo.width = 1;
			createinfo.usage = GFX::IMAGE_USAGE_SAMPLED_BIT;
			createinfo.height = 1;

			GFX::Texture tex;
			GFX::createTextures(&createinfo, &tex, 1);
			return tex;
		}

		Platform::GFX::GraphicsPipeline createGraphicsPipelineFromBinaries(const Engine::Display* target, Base::Array<uint8> vertsource, Base::Array<uint8> fragsource, Base::Array<GFX::DescriptorSetLayout> userlayouts) {

			//create the shader modules
			Platform::GFX::ShaderCreateinfo vertcreate{};
			vertcreate.chachedir = nullptr;
			vertcreate.size = vertsource.count;
			vertcreate.sourcecode = (void*)vertsource.data;
			vertcreate.stage = GFX::SHADER_STAGE_VERTEX_BIT;
			vertcreate.sourcetype = Platform::GFX::ShaderSourceType::VULKAN_BINARIES;

			Platform::GFX::ShaderCreateinfo fragcreate{};
			fragcreate.chachedir = nullptr;
			fragcreate.size = fragsource.count;
			fragcreate.sourcecode = (void*)fragsource.data;
			fragcreate.stage = GFX::SHADER_STAGE_FRAGMENT_BIT;
			fragcreate.sourcetype = Platform::GFX::ShaderSourceType::VULKAN_BINARIES;

			Platform::GFX::ShaderCreateinfo infos[2] = { vertcreate, fragcreate };
			Platform::GFX::Shader shaders[2];

			if (!GFX::createShaders(infos, shaders, 2)) {
				Engine::WARN << "failed to create shaders!\n";
			}

			if (globalDescriptorSetLayout == PH_GFX_NULL) {
				globalDescriptorSetLayout = createGlobalDescriptorSetLayout();
			}

			//adding the scene layout and the renderer layout together
			auto layouts = Engine::ArrayList<GFX::DescriptorSetLayout>::create(1);
			layouts.pushBack(globalDescriptorSetLayout);

			//TODO add ability to have global descriptors
			for (auto layout : userlayouts) {
				layouts.pushBack(layout);
			}


			//adding it all together in the pipeline create;
			PH::Platform::GFX::GraphicsPipelineCreateinfo pipelinecreate{};
			pipelinecreate.layouts = layouts.getArray();
			pipelinecreate.renderpass = target->renderpass;
			pipelinecreate.shaderstages = { shaders, 2 };
			pipelinecreate.topology = Platform::GFX::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			pipelinecreate.vertexbindingdescriptions = { quadvertexbindingdescription, ARRAY_LENGTH(quadvertexbindingdescription) };
			pipelinecreate.vertexattributedescriptions = { quadvertexinputattributes, ARRAY_LENGTH(quadvertexinputattributes) };
			pipelinecreate.depthtest = false;

			Platform::GFX::GraphicsPipeline pipeline;

			if (!Platform::GFX::createGraphicsPipelines(&pipelinecreate, &pipeline, 1)) {
 				Engine::WARN << "failed to create graphicspipeline!\n";
			}

			//destroying the layouts array
			layouts.release();

			return pipeline;
		}

		Platform::GFX::GraphicsPipeline createGraphicsPipelineFromGLSLSource(const Engine::Display* target, Base::Array<uint8> vertsource, Base::Array<uint8> fragsource, Base::Array<Platform::GFX::DescriptorSetLayout> userlayouts) {

			//setup the shaderc comiler
			shaderc::Compiler compiler;
			shaderc::CompileOptions options;
			options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
			options.SetAutoMapLocations(false);
			options.SetAutoBindUniforms(false);
			const bool optimize = true;
			if (optimize)
				options.SetOptimizationLevel(shaderc_optimization_level_performance);

			//compile vertex shader
			shaderc_shader_kind kind = shaderc_shader_kind::shaderc_vertex_shader;
			std::string sourcecode = (const char*)vertsource.data;
			shaderc::SpvCompilationResult compiledvertmodule = compiler.CompileGlslToSpv(sourcecode, kind, "ref");
			if (compiledvertmodule.GetCompilationStatus() != shaderc_compilation_status_success)
			{
				WARN << "Failed to compile shaderstage: Vertex Shader\n\n";
				WARN << "Source Code: \n\n" << (const char*)vertsource.data << "\n\n";
				WARN << compiledvertmodule.GetErrorMessage().c_str() << "\n";
				return false;
			}

			sizeptr vertsize = (compiledvertmodule.cend() - compiledvertmodule.cbegin()) * sizeof(uint32);

			kind = shaderc_shader_kind::shaderc_fragment_shader;
			sourcecode = (const char*)fragsource.data;

			shaderc::SpvCompilationResult compiledfragmodule = compiler.CompileGlslToSpv(sourcecode, kind, "ref");
			if (compiledfragmodule.GetCompilationStatus() != shaderc_compilation_status_success)
			{
				WARN << "Failed to compile shaderstage: fragmentshader Shader\n\n";
				WARN << "Source Code: \n\n" << (const char*)fragsource.data << "\n\n";
				WARN << compiledfragmodule.GetErrorMessage().c_str() << "\n";
				return false;
			}

			sizeptr fragsize = (compiledfragmodule.cend() - compiledfragmodule.cbegin()) * sizeof(uint32);

			return createGraphicsPipelineFromBinaries(target, { (uint8*)compiledvertmodule.cbegin(), vertsize }, { (uint8*)compiledfragmodule.cbegin(), fragsize }, userlayouts);
		}

		Platform::GFX::GraphicsPipeline createGraphicsPipelineFromGLSLSource(const Engine::Display* target, const char* vertpath, const char* fragpath, Base::Array<GFX::DescriptorSetLayout> userlayouts) {

			Platform::GFX::GraphicsPipelineCreateinfo info{};

			Platform::FileBuffer vertsource;
			Platform::FileBuffer fragsource;
			if (!Platform::loadFile(&vertsource, vertpath)) {
				Engine::ERR << "failed to load vertsource!\n";
			}

			if (!Platform::loadFile(&fragsource, fragpath)) {
				Engine::ERR << "failed to load fragsource!\n";
			}

			auto result = createGraphicsPipelineFromGLSLSource(target, { (uint8*)vertsource.data, vertsource.size }, { (uint8*)fragsource.data, fragsource.size }, userlayouts);

			Platform::unloadFile(&vertsource);
			Platform::unloadFile(&fragsource);

			return result;
		}

		bool32 setView(Context* context, const glm::mat4& view) {
			context->localscenebuffer.view = view;
			return true;
		}
		bool32 setProjection(Context* context, const glm::mat4& projection) {
			context->localscenebuffer.projection = projection;
			return true;
		}

		Platform::GFX::DescriptorSet createGlobalDescriptorSet(Context* context, sizeptr scenebufferoffset) {

			GFX::DescriptorSetCreateinfo createinfo{};
			createinfo.dynamic = true;
			createinfo.layout = globalDescriptorSetLayout;

			GFX::DescriptorSet set;
			GFX::createDescriptorSets(&createinfo, &set, 1);
				
			//making 1 array of textures
			GFX::DescriptorImageInfo infos[MAX_TEXTURES];
			for (uint32 i = 0; i < MAX_TEXTURES; i++) {
				infos[i].texture = context->nulltexture;
			}

			//writing the null texture into the set
			GFX::DescriptorWrite texturewrite;
			texturewrite.dynamicwrite = false;
			texturewrite.arrayelement = 0;
			texturewrite.dstset = set;
			texturewrite.type = GFX::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			texturewrite.descriptorcount = MAX_TEXTURES;
			texturewrite.dstbinding = TEXTURESAMPLER_BINDING;
			texturewrite.descriptorinfo = infos;

			GFX::DescriptorBufferInfo bufferinfo;
			bufferinfo.buffer = context->scenebuffer;
			bufferinfo.offset = scenebufferoffset;
			bufferinfo.range = sizeof(SceneBuffer);

			//note not dynamic write because this write should set the buffer for every frame
			GFX::DescriptorWrite bufferwrite;
			bufferwrite.arrayelement = 0;
			bufferwrite.descriptorcount = 1;
			bufferwrite.dstset = set;
			bufferwrite.descriptorinfo = &bufferinfo;
			bufferwrite.dynamicwrite = false;
			bufferwrite.type = GFX::DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			bufferwrite.dstbinding = SCENE_UBO_BINDING;

			GFX::DescriptorWrite writes[2] = { texturewrite, bufferwrite };

			GFX::updateDescriptorSets(writes, 2);

			return set;
		}

		Buffers createBuffers(const InitInfo& initinfo) {

			//the instance buffer
			Platform::GFX::BufferCreateinfo instancecreate{};
			instancecreate.bufferusage = Platform::GFX::BUFFER_USAGE_VERTEX_BUFFER_BIT;
			instancecreate.dynamic = true;
			instancecreate.memoryproperties = Platform::GFX::MEMORY_PROPERTY_HOST_VISIBLE_BIT | Platform::GFX::MEMORY_PROPERTY_HOST_COHERENT_BIT;
			instancecreate.size = initinfo.instancebuffersize;
			instancecreate.data = nullptr;

			//the vertex buffer
			Platform::GFX::BufferCreateinfo vertexbuffercreate{};
			vertexbuffercreate.bufferusage = Platform::GFX::BUFFER_USAGE_VERTEX_BUFFER_BIT;
			vertexbuffercreate.dynamic = false;
			vertexbuffercreate.memoryproperties = Platform::GFX::MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			vertexbuffercreate.size = sizeof(quadvertices);
			vertexbuffercreate.data = quadvertices;

			//the index buffer
			Platform::GFX::BufferCreateinfo indexbuffercreate{};
			indexbuffercreate.bufferusage = Platform::GFX::BUFFER_USAGE_INDEX_BUFFER_BIT;
			indexbuffercreate.dynamic = false;
			indexbuffercreate.memoryproperties = Platform::GFX::MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			indexbuffercreate.size = sizeof(quadindices);
			indexbuffercreate.data = quadindices;

			//adding the buffers in an array
			Platform::GFX::BufferCreateinfo buffercreateinfos[3] = {
				instancecreate, vertexbuffercreate, indexbuffercreate
			};

			//createing the buffers
			Platform::GFX::Buffer buffers[3];
			Platform::GFX::createBuffers(buffercreateinfos, buffers, 3);

			//distributing the results
			Buffers result;
			result.instance = buffers[0];
			result.vertex = buffers[1];
			result.index = buffers[2];

			return result;
		}

		Context* createContext(const InitInfo& initinfo) {

			Context result{};
			if (globalDescriptorSetLayout == PH_GFX_NULL) {
				globalDescriptorSetLayout = createGlobalDescriptorSetLayout();
			}

			result.nulltexture = createNullTexture();
			result.scenebuffer = createScenebuffer();
			result.globaldescriptors = Engine::ArrayList<GFX::DescriptorSet>::create(1);
			result.currentpipeline = initinfo.currentpipeline;
			result.quads = Engine::ArrayList<ColoredQuadInstance>::create(1);
			result.buffers = createBuffers(initinfo);
			result.instancebuffersize = initinfo.instancebuffersize;
			result.userdescriptors = Engine::ArrayList<GFX::DescriptorSet>::create(1);

			result.textures = ArrayList<GFX::Texture>::create(MAX_TEXTURES);

			Context* resultptr = (Context*)Engine::Allocator::alloc(sizeof(Renderer2D::Context));
			*resultptr = result;
			return resultptr;
		}

		bool32 updateDescriptorSet(Context* context) {
			GFX::DescriptorWrite write{};
			write.dynamicwrite = true;

			GFX::DescriptorImageInfo infos[MAX_TEXTURES];
			for (uint32 i = 0; i < context->textures.getCount(); i++) {
				infos[i].texture = context->textures[i];
			}

			write.arrayelement = 0;
			write.descriptorcount = context->textures.getCount();
			write.dstbinding = TEXTURESAMPLER_BINDING;
			write.dstset = context->globaldescriptors[context->globaldescriptorpointer];
			write.type = GFX::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.descriptorinfo = infos;

			GFX::updateDescriptorSets(&write, 1);

			return true;
		}

		bool32 updateSceneBuffer(Context* context) {
			
			SceneBuffer* gpubuffer;
			PH::Platform::GFX::mapBuffer((void**) & gpubuffer, context->scenebuffer);
			*gpubuffer = context->localscenebuffer;

			PH::Platform::GFX::unmapBuffer(context->scenebuffer);

			return true;
		}

		bool32 begin(Context* context) {

			if (context->globaldescriptors.getCount() <= 0) {
				context->globaldescriptors.pushBack(createGlobalDescriptorSet(context, 0));
			}

			context->textures.clear();
			context->textures.pushBack(context->nulltexture);
			context->quads.clear();

			context->drawninstances = 0;
			context->globaldescriptorpointer = 0;
			return true;
		}

		bool32 end(Context* context) {
			return true;
		}

		bool32 flush(Context* context, Base::Array<Platform::GFX::DescriptorSet> additionaldescriptors) {

			Engine::ArenaScope s;

			//calculate the offeset in the buffer because of the instances that have already been drawn in this frame, this allows for multiple flushes in one frame without overwriting the instance data of the previous flushes
			sizeptr instanceoffset = sizeof(ColoredQuadInstance) * context->drawninstances;

			//copy the instancedata into the gpu buffer
			void* instancememory = nullptr;
			PH::Platform::GFX::mapBuffer(&instancememory, context->buffers.instance);

			if (instanceoffset + context->quads.getCount() * sizeof(ColoredQuadInstance) > context->instancebuffersize) {
				WARN << "there are more quads submittend than the size of the instance buffer allows the engine to draw\n";
				return false;
			}

			//might not be necessary to copy the data every frame if the buffer is persistently mapped but for now this is the easiest way to do it
			Base::copyMemory(context->quads.raw(), (uint8*)instancememory + instanceoffset, context->quads.getCount() * sizeof(ColoredQuadInstance));

			updateSceneBuffer(context);
			updateDescriptorSet(context);

			PH::Platform::GFX::bindGraphicsPipeline(context->currentpipeline);

			auto descriptorsets = Base::ArrayList<GFX::DescriptorSet, Engine::ArenaAllocator>::create(1 + additionaldescriptors.count + context->userdescriptors.getCount());
			descriptorsets.pushBack(context->globaldescriptors[context->globaldescriptorpointer]);
			descriptorsets.pushBack(context->userdescriptors.getArray());
			descriptorsets.pushBack(additionaldescriptors);
				
			PH::Platform::GFX::bindDescriptorSets(descriptorsets.raw(), 0, descriptorsets.getCount());

			PH::Platform::GFX::bindIndexBuffer(context->buffers.index, 0);
			PH::Platform::GFX::Buffer buffers[2] = { context->buffers.vertex, context->buffers.instance };
			sizeptr offsets[] = { 0, instanceoffset};
			PH::Platform::GFX::bindVertexBuffers(0, 2, buffers, offsets);
			PH::Platform::GFX::drawIndexed(6, context->quads.getCount(), 0, 0);
			
			context->drawninstances += context->quads.getCount();	

			//reset all the data for the next drawcall
			context->textures.clear();
			context->textures.pushBack(context->nulltexture);
			context->quads.clear();

			descriptorsets.release();
			return true;
		}

		void moveToNextDesciptorSet(Context* context) {
			context->globaldescriptorpointer++;
			if (context->globaldescriptors.getCount() <= context->globaldescriptorpointer) {
				context->globaldescriptors.pushBack(createGlobalDescriptorSet(context, 0));
			}
		}

		bool32 pushGraphicsPipeline(Platform::GFX::GraphicsPipeline pipeline, Base::Array<Platform::GFX::DescriptorSet> userdescriptors, Renderer2D::Context* context) {
			if (!context->quads.empty()) {
				flush(context, {nullptr, 0});
				moveToNextDesciptorSet(context);
			}
			else {
				if (context->globaldescriptors.getCount() <= context->globaldescriptorpointer) {
					context->globaldescriptors.pushBack(createGlobalDescriptorSet(context, 0));
				}
			}
			context->currentpipeline = pipeline;
			context->userdescriptors.clear();
			context->userdescriptors.pushBack(userdescriptors);

			return true;
		}
	}

	namespace Renderer3D {

		Engine::Assets::Mesh cube;
		Engine::Assets::Mesh arrowshaft;
		Engine::Assets::Mesh arrowpoint;

		struct Instance {
			Engine::Assets::MaterialInstance* material;
			Engine::Assets::Mesh* mesh;
			uint32 submesh;
			glm::vec4 color;
			uint32 objectid;
			glm::mat3x4 transform;
		};

		struct Context {
			ArrayList<Instance> instances;

			sizeptr instancebufferoffset;
			PH::Platform::GFX::Buffer instancebuffer;

			Engine::Assets::MaterialInstance* colorshader;
			Engine::Assets::MaterialInstance* flatcolorshader;

			RenderStats stats;
		};

		real32 cubeVertices[] = {
			// positions          // normals           // texture coords
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
			 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

			-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
			-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

			-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

			 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

			-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
			 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

			-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
			 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
			-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
		};

		uint32 indices[]{
			0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35
		};

		Engine::Assets::Mesh createCube() {
			Engine::Assets::Mesh result = Assets::createMeshFromData({ (Assets::Vertex*)cubeVertices, 36 }, { indices, 36 }, false);
			return result;
		}


		Context* createContext(const InitInfo& init) {

			Context result{};

			//the instance buffer
			Platform::GFX::BufferCreateinfo instancecreate{};
			instancecreate.bufferusage = Platform::GFX::BUFFER_USAGE_VERTEX_BUFFER_BIT;
			instancecreate.dynamic = true;
			instancecreate.memoryproperties = Platform::GFX::MEMORY_PROPERTY_HOST_VISIBLE_BIT | Platform::GFX::MEMORY_PROPERTY_HOST_COHERENT_BIT;
			instancecreate.size = MEGA_BYTE;
			instancecreate.data = nullptr;

			result.colorshader = init.colorshader;
			result.flatcolorshader = init.flatcolorshader;

			Platform::GFX::createBuffers(&instancecreate, &result.instancebuffer, 1);

			result.instances = ArrayList<Instance>::create(1);

			Context* ptr = (Context*)Engine::Allocator::alloc(sizeof(Renderer3D::Context));
			*ptr = result;

			return ptr;
		}

		void begin(Renderer3D::Context* context) {
			context->instancebufferoffset = 0;
			context->instances.clear();
		}
		
		void drawMeshSafe(
			glm::mat4 transform,
			Assets::Mesh* mesh,
			Base::Array<Engine::Assets::MaterialInstance*> materials,
			Renderer3D::Context* context
		) {
			if (!(mesh->Ready())) {
				return;
			}

			uint32 index = 0;
			for (auto submesh : mesh->submeshes) {

				if (index >= materials.count) {
					break;
				}
				Instance inst{};
				inst.material = materials[index];
				if (!inst.material || !inst.material->Ready()) {
					continue;
				}

				inst.mesh = mesh;
				inst.submesh = index;
				inst.color = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f);
				inst.transform = glm::transpose(transform);

				index++;

				context->instances.pushBack(inst);
			}


		}

		void drawMesh(
			glm::mat4 transform,
			Assets::Mesh* mesh, 
			Base::Array<Engine::Assets::MaterialInstance*> materials,
			glm::vec4 color,
			Renderer3D::Context* context
		) {
			uint32 index = 0;
			for (const auto& submesh : mesh->submeshes) {

				Instance inst{};
				inst.material = materials[index];

				inst.mesh = mesh;
				inst.submesh = index;
				inst.color = color;
				inst.transform = glm::transpose(transform);

				index++;

				context->instances.pushBack(inst);
			}
		}

		void drawMesh(
			glm::mat4 transform,
			Assets::Mesh* mesh,
			Base::Array<Engine::Assets::MaterialInstance*> materials,
			Renderer3D::Context* context
		) {
			drawMesh(transform, mesh, materials, glm::vec4(1.0f), context);
		}

		void drawCube(glm::mat4 transform, Engine::Assets::MaterialInstance* material, Renderer3D::Context* context) {

			Instance inst{};
			inst.material = material;
			inst.mesh = &cube;
			inst.submesh = 0;
			inst.transform = glm::transpose(transform);
			context->instances.pushBack(inst);
		}

		void drawColoredCube(glm::mat4 transform, glm::vec4 color, Renderer3D::Context* context) {

			//PH_DEBUG_ASSERT(context->colorshader, "no color shader set!")
			if (!context->colorshader) {
				return;
			}

			if (!context->colorshader->Ready()) {
				return;
			}

			drawMesh(transform, &cube, { &context->colorshader, 1 }, color, context);

		}

		void drawFlatColoredCube(glm::mat4 transform, glm::vec4 color, Renderer3D::Context* context) {
			//PH_DEBUG_ASSERT(context->colorshader, "no color shader set!")
			if (!context->flatcolorshader) {
				return;
			}

			if (!context->flatcolorshader->Ready()) {
				return;
			}
	
			drawMesh(transform, &cube, { &context->flatcolorshader, 1 }, color, context);
		}

		void end(Renderer3D::Context* context) {

		}

		void flush(Renderer3D::Context* context, GFX::DescriptorSet scenedescriptorset) {


			if (context->instances.getCount() <= 0) {
				return;
			}


			sizeptr offsets[2] = { 0, context->instancebufferoffset };
			GFX::Buffer buffers[2];

			DrawInstance* li; 
			PH::Platform::GFX::mapBuffer((void**)&li, context->instancebuffer);

			li = (DrawInstance*)((uint8*)li + offsets[1]);

			Platform::GFX::GraphicsPipeline currentpipeline = context->instances[0].material->parent->pipeline;
			PH::Platform::GFX::bindGraphicsPipeline(currentpipeline);
			PH::Platform::GFX::bindDescriptorSets(&scenedescriptorset, 0, 1);

			Platform::GFX::DescriptorSet currentmaterialdescriptorset = context->instances[0].material->descriptorset;
			if (currentmaterialdescriptorset != PH_GFX_NULL) {
				PH::Platform::GFX::bindDescriptorSets(&currentmaterialdescriptorset, 1, 1);
			}

			Assets::Mesh* currentmesh = context->instances[0].mesh;
			buffers[0] = currentmesh->vertexbuffer;
			buffers[1] = context->instancebuffer;
			PH::Platform::GFX::bindVertexBuffers(0, 1, &currentmesh->vertexbuffer, &offsets[0]);
			PH::Platform::GFX::bindIndexBuffer(currentmesh->indexbuffer, 0);

			for (auto instance : context->instances) {
				//instance.transform = instance.transform;

				li->transform[0] = instance.transform[0];
				li->transform[1] = instance.transform[1];
				li->transform[2] = instance.transform[2];

				li->color = instance.color;
				li->objectid = instance.objectid;

				if (instance.material->parent->pipeline != currentpipeline) {
					currentpipeline = instance.material->parent->pipeline;
					PH::Platform::GFX::bindGraphicsPipeline(currentpipeline);
					PH::Platform::GFX::bindDescriptorSets(&scenedescriptorset, 0, 1);

					//this causes the descriptorsets to rebind
					currentmaterialdescriptorset = PH_GFX_NULL;
					currentmesh = nullptr;
				}

				if (currentmaterialdescriptorset != instance.material->descriptorset) {
					currentmaterialdescriptorset = instance.material->descriptorset;
					if (currentmaterialdescriptorset != PH_GFX_NULL) {
						PH::Platform::GFX::bindDescriptorSets(&currentmaterialdescriptorset, 1, 1);
					}
				}

				if (currentmesh != instance.mesh) {
					currentmesh = instance.mesh;

					PH::Platform::GFX::bindVertexBuffers(0, 1, &currentmesh->vertexbuffer, &offsets[0]);
					PH::Platform::GFX::bindIndexBuffer(currentmesh->indexbuffer, 0);
				}

				buffers[0] = instance.mesh->vertexbuffer;
				buffers[1] = context->instancebuffer;
				Engine::Assets::Mesh::SubMesh submesh = instance.mesh->submeshes[instance.submesh];

				context->stats.drawcalls += 1;
				context->stats.trianglecount += submesh.count / 3;

				PH::Platform::GFX::bindVertexBuffers(1, 1, &context->instancebuffer, &offsets[1]);
				PH::Platform::GFX::drawIndexed(submesh.count, 1, submesh.offset, 0);

				li += 1;
				offsets[1] += sizeof(DrawInstance);
			}



			context->instancebufferoffset = offsets[1];
			context->instances.clear();
		}

		void resetStats(Renderer3D::Context* context) {
			context->stats.drawcalls = 0;
			context->stats.trianglecount = 0;
		}

		RenderStats getStats(Renderer3D::Context* context) {
			return context->stats;
		}

	}
}