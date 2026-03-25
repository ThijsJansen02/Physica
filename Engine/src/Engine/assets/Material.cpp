#include "Material.h"
#include <Platform/platformAPI.h>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>

#include "../YamlExtensions.h"

#include "../Display.h"
#include "../AssetLibrary.h"
#include "../Engine.h"
#include "../Scene.h"

#include <Base/Datastructures/String.h>

namespace PH::Engine::Assets {

	using namespace PH::Platform;

	bool32 checkForBinaries(ShaderModule* module) {
		if (!module->loadedbinary) {
			FileBuffer file;
			if (!loadFile(&file, module->binarypath.getC_Str())) {
				WARN << "Failed to load file " << module->binarypath.getC_Str() << "\n";
				return false;
			}

			module->loadedbinary = Engine::Allocator::alloc(file.size);
			module->loadedbinarysize = file.size;
			Base::copyMemory(file.data, module->loadedbinary, file.size);

			unloadFile(&file);
		}
	}
	
	GFX::DescriptorType getDescriptorType(UniformResource* res) {
		switch (res->type) {
		case UniformResourceType::UNIFORM_BUFFER:
			return GFX::DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			break;
		case UniformResourceType::UNIFORM_SAMPLER:
			return GFX::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			break;
		}
	}

	GFX::DescriptorSetLayout createMaterialDSLayout(Material* mat) {

		auto bindings = Base::ArrayList<GFX::DescriptorBinding, ArenaAllocator>::create(1);

		//clear the ubodescriptions
		for (auto* res : mat->resources.getAll<Engine::ArenaAllocator>()) {
			GFX::DescriptorBinding binding{};
			binding.type = getDescriptorType(res);
			binding.descriptorcount = res->arraysize;
			binding.shaderstages = res->shaderstages;
			binding.binding = res->binding;

			bindings.pushBack(binding);
		}
		
		GFX::DescriptorSetLayoutCreateinfo dc{};
		dc.bindings = { bindings.raw(), bindings.getCount() };

		if (bindings.getCount() == 0) {
			return PH_GFX_NULL;
		}

		GFX::DescriptorSetLayout layout;
		GFX::createDescriptorSetLayouts(&dc, &layout, 1);

		return layout;
	}

	uint32 getMaterialTextureCount(Material* material) {
		Engine::ArenaScope s();

		uint32 count = 0;
		for (auto* res : material->resources.getAll<Engine::ArenaAllocator>()) {
			if (res->type == UniformResourceType::UNIFORM_SAMPLER) {
				count += res->arraysize;
			}
		}
		return count;
	}

	bool32 compileMaterialGraphicsPipeline(Material* material, GFX::RenderpassDescription renderpass, GFX::DescriptorSetLayout scenedescriptorset) {

		auto resetpoint = Engine::ArenaAllocator::getResetPoint();
		GFX::Shader shaders[2] = { material->vertexshader.shader, material->fragmentshader.shader };

		for (auto shader : shaders) {
			if (shader == PH_GFX_NULL) {
				return false;
			}
		}

		auto layouts = Base::ArrayList<GFX::DescriptorSetLayout, Engine::ArenaAllocator>::create(1);
		
		//if there is already a layout, delete it
		if (material->layout != PH_GFX_NULL) {
			GFX::destroyDescriptorSetLayouts(&material->layout, 1);
		}

		material->layout = createMaterialDSLayout(material);
		
		layouts.pushBack(scenedescriptorset);
		
		if (material->layout != PH_GFX_NULL) {
			layouts.pushBack(material->layout);
		}

		//adding it all together in the pipeline create;
		PH::Platform::GFX::GraphicsPipelineCreateinfo pipelinecreate{};
		pipelinecreate.layouts = layouts.getArray();
		pipelinecreate.renderpass = renderpass;
		pipelinecreate.shaderstages = { shaders, 2 };
		pipelinecreate.topology = Platform::GFX::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipelinecreate.vertexbindingdescriptions = { Renderer3D::vertexbindingdescription, ARRAY_LENGTH(Renderer3D::vertexbindingdescription) };
		pipelinecreate.vertexattributedescriptions = { Renderer3D::vertexinputattributes, ARRAY_LENGTH(Renderer3D::vertexinputattributes) };
		pipelinecreate.cullmode = material->cullmode;

		pipelinecreate.depthtest = material->depthtest;

		Platform::GFX::GraphicsPipeline pipeline;
		if (!Platform::GFX::createGraphicsPipelines(&pipelinecreate, &pipeline, 1)) {
			return false;
		}

		material->texturecount = getMaterialTextureCount(material);
		material->pipeline = pipeline;
		material->compiled = true;
		material->status = AssetStatus::LOADED;
		return true;

		Engine::ArenaAllocator::resetArena(resetpoint);
	}

	const char* ShaderstageToString(GFX::ShaderStageFlags shaderstage) {
		switch (shaderstage) {
		case GFX::SHADER_STAGE_FRAGMENT_BIT:
			return "Fragement Shader";

		case GFX::SHADER_STAGE_VERTEX_BIT:
			return "Vertex Shader";

		default:
			return "Unsupported shader stage";
		}
	}

	void puttabs(uint32 count) {
		for (uint32 i = 0; i < count; i++) {
			INFO << "\t";
		}
	}

	bool32 reflectTraceStruct(const spirv_cross::TypeID type_id, spirv_cross::Compiler& compiler, uint32 tc) {

		const spirv_cross::SPIRType& type = compiler.get_type(type_id);

		uint32_t buffersize = compiler.get_declared_struct_size(type);
		
		int membercount = type.member_types.size();

		puttabs(tc);
		INFO << "struct size: " << buffersize << "\n";
		puttabs(tc);
		INFO << "membercount: " << membercount << "\n";

		for (uint32 i = 0; i < membercount; i++) {

			const spirv_cross::TypeID membertype_id = type.member_types[i];
			const spirv_cross::SPIRType& membertype = compiler.get_type(membertype_id);

			std::string name = compiler.get_member_name(type.self, i);
			uint32 offset = compiler.get_member_decoration(type.self, i, spv::Decoration::DecorationOffset);
						
			 uint32 arraysize = 1;
			if (membertype.array.size() > 0) {
				arraysize = membertype.array[0];
			}

			puttabs(tc + 1);
			INFO << "member: " << i << " : " << name.c_str() << "[" << arraysize << "]" << " {" << "offset: " << offset;

			
			//puttabs(tc + 1);
			if (membertype.basetype == spirv_cross::SPIRType::BaseType::Struct) {
				INFO << "\n";
				reflectTraceStruct(membertype_id, compiler, tc + 2);
			}

			if (membertype.basetype == spirv_cross::SPIRType::BaseType::Float) {
				INFO << "type: float " << membertype.vecsize << "x" << membertype.columns;
			}

			INFO << "}\n";
		}

		return true;
	}

	bool32 reflectTraceUBO(const spirv_cross::Resource& resource, spirv_cross::Compiler& compiler) {

		const spirv_cross::SPIRType& UBOtype = compiler.get_type(resource.base_type_id);

		std::string name = compiler.get_name(resource.id);
		uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
		uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

		int membercount = UBOtype.member_types.size();

		INFO << "\t\tresource name: " << name.c_str() << "\n";
		INFO << "\t\tdescriptor set: " << set << "\n";
		INFO << "\t\tbinding: " << binding << "\n";

		reflectTraceStruct(resource.base_type_id, compiler, 3);
		
		return true;
	}

	Base::ArrayList<Assets::Member, Engine::ArenaAllocator> getStructDescription(const spirv_cross::TypeID type_id, spirv_cross::Compiler& compiler, uint32 offset, const char* base) {

		auto members = Base::ArrayList<Assets::Member, Engine::ArenaAllocator>::create(1);
		const spirv_cross::SPIRType& type = compiler.get_type(type_id);

		int membercount = type.member_types.size();

		for (uint32 i = 0; i < membercount; i++) {

			uint32 offsetinstruct = compiler.get_member_decoration(type.self, i, spv::Decoration::DecorationOffset);
			const spirv_cross::TypeID membertype_id = type.member_types[i];
			const spirv_cross::SPIRType& membertype = compiler.get_type(membertype_id);

			std::string name = compiler.get_member_name(type.self, i);

			uint32 arraysize = 1;
			if (membertype.array.size() > 0) {
				arraysize = membertype.array[0];
			}

			for (uint32 i = 0; i < arraysize; i++) {
				
				char namebuffer[256];

				//puttabs(tc + 1);
				if (membertype.basetype == spirv_cross::SPIRType::BaseType::Struct) {

					sprintf_s<256>(namebuffer, "%s%s[%u]::", base, name.c_str(), i);
					members.pushBack(getStructDescription(membertype_id, compiler, offset, namebuffer));
				}

				sprintf_s<256>(namebuffer, "%s%s[%u]", base, name.c_str(), i);

				Assets::Member mem;
				mem.name = String::create(namebuffer);
				mem.offset = offsetinstruct + offset;
				if (membertype.basetype == spirv_cross::SPIRType::BaseType::Float) {
					mem.columns = membertype.columns;
					mem.rows = membertype.vecsize;
					mem.format = Assets::MemberType::REAL32;
				}

				members.pushBack(mem);
			}
		}
		return members;
	}

	Assets::UniformResource getUniformBufferDescription(const spirv_cross::Resource& resource, spirv_cross::Compiler& compiler) {

		//set a scope for the arena allocator
		Engine::ArenaScope s();

		Assets::UniformResource description{};
		const spirv_cross::SPIRType& UBOtype = compiler.get_type(resource.base_type_id);

		std::string name = compiler.get_name(resource.id);
		uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
		uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
		uint32_t buffersize = compiler.get_declared_struct_size(UBOtype);

		sizeptr arraysize = 1;
		if (UBOtype.array.size() > 0) {
			arraysize = UBOtype.array[0];
		}

		description.arraysize = arraysize;
		description.type = UniformResourceType::UNIFORM_BUFFER;
		description.binding = binding;
		description.name = String::create(name.c_str());
		description.ubodescription.size = buffersize;
		description.ubodescription.members = ArrayList<Member>::create(1);
		description.ubodescription.members.pushBack(getStructDescription(resource.base_type_id, compiler, 0, ""));

		return description;
	}

	Assets::UniformResource getUniformSamplerDescription(const spirv_cross::Resource& resource, spirv_cross::Compiler& compiler) {
		
		Assets::UniformResource description{};
		const spirv_cross::SPIRType& samplertype = compiler.get_type(resource.base_type_id);

		std::string name = compiler.get_name(resource.id);
		uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
		uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

		sizeptr arraysize = 1;
		if (samplertype.array.size() > 0) {
			arraysize = samplertype.array[0];
		}

		description.type = UniformResourceType::UNIFORM_SAMPLER;
		description.name = String::create(name.c_str());
		description.arraysize = arraysize;
		description.binding = binding;

		return description;
	}


	bool32 reflectOnBinary(ShaderModule* module, GFX::ShaderStageFlags shaderstage, ResourceMap* resourcemap) {

		spirv_cross::Compiler compiler((uint32*)module->loadedbinary, module->loadedbinarysize / 4);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		for (const auto& resource : resources.uniform_buffers)
		{
			uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			if (set != 1) {
				continue;
			}

			UniformResource ubodes = getUniformBufferDescription(resource, compiler);
			UniformResource* ubodesptr = resourcemap->get_last(ubodes.binding);
			if (!ubodesptr) {
				ubodesptr = resourcemap->add(ubodes.binding, ubodes);
			}
			ubodesptr->shaderstages |= shaderstage;
			
		}

		for (const auto& resource : resources.sampled_images) {

			uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			if (set != 1) {
				continue;
			}

			UniformResource samplerdes = getUniformSamplerDescription(resource, compiler);
			UniformResource* samplerdesptr = resourcemap->get_last(samplerdes.binding);
			if (!samplerdesptr) {
				samplerdesptr = resourcemap->add(samplerdes.binding, samplerdes);
			}
			samplerdesptr->shaderstages |= shaderstage;
		}

		return true;
	}

	bool32 tryCompileShaderModuleFromBinaries(ShaderModule* module, GFX::ShaderStageFlags shaderstage, ResourceMap* resources) {

		if (!checkForBinaries(module)) {
			return false;
		}

		if (!reflectOnBinary(module, shaderstage, resources)) {
			WARN << "Failed to reflect on binary";
		}

		Platform::GFX::ShaderCreateinfo createfrombin{};
		createfrombin.chachedir = nullptr;
		createfrombin.size = module->loadedbinarysize;
		createfrombin.sourcecode = (void*)module->loadedbinary;
		createfrombin.stage = shaderstage;
		createfrombin.sourcetype = Platform::GFX::ShaderSourceType::VULKAN_BINARIES;
		
		if (module->shader != PH_GFX_NULL) {
			GFX::destroyShaders(&module->shader, 1);
		}

		GFX::createShaders(&createfrombin, &module->shader, 1);

		return true;
	}

	bool32 tryCompileShaderModuleFromGLSLSource(ShaderModule* module, GFX::ShaderStageFlags shaderstage, ResourceMap* resources) {

		//setup the shaderc comiler
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		const bool optimize = true;
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);
		

		//convert the shader kind
		shaderc_shader_kind kind{};
		switch (shaderstage) {
		case Platform::GFX::SHADER_STAGE_FRAGMENT_BIT:
			kind = shaderc_shader_kind::shaderc_fragment_shader;
			break;
		case Platform::GFX::SHADER_STAGE_VERTEX_BIT:
			kind = shaderc_shader_kind::shaderc_vertex_shader;
			break;
		}

		//compile the shader source to spirv binary
		std::string sourcecode = (const char*)module->sourcecode.getC_Str();
		shaderc::SpvCompilationResult compiledmodule = compiler.CompileGlslToSpv(sourcecode, kind, "ref");
		if (compiledmodule.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			WARN << "Failed to compile shaderstage: " << ShaderstageToString(shaderstage) << "\n\n";
			WARN << compiledmodule.GetErrorMessage().c_str() << "\n";
			return false;
		}

		sizeptr size = (compiledmodule.cend() - compiledmodule.cbegin()) * sizeof(uint32);

		//write the binary to disk cache
		if (module->binarypath.getC_Str()) {
			Platform::FileBuffer file;
			file.data = (void*)compiledmodule.cbegin();
			file.size = size;

			PH::Platform::writeFile(file, module->binarypath.getC_Str());
		}
		
		//deallocate last loaded binary
		if (module->loadedbinary) {
			Engine::Allocator::dealloc(module->loadedbinary);
		}
		
		//copy the binary in our own local memory;
		module->loadedbinarysize = size;
		module->loadedbinary = Engine::Allocator::alloc(module->loadedbinarysize);

		Base::copyMemory((void*)compiledmodule.cbegin(), module->loadedbinary, module->loadedbinarysize);

		return tryCompileShaderModuleFromBinaries(module, shaderstage, resources);
	}

	bool32 compileMaterialFromBinaries(Material* material, Platform::GFX::RenderpassDescription intendedrenderpass, GFX::DescriptorSetLayout scenedescriptorsetlayout) {

		ResourceMap newresourcemap = ResourceMap::create(4);
		ResourceMap oldresourcemap = material->resources;

		if (!tryCompileShaderModuleFromBinaries(&material->fragmentshader, GFX::SHADER_STAGE_FRAGMENT_BIT, &newresourcemap)) {
			ResourceMap::destroy(&newresourcemap);
			return false;
		}

		if (!tryCompileShaderModuleFromBinaries(&material->vertexshader, GFX::SHADER_STAGE_VERTEX_BIT, &newresourcemap)) {
			ResourceMap::destroy(&newresourcemap);
			return false;
		}
		
		material->resources = newresourcemap;
		if (!compileMaterialGraphicsPipeline(material, intendedrenderpass, scenedescriptorsetlayout)) {
			material->resources = oldresourcemap;
			ResourceMap::destroy(&newresourcemap);
			Engine::WARN << "Failed to compile material graphics pipeline " << material->references[0].getC_Str() << "\n";
			return false;
		}

		ResourceMap::destroy(&oldresourcemap);
		return true;

	}

	bool32 compileMaterialFromSource(
		Material* material, 
		Platform::GFX::RenderpassDescription intendedrenderpass, 
		GFX::DescriptorSetLayout scenedescriptorsetlayout
	) {

		ResourceMap newresourcemap = ResourceMap::create(4);
		ResourceMap oldresourcemap = material->resources;

		if (!tryCompileShaderModuleFromGLSLSource(&material->vertexshader, GFX::SHADER_STAGE_VERTEX_BIT, &newresourcemap)) {
			ResourceMap::destroy(&newresourcemap);
			return false;
		}

		if (!tryCompileShaderModuleFromGLSLSource(&material->fragmentshader, GFX::SHADER_STAGE_FRAGMENT_BIT, &newresourcemap)) {
			ResourceMap::destroy(&newresourcemap);
			return false;
		}

		material->resources = newresourcemap;
		if (!compileMaterialGraphicsPipeline(material, intendedrenderpass, scenedescriptorsetlayout)) {
			ResourceMap::destroy(&newresourcemap);
			material->resources = oldresourcemap;
			Engine::WARN << "Failed to compile material graphics pipeline: " << material->references[0].getC_Str() << "\n";
			return false;
		}

		ResourceMap::destroy(&oldresourcemap);
		return true;
	}

	Material createMaterial(const char* bindir, const char* binname) {

		Material mat{};
		mat.instances = ArrayList<UUID>::create(1);

		Base::SubString assetpath(bindir);
		mat.fragmentshader.binarypath = String::create(bindir).append("/").append(binname).append("_fs.spv");
		mat.fragmentshader.sourcecode = String::create("#version 450");

		mat.vertexshader.binarypath = String::create(bindir).append("/").append(binname).append("_vs.spv");
		mat.vertexshader.sourcecode = String::create("#version 450");

		mat.compiled = false;
		mat.status = AssetStatus::LOADED;
		return mat;
	}

	bool32 destroyMaterialInstanceForRebuild(MaterialInstance* instance) {

		GFX::destroyDescriptorSets(&instance->descriptorset, 1);
		instance->status = AssetStatus::NOT_LOADED;

		return true;
	}

	bool32 rebuildMaterialInstance(MaterialInstance* inst, TextureImage* defaultimage) {
		destroyMaterialInstanceForRebuild(inst);
		return createMaterialInstance(inst->parent, inst, defaultimage);
	}

	bool32 setTextureForMaterialInstanceSafe(MaterialInstance* instance, uint32 binding, uint32 arrayelement, TextureImage* image, Library* lib) {
		
		if (image->status == AssetStatus::NOT_LOADED) {
			lib->load<TextureImage>(image->id);
		}

		if (image->status == AssetStatus::LOADING) {
			
			//wait for the image to load, this should of course be handled by a synchronization primitive... but these are not yet in place
			while (image->status == AssetStatus::LOADING) {
				INFO << "waiting on image to load!\n";
				Sleep(100);
			}

			if (image->status != AssetStatus::LOADED) {
				return false;
			}
		}

		setTextureForMaterialInstance(instance, binding, arrayelement, image);
	}

	bool32 setTextureForMaterialInstance(MaterialInstance* instance, uint32 binding, uint32 arrayelement, TextureImage* image) {
		
		UniformResource* res = instance->resources.get_last(binding);
		if (!res) {
			return false;
		}

		if (res->arraysize < arrayelement || res->type != UniformResourceType::UNIFORM_SAMPLER) {
			return false;
		}

		if (!image) {
			return false;
		}

		res->samplerdescription.boundimages[arrayelement] = image;

		GFX::DescriptorImageInfo info{};
		info.texture = image->base;

		GFX::DescriptorWrite write{};
		write.descriptorcount = 1;
		write.descriptorinfo = &info;
		write.dstset = instance->descriptorset;
		write.type = GFX::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.arrayelement = arrayelement;
		write.dstbinding = binding;
			
		write.dynamicwrite = false;
		Platform::GFX::updateDescriptorSets(&write, 1);
	}

	void releaseUniformResource(UniformResource* res) {
		String::destroy(&res->name);

		if (res->type == UniformResourceType::UNIFORM_BUFFER) {
			res->ubodescription.members.release();
			res->ubodescription.boundbuffers.release();
		}

		if (res->type == UniformResourceType::UNIFORM_SAMPLER) {
			res->samplerdescription.boundimages.release();
		}
	}

	bool32 isResourceCompatible(UniformResource* derived, UniformResource* base) {

		if (derived->type != base->type) {
			return false;
		}

		if (derived->binding != base->binding) {
			return false;
		}

		if (derived->arraysize != base->arraysize) {
			return false;
		}

		if (derived->type == UniformResourceType::UNIFORM_BUFFER) {
			if (derived->ubodescription.size != base->ubodescription.size) {
				return false;
			}
		}

		return true;
	}

	UniformResource createUniformResource(UniformResource* base, TextureImage* defaulttexture) {
		UniformResource result = *base;
		result.name = String::create(base->name.getC_Str());
		

		if(result.type == UniformResourceType::UNIFORM_BUFFER) {
			result.ubodescription.boundbuffers = Engine::ArrayList<UniformBufferSubResource>::create(result.arraysize);
			for (uint32 i = 0; i < result.arraysize; i++) {

				//this should be optimized to one big buffer for the entire array and maybe the entire material. secondly it should be in a static buffer
				UniformBufferSubResource buffer{};
				buffer.base = createDynamicUniformBuffer(result.ubodescription.size);
				buffer.offset = 0;
				buffer.size = result.ubodescription.size;
				result.ubodescription.boundbuffers.pushBack(buffer);
			}
		}

		if (result.type == UniformResourceType::UNIFORM_SAMPLER) {
			result.samplerdescription.boundimages = Engine::ArrayList<TextureImage*>::create(result.arraysize);
			
			for (uint32 i = 0; i < result.arraysize; i++) {
				result.samplerdescription.boundimages.pushBack(defaulttexture);
			}
		}

		return result;
	}

	bool32 createMaterialInstance(Material* sourcematerial, MaterialInstance* instance, TextureImage* defaultimage) {

		Engine::ArenaScope s();

		MaterialInstance result = *instance;
		result.parent = sourcematerial;
		result.type = AssetType::MATERIAL_INSTANCE;

		if (!sourcematerial->compiled) {
			*instance = result;
			return false;
		}

		auto resetpoint = Engine::ArenaAllocator::getResetPoint();
	
		//create the descriptorset
		GFX::DescriptorSetCreateinfo createinfo;
		createinfo.layout = sourcematerial->layout;
		createinfo.dynamic = false;
		
		result.descriptorset = PH_GFX_NULL;
		if (sourcematerial->layout != PH_GFX_NULL) {
			GFX::createDescriptorSets(&createinfo, &result.descriptorset, 1);
		}

		//an array for the write that are going to be made to the descriptorset
		auto writes = Base::ArrayList<GFX::DescriptorWrite, Engine::ArenaAllocator>::create(1);

		//if there is no resource map yet, create it
		if (!result.resources.table.raw()) {
			result.resources = ResourceMap::create(1);
		}

		for(auto* materialres : sourcematerial->resources.getAll<Engine::ArenaAllocator>()) {

			//check if there already exists a resource at the specified binding
			UniformResource* res = result.resources.get_last(materialres->binding);
			if (!res) {
				res = result.resources.add(materialres->binding, {});
				*res = createUniformResource(materialres, defaultimage);
			}
			else {
				if (!isResourceCompatible(res, materialres)) {
					releaseUniformResource(res);
					*res = createUniformResource(materialres, defaultimage);
				}
			}

			GFX::DescriptorWrite write{};
			write.arrayelement = 0;
			write.descriptorcount = res->arraysize;
			write.dstbinding = res->binding;
			write.dstset = result.descriptorset;
			write.dynamicwrite = false;

			//create the descriptor writes
			if (res->type == UniformResourceType::UNIFORM_BUFFER) {
				auto bufferinfos = Base::ArrayList<GFX::DescriptorBufferInfo, ArenaAllocator>::create(res->arraysize);

				for (uint32 i = 0; i < res->arraysize; i++) {
					GFX::DescriptorBufferInfo bufferinfo{};
					bufferinfo.buffer = res->ubodescription.boundbuffers[i].base;
					bufferinfo.offset = 0;
					bufferinfo.range = res->ubodescription.size;

					bufferinfos.pushBack(bufferinfo);
				}

				write.type = GFX::DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				write.descriptorinfo = bufferinfos.raw();
			}

			if (materialres->type == UniformResourceType::UNIFORM_SAMPLER) {

				auto imageinfos = Base::ArrayList<GFX::DescriptorImageInfo, ArenaAllocator>::create(res->arraysize);
				for (uint32 i = 0; i < res->arraysize; i++) {
					GFX::DescriptorImageInfo imageinfo{};
					imageinfo.texture = res->samplerdescription.boundimages[i]->base;
					imageinfos.pushBack(imageinfo);
				}

				write.type = GFX::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				write.descriptorinfo = imageinfos.raw();	
			}
			//push back the write
			writes.pushBack(write);
		}

		//remove resources that do not exist anymore
		for (auto res : result.resources.getAll<Engine::ArenaAllocator>()) {
			if (!result.parent->resources.contains_key(res->binding)) {
				releaseUniformResource(res);
				result.resources.remove(res->binding);
			}
		}

		GFX::updateDescriptorSets(writes.raw(), writes.getCount());
		result.status = AssetStatus::LOADED;
		*instance = result;
		return true;
	}

	bool32 deserializeMaterial(YAML::Node& root, Library* lib, Material* result, const char* path) {

		Material mat = *result;
		if (!root) {
			return false;
		}

		mat.type = AssetType::MATERIAL;
		mat.resources = ResourceMap::create(4);

		mat.cullmode = GFX::CULL_MODE_NONE;
		auto cm = root["CullMode"];
		if (cm) {
			mat.cullmode = cm.as<GFX::CullModeFlags>();
		}

		if (!root["DepthTest"]) {
			mat.depthtest = true;
		}
		else {
			mat.depthtest = root["DepthTest"].as<bool>();
		}

		auto fragmentshader = root["FragmentShader"];
		if (!fragmentshader) {
			WARN << "No fragmentshader found! path:" << path << "\n";
			//in the future just create a fragmentshader
			return false;
		}
		mat.fragmentshader.sourcecode = fragmentshader["Source"].as<String>();
		mat.fragmentshader.binarypath = fragmentshader["BinDir"].as<String>();

		auto vertexshader = root["VertexShader"];
		if (!vertexshader) {
			WARN << "Failed to load vertexshader, assetpath: " << path << "\n";
			return false;
		}
		mat.vertexshader.sourcecode = vertexshader["Source"].as<String>();
		mat.vertexshader.binarypath = vertexshader["BinDir"].as<String>();

		auto instances = root["Instances"];
		mat.instances = Engine::ArrayList<UUID>::create(root.size());
		if (instances) {
			for (auto instance : instances) {
				UUID id = instance.as<UUID>();
				if (!mat.instances.contains(id)) {
					mat.instances.pushBack(id);
				}
			}
		}

		mat.compiled = false;
		if (compileMaterialFromBinaries(&mat, Engine::Display::defaultrenderpassdescription)) {
			mat.compiled = true;
		}
		else if (compileMaterialFromSource(&mat, Engine::Display::defaultrenderpassdescription)) {
			mat.compiled = true;
		}

		*result = mat;
		return true;
	}

	bool32 serializeMaterial(YAML::Emitter& out, Material* material) {

		out << YAML::BeginMap;

		out << YAML::Key << "FragmentShader" << YAML::Value << YAML::BeginMap; //fragmentshader
		out << YAML::Key << "Source" << YAML::Value << YAML::Literal << material->fragmentshader.sourcecode.getC_Str();
		out << YAML::Key << "BinDir" << YAML::Value << material->fragmentshader.binarypath.getC_Str();
		out << YAML::EndMap;//fragmentshader

		out << YAML::Key << "VertexShader" << YAML::Value << YAML::BeginMap; //vertexshaders
		out << YAML::Key << "Source" << YAML::Value << YAML::Literal << material->vertexshader.sourcecode.getC_Str();
		out << YAML::Key << "BinDir" << YAML::Value << material->vertexshader.binarypath.getC_Str();
		out << YAML::EndMap; //vertexshader

		out << YAML::Key << "DepthTest" << YAML::Value << (bool)material->depthtest;
		out << YAML::Key << "CullMode" << YAML::Value << material->cullmode;

		out << YAML::Key << "Instances" << YAML::Value << YAML::BeginSeq;
		
		for (auto instance : material->instances) {
			out << instance;
		}

		out << YAML::EndSeq;

		out << YAML::EndMap;
		return true;
	}

	bool32 serializeMaterial(const char* path, Material* mat) {
		YAML::Emitter out;

		out << YAML::BeginMap;
		serializeMaterial(out, mat);
		out << YAML::EndMap;

		return FileIO::writeYamlFile(out, path);
	}

	bool32 deserializeMaterial(const char* path, Library* lib, Material* result) {
		auto node = FileIO::loadYamlfile(path);
		return deserializeMaterial(node, lib, result, path);
	}

	bool32 deserializeMaterialInstance(YAML::Node& root,  Library* lib, MaterialInstance* result, const char* path) {

		Material* parent = lib->getLoaded<Material>(root["ParentMaterial"].as<UUID>());

		if (!parent) {
			WARN << "this materialinstance's master material is not present!\n";
			return false;
		}

		MaterialInstance mat = *result;
		mat.resources = ResourceMap::create(1);
		mat.status = AssetStatus::LOADED;

		//maybe at default image
		if (!createMaterialInstance(parent, &mat, lib->getLoadedByReference<TextureImage>("default_texture"))) {
			mat.status = AssetStatus::NOT_LOADED;
			Engine::WARN << "loaded material instance for a material that is not compiled! \n";
		}

		if (mat.status == AssetStatus::LOADED) {
			uint32 index = 0;
			for (const auto& res : root["UniformResources"]) {
				if (!res) {
					continue;
				}

				uint32 binding = res["Binding"].as<uint32>();
				String type = res["Type"].as<String>();

				//if it is a uniform sampler, try to set the uniform samplers
				if (Base::stringCompare(type.getC_Str(), "UNIFORM_SAMPLER")) {
					uint32 i = 0;
					for (auto tex : res["Textures"]) {
						UUID texid = tex.as<UUID>();
						TextureImage* im = lib->getLoaded<TextureImage>(texid);
						TextureImage* defaultim = lib->getLoadedByReference<TextureImage>("default_texture");

						setTextureForMaterialInstance(&mat, binding, i, im ? im : defaultim);
						i++;
					}
				}

				//if it is a uniform buffer, try to set the uniform buffer
				if (Base::stringCompare(type.getC_Str(), "UNIFORM_BUFFER")) {

					UniformResource* resource = mat.resources.get_last(binding);
					if (!resource) {
						continue;
					}

					if (resource->type == UniformResourceType::UNIFORM_BUFFER) {

						uint32 index = 0;
						for (auto buffer : res["Data"]) {

							if (index < resource->ubodescription.boundbuffers.getCount()) {
								continue;
							}

							YAML::Binary bin = buffer.as<YAML::Binary>();
							UniformBufferSubResource ubo = resource->ubodescription.boundbuffers[index];

							if (ubo.size <= bin.size()) {
								void* data = mapUniformBufferSubResource(ubo);
								Base::copyMemory((void*)bin.data(), data, bin.size());
							}
							index++;
						}
					}
				}
			}
		}

		mat.type = AssetType::MATERIAL_INSTANCE;

		//if the id of the item is already visible, push it back in the parent
		if (result->id != 0) {
			if (!parent->instances.contains(result->id)) {
				parent->instances.pushBack(result->id);
			}
		}

		*result = mat;
		return true;
	}

	bool32 deserializeMaterialInstance(const char* path, Library* lib, MaterialInstance* result) {
		auto root = FileIO::loadYamlfile(path);
		return deserializeMaterialInstance(root, lib, result, path);
	}

	bool32 serializeMaterialInstance(YAML::Emitter& out, MaterialInstance* materialinstance) 
	{
		out << YAML::BeginMap;
		auto resetpoint = Engine::ArenaAllocator::getResetPoint();
		out << YAML::Key << "ParentMaterial" << YAML::Value << materialinstance->parent->id;
		out << YAML::Key << "UniformResources" << YAML::Value << YAML::BeginSeq;

		//serialize the uniform resources
		for (auto* res : materialinstance->resources.getAll<Engine::ArenaAllocator>()) {

			out << YAML::BeginMap;//resource
			if (res->type == UniformResourceType::UNIFORM_BUFFER) {
				out << YAML::Key << "Type" << YAML::Value << "UNIFORM_BUFFER";
				out << YAML::Key << "Binding" << YAML::Value << res->binding;
				out << YAML::Key << "Data" << YAML::Value << YAML::BeginSeq;

				for (uint32 i = 0; i < res->ubodescription.boundbuffers.getCapacity(); i++) {
					void* data;
					UniformBufferSubResource buffer = (UniformBufferSubResource)res->ubodescription.boundbuffers[i];
					GFX::mapBuffer(&data, buffer.base);
					out << YAML::Binary((unsigned char*)data, buffer.size);
				}
				out << YAML::EndSeq;
			}

			if (res->type == UniformResourceType::UNIFORM_SAMPLER) {
				out << YAML::Key << "Type" << YAML::Value << "UNIFORM_SAMPLER";
				out << YAML::Key << "Binding" << YAML::Value << res->binding;
				out << YAML::Key << "Textures" << YAML::Value << YAML::BeginSeq;
				for (uint32 i = 0; i < res->samplerdescription.boundimages.getCapacity(); i++) {
					void* data;
					TextureImage* image = (TextureImage*)res->samplerdescription.boundimages[i];
					if (image) {
						out << image->id;
					}
					else {
						out << 0;
					}
				}
				out << YAML::EndSeq;
			}
			out << YAML::EndMap;//resource
		}
		out << YAML::EndSeq;

		out << YAML::EndMap;
		return true;
	}

	bool32 serializeMaterialInstance(const char* path, MaterialInstance* material)
	{
		YAML::Emitter out;
		serializeMaterialInstance(out, material);
		FileIO::writeYamlFile(out, path);
		return true;
	}

	bool32 compileMaterialFromBinaries(Material* material, Platform::GFX::RenderpassDescription intendedrenderpass) {
		return compileMaterialFromBinaries(material, intendedrenderpass, Scene::descriptorsetlayout);
	}
	bool32 compileMaterialFromSource(Material* material, Platform::GFX::RenderpassDescription intendedrenderpass) {
		return compileMaterialFromSource(material, intendedrenderpass, Scene::descriptorsetlayout);
	}

}