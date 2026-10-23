
#include "GraphicsPipeline.h"
#include <Engine/Engine.h>

#include "../DefferredRenderer.h"

#include <Base/Datastructures/Stream.h>
#include <Engine/Rendering.h>

#include <Engine/AssetLibrary.h>
#include "Texture.h"

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>

namespace PH::LabCore3D {

	using namespace PH::Platform;


	Platform::GFX::GraphicsPipeline create3DGraphicsPipelineFromBinaries(Platform::GFX::RenderpassDescription renderpass, Base::Array<uint8> vertsource, Base::Array<uint8> fragsource, Base::Array<Platform::GFX::DescriptorSetLayout> userlayouts, bool32 colorblending = false) {

		//create the shader modules
		Platform::GFX::ShaderCreateinfo vertcreate{};
		vertcreate.chachedir = nullptr;
		vertcreate.size = vertsource.count;
		vertcreate.sourcecode = (void*)vertsource.data;
		vertcreate.stage = Platform::GFX::SHADER_STAGE_VERTEX_BIT;
		vertcreate.sourcetype = Platform::GFX::ShaderSourceType::VULKAN_BINARIES;

		Platform::GFX::ShaderCreateinfo fragcreate{};
		fragcreate.chachedir = nullptr;
		fragcreate.size = fragsource.count;
		fragcreate.sourcecode = (void*)fragsource.data;
		fragcreate.stage = Platform::GFX::SHADER_STAGE_FRAGMENT_BIT;
		fragcreate.sourcetype = Platform::GFX::ShaderSourceType::VULKAN_BINARIES;

		Platform::GFX::ShaderCreateinfo infos[2] = { vertcreate, fragcreate };
		Platform::GFX::Shader shaders[2];

		if (!Platform::GFX::createShaders(infos, shaders, 2)) {
			Engine::WARN << "failed to create shaders!\n";
			return PH_GFX_NULL;
		}

		//adding it all together in the pipeline create;
		PH::Platform::GFX::GraphicsPipelineCreateinfo pipelinecreate{};
		pipelinecreate.layouts = userlayouts;
		pipelinecreate.renderpass = renderpass;
		pipelinecreate.shaderstages = { shaders, 2 };
		pipelinecreate.topology = Platform::GFX::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipelinecreate.cullmode = GFX::CULL_MODE_NONE;

		pipelinecreate.vertexbindingdescriptions = { mesh3Dvertexbindingdescription, ARRAY_LENGTH(mesh3Dvertexbindingdescription) };
		pipelinecreate.vertexattributedescriptions = { mesh3Dvertexinputattributes, ARRAY_LENGTH(mesh3Dvertexinputattributes) };
		pipelinecreate.depthtest = true;
		pipelinecreate.blend = colorblending;

		Platform::GFX::GraphicsPipeline pipeline;

		if (!Platform::GFX::createGraphicsPipelines(&pipelinecreate, &pipeline, 1)) {
			Engine::WARN << "failed to create graphicspipeline!\n";
			//TODO: delete shaders because they wont be used
			return PH_GFX_NULL;
		}

		//destroying the layouts array
		return pipeline;
	}

	struct ShaderBinariesHeader {
		sizeptr vertbegin;
		sizeptr vertend;
		sizeptr fragbegin;
		sizeptr fragend;
	};

	struct Binaries {
		Engine::DynamicArray<uint8> vertsrc;
		Engine::DynamicArray<uint8> fragsrc;
	};

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
	bool32 loadBinariesFromGLSL(const char* path, Binaries* b) {

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

		auto vertbin = Engine::Renderer2D::compileGLSLSourceToVulkanBinary(vertsourcestr.getC_Str(), PH::Platform::GFX::SHADER_STAGE_VERTEX_BIT);
		auto fragbin = Engine::Renderer2D::compileGLSLSourceToVulkanBinary(fragsourcestr.getC_Str(), PH::Platform::GFX::SHADER_STAGE_FRAGMENT_BIT);

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
			if (!loadBinariesFromGLSL(srcpath, b)) {
				Engine::WARN << "failed to load graphics pipeline!\n";
				return false;
			}

			if (binpath) {
				writeBinariesToDisk(*b, binpath);
			}
		}
	}

	GFX::GraphicsPipeline createDefferedGeometryPipeline(const Binaries& b, GFX::DescriptorSetLayout userlayout) {

		GFX::DescriptorSetLayout layouts[2] = { DefferedRenderer::scenedescriptorlayout, userlayout };
		GFX::GraphicsPipeline p = create3DGraphicsPipelineFromBinaries(DefferedRenderer::geometryrenderpass, b.vertsrc.getArray(), b.fragsrc.getArray(), { layouts, 2 }, false);

		if (p == PH_GFX_NULL) {
			return {};
		}

		//if neither source nor binaries compile return false
		return p;
	}

	//lighting pipeline should be suitable for the 2d renderer
	GFX::GraphicsPipeline createDefferedLightingPipeline(const Binaries& b) {

		//is of type 2d graphics pipe
		GFX::GraphicsPipeline p = Engine::Renderer2D::createGraphicsPipelineFromBinaries(DefferedRenderer::lightingrenderpass, b.vertsrc.getArray(), b.fragsrc.getArray(), { nullptr, 0 });

		if (p == PH_GFX_NULL) {
			return {};
		}

		//if neither source nor binaries compile return false
		return p;

		//GFX::GraphicsPipeline p = create3DGraphicsPipelineFromBinaries(DefferedRenderer::lightingrenderpass, b.vertsrc.getArray(), b.fragsrc.getArray(), );
	}


	//builds the graphics pipeline based on the type and the binaries
	GFX::GraphicsPipeline buildGraphicsPipeline(PipelineType type, const Binaries& binaries, GFX::DescriptorSetLayout userlayout) {
		switch (type) {
		case LC_PIPELINE_TYPE_3D_DEFFERED_MATERIAL:
			return createDefferedGeometryPipeline(binaries, userlayout);
		case LC_PIPELINE_TYPE_3D_DEFFERED_LIGHTING:
			return createDefferedLightingPipeline(binaries);
		}
	}

	void releaseBinaries(Binaries* b) {
		if (!b->fragsrc.empty()) {
			Engine::DynamicArray<uint8>::destroy(&b->fragsrc);
		}

		if (!b->vertsrc.empty()) {
			Engine::DynamicArray<uint8>::destroy(&b->vertsrc);
		}
	}

	Engine::ArrayList<ShaderResource> reflectOnBinaries(Binaries& b, const char* debugsrcpath = "") {


		Engine::ArrayList<ShaderResource> result = Engine::ArrayList<ShaderResource>::create(1);

		spirv_cross::Compiler compiler((uint32*)b.fragsrc.raw(), b.fragsrc.getCapacity() / 4);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		Engine::INFO << "OpenGLShader::Reflect - " << "fragment shader: " << debugsrcpath << "\n";
		Engine::INFO << "		uniform buffers: " << resources.uniform_buffers.size() << "\n";
		Engine::INFO << "		sampled images: " << resources.sampled_images.size() << "\n";

		Engine::INFO << "Uniform buffers: \n";

		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			int memberCount = bufferType.member_types.size();

			Engine::INFO << resource.name.c_str() << "\n";
			Engine::INFO << "Size = " << bufferSize << "\n";
			Engine::INFO << "Binding = " << binding << "\n";
			Engine::INFO << "Members = " << memberCount << "\n\n";
		}

		Engine::INFO << "\n===================================================\n\n";

		Engine::INFO << "Image samplers:\n";
		for (const auto& resource : resources.sampled_images)
		{
			const auto& type = compiler.get_type(resource.base_type_id);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			//uint32_t arraysize = compiler.get_decoration(resource.id, spv::Decoration)
			uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

			if (set == 1) {
				ShaderResource res;
				res.binding = binding;
				res.set = set;
				res.name = Engine::String::create(resource.name.c_str());
				res.type = GFX::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				res.texture = nullptr;

				result.pushBack(res);
			}

			Engine::INFO << resource.name.c_str() << "\n";
			Engine::INFO << "Set: " << set << "\n";
			Engine::INFO << "binding: " << binding << "\n";
		}

		return result;
	}

	
	GFX::DescriptorSetLayout createDescriptorSetLayoutForResources(Base::Array<ShaderResource> resources, uint32 set) {

		GFX::DescriptorSetLayoutCreateinfo inf;
		Engine::ArrayList<GFX::DescriptorBinding> bindings = Engine::ArrayList<GFX::DescriptorBinding>::create(resources.count);
		for (auto& res : resources) {
			if (res.set == set) {
				GFX::DescriptorBinding b;

				b.type = res.type;
				b.binding = res.binding;
				b.shaderstages = GFX::SHADER_STAGE_FRAGMENT_BIT;

				//should change in the future when using arraylike descriptor
				b.descriptorcount = 1;

				bindings.pushBack(b);
			}
		}

		inf.bindings = bindings.getArray();

		GFX::DescriptorSetLayout r;
		GFX::createDescriptorSetLayouts(&inf, &r, 1);
		bindings.destroy(&bindings);
		return r;
	}

	//updates the descriptor set with the given resources, returns false if not all descriptor bindings have a bound resource
	bool32 updateDescriptorSet(Base::Array<ShaderResource> resources, GFX::DescriptorSet set, uint32 targetset) {
		bool32 result = true;

		for (const auto& res : resources) {
			if (res.set == targetset) {
				if (res.type == GFX::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {

					if (!res.texture) {
						result = false;
						continue;
					}
					
					Texture* tex = (Texture*)res.texture->assetdata;

					GFX::DescriptorImageInfo im;
					im.texture = tex->texture;

					GFX::DescriptorWrite w;

					w.arrayelement = 0;
					w.descriptorcount = 1;
					w.dstbinding = res.binding;
					w.dstset = set;
					w.dynamicwrite = false;
					w.type = res.type;
					w.descriptorinfo = &im;

					GFX::updateDescriptorSets(&w, 1);
				}
			}
		}
		return result;
	}

	GFX::DescriptorSet createResourceDescriptorSet(GFX::DescriptorSetLayout layout) {
		GFX::DescriptorSet r;
		GFX::DescriptorSetCreateinfo inf;
		inf.dynamic = false;
		inf.layout = layout;

		GFX::createDescriptorSets(&inf, &r, 1);
		
		return r;
	}

	LabCore3D::GraphicsPipeline createGraphicsPipeline(const char* sourcepath, const char* binpath, PipelineType type) {

		GraphicsPipeline pl{};
		pl.internalpipeline = PH_GFX_NULL;
		pl.binpath = Engine::String::create(binpath);
		pl.srcpath = Engine::String::create(sourcepath);
		pl.resources = {};
		pl.type = type;
		pl.isvalid = false;

		Binaries b;
		if (!loadBinaries(&b, sourcepath, binpath)) {
			Engine::WARN << "failed to load binarie for pipeline: " << sourcepath;
		}

		auto res = reflectOnBinaries(b, pl.srcpath.getC_Str());
		GFX::DescriptorSetLayout userlayout = createDescriptorSetLayoutForResources(res.getArray(), 1);
		//GFX::DescriptorSet defaultdescriptorset = createResourceDescriptorSet(userlayout);

		GFX::GraphicsPipeline p = buildGraphicsPipeline(type, b, userlayout);
		
		if (p == PH_GFX_NULL) {
			res.destroy(&res);

			Engine::WARN << "failed to create graphics pipeline " << sourcepath << "\n";
			return {};
		}

		releaseBinaries(&b);

		pl.resources = res;
		pl.internalpipeline = p;
		//pl.defaultuserdescriptorset = defaultdescriptorset;
		pl.userlayout = userlayout;

		return pl;

	}

	bool32 revalidateDescriptorSet(GraphicsPipeline* pipe) {

		GFX::DescriptorSet set = createResourceDescriptorSet(pipe->userlayout);
		pipe->isvalid = updateDescriptorSet(pipe->resources.getArray(), set, 1) && (pipe->internalpipeline != PH_GFX_NULL);

		if (pipe->defaultuserdescriptorset != PH_GFX_NULL) {
			GFX::destroyDescriptorSets(&pipe->defaultuserdescriptorset, 1);
		}

		pipe->defaultuserdescriptorset = set;

		return true;
	}

	bool32 reloadGraphicsPipelineFromGLSL(GraphicsPipeline* pipe) {
		Binaries b;
		if (!loadBinariesFromGLSL(pipe->srcpath.getC_Str(), &b)) {
			Engine::WARN << "failed to load binaries\n";
			return false;
		}

		if (!pipe->binpath.isEmpty()) {
			writeBinariesToDisk(b, pipe->binpath.getC_Str());
		}


		auto res = reflectOnBinaries(b, pipe->srcpath.getC_Str());
		GFX::DescriptorSetLayout userlayout = createDescriptorSetLayoutForResources(res.getArray(), 1);
		GFX::DescriptorSet defaultdescriptorset = createResourceDescriptorSet(userlayout);

		GFX::GraphicsPipeline p = buildGraphicsPipeline(pipe->type, b, userlayout);

		if (p == PH_GFX_NULL) {
			res.destroy(&res);

			Engine::WARN << "failed to create graphics pipeline " << pipe->srcpath.getC_Str() << "\n";
			return false;
		}

		releaseBinaries(&b);
		if (pipe->resources.getCapacity() != 0) {
			pipe->resources.destroy(&pipe->resources);
		}

		if (pipe->internalpipeline != PH_GFX_NULL) {
			//add code to destroy old pipe;
		}

		if (pipe->userlayout != PH_GFX_NULL) {
			GFX::destroyDescriptorSetLayouts(&pipe->userlayout, 1);
		}

		if (pipe->defaultuserdescriptorset != PH_GFX_NULL) {
			GFX::destroyDescriptorSets(&pipe->defaultuserdescriptorset, 1);
		}

		pipe->resources = res;
		pipe->internalpipeline = p;
		pipe->defaultuserdescriptorset = defaultdescriptorset;
		pipe->userlayout = userlayout;
		pipe->isvalid = false;

		return true;
	}

	void deserializeGraphicsPipeline(const char* path, GraphicsPipeline* pipe) {

	}

	void serializeGraphicsPipeline(const char* path, GraphicsPipeline* pipe) {

	}

	Engine::AssetDescription createGraphicsPipelineDescription() {
		Engine::AssetDescription d{};
		d.deserialize_ = (Engine::deserializeAsset)deserializeGraphicsPipeline;
		d.serialize_ = (Engine::SerializeAsset)serializeGraphicsPipeline;
		d.localdatasize = sizeof(GraphicsPipeline);
		d.extension = ".lcmat";
		d.id = GraphicsPipeline::getAssetDescriptionID();
		return d;
	}

}