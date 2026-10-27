
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
	GFX::GraphicsPipeline createDefferedLightingPipeline(const Binaries& b, GFX::DescriptorSetLayout userlayout) {

		//the lighting pipe is made out of a renderer2d graphicspipe with a custom renderpass
		GFX::GraphicsPipeline p = Engine::Renderer2D::createGraphicsPipelineFromBinaries(DefferedRenderer::lightingrenderpass, b.vertsrc.getArray(), b.fragsrc.getArray(), { &userlayout, 1 });

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
			return createDefferedLightingPipeline(binaries, userlayout);
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

	Engine::ArrayList<ShaderResource> reflectOnBinaries(const Binaries& b, uint32 targetset, const char* debugsrcpath = "") {


		Engine::ArrayList<ShaderResource> result = Engine::ArrayList<ShaderResource>::create(1);

		spirv_cross::Compiler compiler((uint32*)b.fragsrc.raw(), b.fragsrc.getCapacity() / 4);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		Engine::INFO << "VulkanShader::Reflect - " << "fragment shader: " << debugsrcpath << "\n";
		Engine::INFO << "		uniform buffers: " << resources.uniform_buffers.size() << "\n";
		Engine::INFO << "		sampled images: " << resources.sampled_images.size() << "\n";

		Engine::INFO << "Uniform buffers: \n";

		uint32 userbufferoffset = 0;
		for (const auto& resource : resources.uniform_buffers) {

			const auto& buffertype = compiler.get_type(resource.base_type_id);
			uint32_t buffersize = compiler.get_declared_struct_size(buffertype);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			int membercount = buffertype.member_types.size();
			
			//I want only the resources for set 1 because set 1 is the user set for the pipeline
			if (set != targetset) {
				continue;
			}

			Engine::INFO << resource.name.c_str() << "\n";
			Engine::INFO << "Size = " << buffersize << "\n";
			Engine::INFO << "Binding = " << binding << "\n";
			Engine::INFO << "Members = " << membercount << "\n\n";

			ShaderResource res{};
			res.binding = binding;
			res.name = Engine::String::create(resource.name.c_str());
			res.type = GFX::DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			res.set = set;

			
			UniformBufferResource ubr{};
			ubr.userbufferoffset = userbufferoffset;
			ubr.members = Engine::ArrayList<BufferMember>::create(membercount);
			ubr.buffersize = buffersize;

			//increment user buffer offset to accomodate for the next buffer
			userbufferoffset += buffersize;

			for (unsigned i = 0; i < membercount; i++)
			{
				//the buffer member;
				BufferMember bm;

				auto& member_type = compiler.get_type(buffertype.member_types[i]);
				size_t member_size = compiler.get_declared_struct_member_size(buffertype, i);

				// Get member offset within this struct.
				size_t offset = compiler.type_struct_member_offset(buffertype, i);
			
				const std::string& name = compiler.get_member_name(buffertype.self, i);
				Engine::INFO << "member: " << i << ", Name: " << name.c_str() << ", Size: " << member_size << ", Offset" << offset << "\n";

				//im not going to use multi dimensional arrays for now!
				bm.name = Engine::String::create(name.c_str());

				bm.arraysize = member_type.array.size() == 0 ? 1 : member_type.array[0];

				if (bm.arraysize > 1) {
					bm.arraystride = compiler.type_struct_member_array_stride(buffertype, i);
				}
				
				bm.vecsize = member_type.vecsize;
				bm.columsize = member_type.columns;

				bm.offset = offset;
				bm.userbufferoffset = ubr.userbufferoffset + offset;

				bm.type = LC_BUFFER_MEMBER_UNDEFINED;

				//for now only support float and unit
				if (member_type.basetype == spirv_cross::SPIRType::Float) {
					bm.type = LC_BUFFER_MEMBER_FLOAT;
				}

				if (member_type.basetype == spirv_cross::SPIRType::UInt) {
					bm.type = LC_BUFFER_MEMBER_UINT;
				}
				ubr.members.pushBack(bm);
			}

			res.uniformbufferresource = Engine::Allocator::push<UniformBufferResource>(ubr);
			result.pushBack(res);
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

		Engine::INFO << "\n===================================================\n\n";

		return result;
	}

	uint32 getUserBufferSize(Base::Array<ShaderResource> resources, uint32 targetset) {
		
		uint32 size = 0;
		for (auto& res : resources) {
			if (res.set == targetset && res.type == GFX::DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
				size += res.uniformbufferresource->buffersize;
			}
		}
		return size;
	}
	
	GFX::DescriptorSetLayout createDescriptorSetLayoutForResources(Base::Array<ShaderResource> resources, uint32 targetset) {

		GFX::DescriptorSetLayoutCreateinfo inf;
		Engine::ArrayList<GFX::DescriptorBinding> bindings = Engine::ArrayList<GFX::DescriptorBinding>::create(resources.count);
		for (auto& res : resources) {
			if (res.set == targetset) {
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
	bool32 updateDescriptorSet(Base::Array<ShaderResource> resources, GFX::Buffer userbuffer, GFX::DescriptorSet set, uint32 targetset) {

		bool32 result = true;
		for (const auto& res : resources) {
			if (res.set == targetset) {
				
				GFX::DescriptorWrite w{};
				w.arrayelement = 0;
				w.descriptorcount = 1;
				w.dstbinding = res.binding;
				w.dstset = set;
				w.dynamicwrite = false;
				w.type = res.type;

				if (res.type == GFX::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {

					if (!res.texture) {
						result = false;
						continue;
					}
					
					Texture* tex = (Texture*)res.texture->assetdata;

					GFX::DescriptorImageInfo im;
					im.texture = tex->texture;
					w.descriptorinfo = &im;
					GFX::updateDescriptorSets(&w, 1);
				}

				if (res.type == GFX::DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
					GFX::DescriptorBufferInfo bi;
					bi.buffer = userbuffer;
					bi.offset = res.uniformbufferresource->userbufferoffset;
					bi.range = res.uniformbufferresource->buffersize;
					w.descriptorinfo = &bi;
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

	bool32 revalidateDescriptorSet(GraphicsPipeline* pipe) {

		GFX::DescriptorSet set = createResourceDescriptorSet(pipe->userlayout);
		pipe->isvalid = updateDescriptorSet(pipe->resources.getArray(), pipe->userbuffer.buffer, set, 1) && (pipe->internalpipeline != PH_GFX_NULL);

		if (pipe->defaultuserdescriptorset != PH_GFX_NULL) {
			GFX::destroyDescriptorSets(&pipe->defaultuserdescriptorset, 1);
		}

		pipe->defaultuserdescriptorset = set;

		return true;
	}

	//tries to couple the old resources with the new, if any resource of the new resources does not exist in the old resources returns false false
	bool32 revalidateResourcesForTargetSetBinding(Base::Array<ShaderResource> newresources, Base::Array<ShaderResource> oldresources, uint32 targetset) {
		bool32 isvalid = true;
		for (auto& newres : newresources) {

			if (newres.set != targetset) {
				continue;
			}

			//uniform buffer descriptors are automatically valid
			if (newres.type == GFX::DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
				continue;
			}

			//trying to find if the resources exists in the existing resource pack
			if (newres.type == GFX::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
				bool32 foundres = false;
				for (auto& oldres : oldresources) {
					if (oldres.set == newres.set && oldres.binding == newres.binding && oldres.type == newres.type) {
						newres.texture = oldres.texture;
						foundres = true;
						break;
					}
				}
				isvalid &= foundres;
			}
		}
		return isvalid;
	}

#define USER_DESCRIPTORSET_TARGET 1

	bool32 areResourcesValidForTargetSet(Base::Array<ShaderResource> resources, uint32 targetset) {
		for (auto& res : resources) {
			if (res.set != targetset) {
				continue;
			}

			if(res.type == GFX::DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
				continue;
			}

			if (res.type == GFX::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER && res.texture == nullptr) {
				return false;
			}
		}
		return true;
	}

	bool32 reloadGraphicsPipelineFromBinaries(GraphicsPipeline* pipe, const Binaries& b, uint8* userbufferdata = nullptr) {

		auto res = reflectOnBinaries(b, 1, pipe->srcpath.getC_Str());
		GFX::DescriptorSetLayout userlayout = createDescriptorSetLayoutForResources(res.getArray(), 1);
		//GFX::DescriptorSet defaultdescriptorset = createResourceDescriptorSet(userlayout);

		uint32 userbuffersize = getUserBufferSize(res.getArray(), USER_DESCRIPTORSET_TARGET);
		
		//create the user uniform buffer
		if (pipe->userbuffer.capacity != userbuffersize) {
			if (userbuffersize > 0) {
				auto userbufferlocaldata = Engine::Allocator::alloc(userbuffersize);

				if (pipe->userbufferlocaldata) {
					Base::copyMemory(pipe->userbufferlocaldata, userbufferlocaldata, Base::minVal(pipe->userbuffer.capacity, (sizeptr)userbuffersize));
				}

				auto userbuffer = Engine::StaticGPUBuffer<uint8>::create({ userbufferdata ? userbufferdata : (uint8*)userbufferlocaldata, userbuffersize }, GFX::BUFFER_USAGE_UNIFORM_BUFFER_BIT | GFX::BUFFER_USAGE_TRANSFER_DST_BIT);

				if (pipe->userbuffer.buffer != PH_GFX_NULL) {
					Engine::StaticGPUBuffer<uint8>::destroy(&pipe->userbuffer);
				}
				
				
				if (pipe->userbufferlocaldata != nullptr) {
					Engine::Allocator::dealloc(pipe->userbufferlocaldata);
				}

				pipe->userbuffer = userbuffer;
				pipe->userbufferlocaldata = userbufferlocaldata;
			} else {
				pipe->userbuffer = {};
				pipe->userbuffer.buffer = PH_GFX_NULL;
				pipe->userbuffer.capacity = 0;
			}
		}
		
		GFX::GraphicsPipeline p = buildGraphicsPipeline(pipe->type, b, userlayout);

		if (p == PH_GFX_NULL) {
			res.destroy(&res);

			Engine::WARN << "failed to create graphics pipeline " << pipe->srcpath.getC_Str() << "\n";
			return false;
		}

		pipe->isvalid = false;

		if (pipe->internalpipeline != PH_GFX_NULL) {
			//add code to destroy old pipe;
		}

		if (pipe->userlayout != PH_GFX_NULL) {
			GFX::destroyDescriptorSetLayouts(&pipe->userlayout, 1);
		}

		if (pipe->defaultuserdescriptorset != PH_GFX_NULL) {
			GFX::destroyDescriptorSets(&pipe->defaultuserdescriptorset, 1);
		}
		
		pipe->internalpipeline = p;
		pipe->userlayout = userlayout;
		pipe->defaultuserdescriptorset = PH_GFX_NULL;

		if (revalidateResourcesForTargetSetBinding(res.getArray(), pipe->resources.getArray(), USER_DESCRIPTORSET_TARGET)) {
			revalidateDescriptorSet(pipe);
		}

		Engine::ArrayList<ShaderResource>::destroy(&pipe->resources);
		pipe->resources = res;

		return true;
	}

	LabCore3D::GraphicsPipeline createGraphicsPipeline(const char* sourcepath, const char* binpath, PipelineType type) {

		GraphicsPipeline pl{};
		pl.internalpipeline = PH_GFX_NULL;
		pl.binpath = Engine::String::create(binpath);
		pl.srcpath = Engine::String::create(sourcepath);
		pl.resources = Engine::ArrayList<ShaderResource>{};
		pl.type = type;
		pl.isvalid = false;

		Binaries b;
		if (!loadBinaries(&b, sourcepath, binpath)) {
			Engine::WARN << "failed to load binaries for pipeline: " << sourcepath;
		}

		reloadGraphicsPipelineFromBinaries(&pl, b);

		releaseBinaries(&b);

		return pl;
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

		reloadGraphicsPipelineFromBinaries(pipe, b);

		releaseBinaries(&b);
	}

	void deserializeGraphicsPipeline(YAML::Node n, GraphicsPipeline* pipe) {

		pipe->isvalid = false;
		
		if (!n) {
			return;
		}

		if (!n["type"] || !n["srcpath"] || !n["binpath"]) {
			Engine::WARN << "failed to deserialize graphics pipeline because either type, srcpath or binpath are invalid!\n";
			return;
		}
		
		PipelineType type = (PipelineType)n["type"].as<uint32>();
		Engine::String srcpath = n["srcpath"].as<Engine::String>();
		Engine::String binpath = n["binpath"].as<Engine::String>();

		*pipe = createGraphicsPipeline(srcpath.getC_Str(), binpath.getC_Str(), type);

		Engine::ArrayList<ShaderResource> resources = Engine::ArrayList<ShaderResource>::create(1);

		if (n["resources"]) {
			for (auto res : n["resources"]) {
				ShaderResource sr{};
				sr.binding = res["binding"].as<uint32>();
				sr.set = res["set"].as<uint32>();
				sr.type = (GFX::DescriptorType)res["type"].as<uint32>();
				if (sr.type == GFX::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
					Engine::AssetID texid = res["texture"].as<Engine::AssetID>();

					//might want to add the asset library as a parameter, on the other hand, this is all in labcore space
					sr.texture = LabCore::getAssetLibrary()->getAssetIdentifier<LabCore3D::Texture>(texid);
				}
				resources.pushBack(sr);
			}
		}

		if (n["userbuffer"]) {
			YAML::Binary b = n["userbuffer"].as<YAML::Binary>();

			//only upload memory if the shader actually requires it
			if (pipe->userbuffer.capacity > 0) {
				pipe->userbuffer.uploadData((uint8*)b.data(), b.size());
				Base::copyMemory((void*)b.data(), pipe->userbufferlocaldata, Base::minVal(pipe->userbuffer.capacity, b.size()));
			}
		}

		//if the resources completly revalidate the descriptorset than revalidate the descriptorset
		if (revalidateResourcesForTargetSetBinding(pipe->resources.getArray(), resources.getArray(), 1)) {
			revalidateDescriptorSet(pipe);
		}

		//destroy created resources
		Engine::String::destroy(&binpath);
		Engine::String::destroy(&srcpath);
		Engine::ArrayList<ShaderResource>::destroy(&resources);

	}

	bool32 revalidateTextures(GraphicsPipeline* pipe) {
		return revalidateDescriptorSet(pipe);
	}

	bool32 revalidateUniformBuffers(GraphicsPipeline* pipe, Engine::DynamicGPUBuffer<uint8>* stagingbuffer) {

		if (stagingbuffer) {
			pipe->userbuffer.uploadDataAsynchronous((uint8*)pipe->userbufferlocaldata, pipe->userbuffer.capacity, stagingbuffer);
			return true;
		}

		pipe->userbuffer.uploadDataAsynchronous((uint8*)pipe->userbufferlocaldata, pipe->userbuffer.capacity);
	}


	void serializeResources(YAML::Emitter& out, Base::Array<ShaderResource> resources) {

		out << YAML::BeginSeq;

		for (auto& res : resources) {
			out << YAML::Value << YAML::BeginMap;

			out << YAML::Key << "name" << YAML::Value << res.name.getC_Str();
			out << YAML::Key << "binding" << YAML::Value << res.binding;
			out << YAML::Key << "type" << YAML::Value << res.type;
			out << YAML::Key << "set" << YAML::Value << res.set;

			if (res.type == GFX::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
				if (res.texture) {
					out << YAML::Key << "texture" << YAML::Value << res.texture->assetid;
				}
				else {
					out << YAML::Key << "texture" << YAML::Value << PH_ASSET_NULL;
				}
			}
			out << YAML::EndMap;
		}

		out << YAML::EndSeq;

	}

	void serializeGraphicsPipeline(YAML::Emitter& out, GraphicsPipeline* pipe) {
		out << YAML::BeginMap;

		out << YAML::Key << "type" << YAML::Value << pipe->type;

		out << YAML::Key << "binpath" << pipe->binpath.getC_Str();
		out << YAML::Key << "srcpath" << pipe->srcpath.getC_Str();
		if (pipe->userbuffer.capacity != 0) {
			out << YAML::Key << "userbuffer" << YAML::Value << YAML::Binary((uint8*)pipe->userbufferlocaldata, pipe->userbuffer.capacity);
		}
		out << YAML::Key << "resources" << YAML::Value;


		serializeResources(out, pipe->resources.getArray());
		out << YAML::EndMap;
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