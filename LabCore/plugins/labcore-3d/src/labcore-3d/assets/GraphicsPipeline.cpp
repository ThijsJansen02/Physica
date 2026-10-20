
#include "GraphicsPipeline.h"
#include <Engine/Engine.h>

#include "../DefferredRenderer.h"

#include <Base/Datastructures/Stream.h>
#include <Engine/Rendering.h>

namespace PH::LabCore3D {

	using namespace PH::Platform;


	Platform::GFX::GraphicsPipeline create3DGraphicsPipelineFromBinaries(Platform::GFX::RenderpassDescription renderpass, Base::Array<uint8> vertsource, Base::Array<uint8> fragsource, Base::Array<Platform::GFX::DescriptorSetLayout> userlayouts) {

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
		pipelinecreate.vertexbindingdescriptions = { mesh3Dvertexbindingdescription, ARRAY_LENGTH(mesh3Dvertexbindingdescription) };
		pipelinecreate.vertexattributedescriptions = { mesh3Dvertexinputattributes, ARRAY_LENGTH(mesh3Dvertexinputattributes) };
		pipelinecreate.depthtest = false;

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

	bool32 writeBinariesToDisk(Base::Array<uint8> vertsrc, Base::Array<uint8> fragsrc, const char* binpath) {

		Base::Stream<Engine::Allocator> s = Base::Stream<Engine::Allocator>::create(fragsrc.count + vertsrc.count + sizeof(ShaderBinariesHeader));

		ShaderBinariesHeader h;
		h.vertbegin = sizeof(ShaderBinariesHeader);
		h.vertend = h.vertbegin + vertsrc.count;
		h.fragbegin = h.vertend;
		h.fragend = h.fragbegin + fragsrc.count;

		s << h << vertsrc << fragsrc;

		FileBuffer f;
		f.data = s.raw();
		f.size = s.getSize();

		return Platform::writeFile(f, binpath);
	}


	//reloads the graphics pipeline from glsl source
	bool32 loadGraphicsPipelineFromGLSLSource(GraphicsPipeline* pipe) {

		PH::Platform::FileBuffer f;

		if (!PH::Platform::loadFile(&f, pipe->srcpath.getC_Str())) {
			Engine::WARN << "failed to load source for graphics pipeline: " << pipe->srcpath.getC_Str();
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

		Platform::GFX::GraphicsPipeline p = create3DGraphicsPipelineFromBinaries(Engine::Display::defaultrenderpassdescription, vertbin.getArray(), fragbin.getArray(), { &DefferedRenderer::scenedescriptorlayout, 1 });
		
		if (p == PH_GFX_NULL) {
			return false;
		}

		if (!pipe->bindir.isEmpty()) {
			writeBinariesToDisk(vertbin.getArray(), fragbin.getArray(), pipe->bindir.getC_Str());
		}

		//if all went well replace old pipeline with new one
		if (pipe->internalpipeline != PH_GFX_NULL) {
			//add graphics pipeline delete;
		}

		Engine::DynamicArray<uint8>::destroy(&vertbin);
		Engine::DynamicArray<uint8>::destroy(&fragbin);

		pipe->internalpipeline = p;
		return true;
	}
	
	struct Binaries {
		Engine::DynamicArray<uint8> vertsrc;
		Engine::DynamicArray<uint8> fragsrc;
	};

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

	bool32 loadGraphicsPipelineFromBinaries(GraphicsPipeline* pipe) {
		if (!pipe->bindir.isEmpty()) {
			Binaries b;
			if (loadBinariesFromDisk(pipe->bindir.getC_Str(), &b)) {
				Platform::GFX::GraphicsPipeline p = create3DGraphicsPipelineFromBinaries(Engine::Display::defaultrenderpassdescription, b.vertsrc.getArray(), b.fragsrc.getArray(), { &DefferedRenderer::scenedescriptorlayout, 1 });

				if (p == PH_GFX_NULL) {
					return false;
				}

				//if all went well replace old pipeline with new one
				if (pipe->internalpipeline != PH_GFX_NULL) {
					//add graphics pipeline delete;
				}

				Engine::DynamicArray<uint8>::destroy(&b.fragsrc);
				Engine::DynamicArray<uint8>::destroy(&b.vertsrc);

				pipe->internalpipeline = p;
				return true;
			}
		}
		return false;
	}

	//loads the graphics pipeline from binaries if they exist, and reloads them from source if not
	bool32 loadGraphicsPipeline(GraphicsPipeline* pipe) {

		if (loadGraphicsPipelineFromBinaries(pipe)) {
			return true;
		}


		if (loadGraphicsPipelineFromGLSLSource(pipe)) {
			return true;
		}

		//if neither source nor binaries compile return false
		return false;
	}

	GraphicsPipeline createGraphicsPipeline(const char* sourcepath, const char* binpath) {
		
		GraphicsPipeline pl{};
		pl.internalpipeline = PH_GFX_NULL;
		pl.bindir = Engine::String::create(binpath);
		pl.srcpath = Engine::String::create(sourcepath);
		pl.type = PIPELINE_TYPE_3D_DEFFERED_MATERIAL;

		loadGraphicsPipeline(&pl);

		return pl;

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