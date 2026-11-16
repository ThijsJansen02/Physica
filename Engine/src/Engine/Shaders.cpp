#include <Engine/shaders.h>
#include <Base/Datastructures/Stream.h>
#include <Platform/platformAPI.h>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>

namespace PH::Engine {

	using namespace PH::Platform;

	Engine::DynamicArray<uint8> compileGLSLSourceToVulkanBinary(const char* source, PH::Platform::GFX::ShaderStageFlags shaderstage) {

		shaderc_shader_kind kind;

		if (shaderstage & GFX::SHADER_STAGE_VERTEX_BIT) {
			kind = shaderc_shader_kind::shaderc_vertex_shader;
		}
		else if (shaderstage & GFX::SHADER_STAGE_FRAGMENT_BIT) {
			kind = shaderc_shader_kind::shaderc_fragment_shader;
		}
		else {
			Engine::WARN << "unsupported shader stage for compilation\n";
			return Engine::DynamicArray<uint8>::create(0);
		}

		//setup the shaderc comiler
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		options.SetAutoMapLocations(false);
		options.SetAutoBindUniforms(false);
		const bool optimize = true;
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		//binary
		std::string sourcecode = (const char*)source;
		shaderc::SpvCompilationResult compiledmodule = compiler.CompileGlslToSpv(sourcecode, kind, "ref");
		if (compiledmodule.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			WARN << "Failed to compile shader: \n\n";
			WARN << "Source Code: \n\n" << (const char*)source << "\n\n";
			WARN << compiledmodule.GetErrorMessage().c_str() << "\n";
			return Engine::DynamicArray<uint8>::create(0);
		}

		sizeptr vertsize = (compiledmodule.cend() - compiledmodule.cbegin()) * sizeof(uint32);

		Engine::DynamicArray<uint8> result = Engine::DynamicArray<uint8>::create(vertsize);
		Base::copyMemory((void*)compiledmodule.cbegin(), (void*)result.raw(), vertsize);
		return result;
	}

	bool32 writeBinariesToDisk(const Binaries& b, const char* binpath) {

		Base::Stream<Engine::Allocator> s = Base::Stream<Engine::Allocator>::create(b.fragsrc.getCapacity() + b.vertsrc.getCapacity() + sizeof(ShaderBinariesHeader));

		ShaderBinariesHeader h;
		h.vertbegin = sizeof(ShaderBinariesHeader);
		h.vertend = h.vertbegin + b.vertsrc.getCapacity();
		h.fragbegin = h.vertend;
		h.fragend = h.fragbegin + b.fragsrc.getCapacity();

		s << h << b.vertsrc.getArray() << b.fragsrc.getArray();

		FileBuffer f;
		f.data = s.raw();
		f.size = s.getSize();

		bool32 r = Platform::writeFile(f, binpath);
		Base::Stream<Engine::Allocator>::destroy(&s);
		return r;
	}



	//reloads the graphics pipeline from glsl source
	bool32 compileBinariesFromGLSL(const char* path, Binaries* b) {

		PH::Platform::FileBuffer f;

		if (!PH::Platform::loadFile(&f, path)) {
			Engine::WARN << "failed to load source for graphics pipeline: " << path;
			return false;
		}

		PH::Base::SubString s((const char*)f.data);


		//should add error check
		s = s.getFrom(">>>VERTEX_SHADER");
		auto vertsource = s.getTill(">>>FRAGMENT_SHADER");
		auto vertsourcestr = Engine::String::create(vertsource);

		s = s.getFrom(">>>FRAGMENT_SHADER");
		auto fragsource = s.getTill(">>>END_OF_FILE");
		auto fragsourcestr = Engine::String::create(fragsource);

		//Engine::Renderer2D::checkCompileBinaries();

		auto vertbin = compileGLSLSourceToVulkanBinary(vertsourcestr.getC_Str(), PH::Platform::GFX::SHADER_STAGE_VERTEX_BIT);
		auto fragbin = compileGLSLSourceToVulkanBinary(fragsourcestr.getC_Str(), PH::Platform::GFX::SHADER_STAGE_FRAGMENT_BIT);

		if (vertbin.empty() || fragbin.empty()) {
			return false;
		}

		b->vertsrc = vertbin;
		b->fragsrc = fragbin;

		return true;
	}


	//loads binarie files from disk
	bool32 loadBinariesFromDisk(const char* path, Binaries* binaries) {

		Platform::FileBuffer f;
		if (!Platform::loadFile(&f, path)) {
			return false;
		}

		uint8* dataptr = (uint8*)f.data;

		ShaderBinariesHeader* h = (ShaderBinariesHeader*)dataptr;

		Binaries b;
		if (h->vertbegin != h->vertend) {
			b.vertsrc = Engine::DynamicArray<uint8>::create(Base::Array<uint8>{ dataptr + h->vertbegin, h->vertend - h->vertbegin });
		}

		if (h->fragbegin != h->fragend) {
			b.fragsrc = Engine::DynamicArray<uint8>::create(Base::Array<uint8>{ dataptr + h->fragbegin, h->fragend - h->fragbegin });
		}

		//unload file
		Platform::unloadFile(&f);
		*binaries = b;

		return true;
	}

	//loads the graphics pipeline from binaries if they exist, and reloads them from source if not
	bool32 loadBinaries(Binaries* b, const char* srcpath, const char* binpath) {

		//load binaries from disk if existent
		if (!loadBinariesFromDisk(binpath, b)) {
			if (!compileBinariesFromGLSL(srcpath, b)) {
				Engine::WARN << "failed to load graphics pipeline!\n";
				return false;
			}

			if (binpath) {
				writeBinariesToDisk(*b, binpath);
			}
		}
		return true;
	}
	
	Platform::GFX::GraphicsPipeline createGraphicsPipelineFromBinaries(
		Platform::GFX::RenderpassDescription renderpass,
		const Binaries& b,
		Base::Array<GFX::DescriptorSetLayout> userlayouts,
		Base::Array<GFX::VertexInputAttributeDescription> vertexinputattributes,
		Base::Array<GFX::VertexInputBindingDescription> vertexinputbindings,
		Base::Array<GFX::PushConstantRange> pushconstants,
		bool32 depthtest,
		bool32 blend
	) {
		//create the shader modules
		Platform::GFX::ShaderCreateinfo vertcreate{};
		vertcreate.chachedir = nullptr;
		vertcreate.size = b.vertsrc.getCapacity();
		vertcreate.sourcecode = (void*)b.vertsrc.raw();
		vertcreate.stage = GFX::SHADER_STAGE_VERTEX_BIT;
		vertcreate.sourcetype = Platform::GFX::ShaderSourceType::VULKAN_BINARIES;

		Platform::GFX::ShaderCreateinfo fragcreate{};
		fragcreate.chachedir = nullptr;
		fragcreate.size = b.fragsrc.getCapacity();
		fragcreate.sourcecode = (void*)b.fragsrc.raw();
		fragcreate.stage = GFX::SHADER_STAGE_FRAGMENT_BIT;
		fragcreate.sourcetype = Platform::GFX::ShaderSourceType::VULKAN_BINARIES;

		Platform::GFX::ShaderCreateinfo infos[2] = { vertcreate, fragcreate };
		Platform::GFX::Shader shaders[2];

		if (!GFX::createShaders(infos, shaders, 2)) {
			Engine::WARN << "failed to create shaders!\n";
			return PH_GFX_NULL;
		}

		//adding it all together in the pipeline create;
		PH::Platform::GFX::GraphicsPipelineCreateinfo pipelinecreate{};
		pipelinecreate.layouts = userlayouts;
		pipelinecreate.renderpass = renderpass;
		pipelinecreate.shaderstages = { shaders, 2 };
		pipelinecreate.topology = Platform::GFX::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipelinecreate.vertexbindingdescriptions = vertexinputbindings;
		pipelinecreate.vertexattributedescriptions = vertexinputattributes;
		pipelinecreate.depthtest = depthtest;
		pipelinecreate.blend = blend;
		pipelinecreate.pushconstantranges = pushconstants;

		Platform::GFX::GraphicsPipeline pipeline;

		if (!Platform::GFX::createGraphicsPipelines(&pipelinecreate, &pipeline, 1)) {
			Engine::WARN << "failed to create graphicspipeline!\n";
			//TODO: delete shaders because they wont be used
			return PH_GFX_NULL;
		}

		return pipeline;
	}

	Platform::GFX::GraphicsPipeline createGraphicsPipelineFromBinaries(
		Platform::GFX::RenderpassDescription renderpass,
		const Binaries& b,
		Base::Array<GFX::DescriptorSetLayout> userlayouts,
		Base::Array<GFX::VertexInputAttributeDescription> vertexinputattributes,
		Base::Array<GFX::VertexInputBindingDescription> vertexinputbindings
	) {
		return createGraphicsPipelineFromBinaries(renderpass, b, userlayouts, vertexinputattributes, vertexinputbindings, { nullptr, 0 }, false, false);
	}

}