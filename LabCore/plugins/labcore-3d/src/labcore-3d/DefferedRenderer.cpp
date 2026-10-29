#include "DefferredRenderer.h"

#include <LabCore/LabCoreAPI.h>

using namespace PH::Platform;

namespace PH::LabCore3D {

	//globals
	DefferedRenderer renderer = {};
	PH::Platform::GFX::DescriptorSetLayout DefferedRenderer::scenedescriptorlayout = PH_GFX_NULL;
	PH::Platform::GFX::RenderpassDescription DefferedRenderer::geometryrenderpass = PH_GFX_NULL;
	PH::Platform::GFX::RenderpassDescription DefferedRenderer::lightingrenderpass = PH_GFX_NULL;

	PH::Platform::GFX::DescriptorSetLayout DefferedRenderer::createSceneDescriptorLayout() {

		PH::Platform::GFX::DescriptorSetLayoutCreateinfo ci;

		Platform::GFX::DescriptorBinding cv;
		cv.binding = 0;
		cv.descriptorcount = 1;
		cv.shaderstages = Platform::GFX::SHADER_STAGE_VERTEX_BIT;
		cv.type = Platform::GFX::DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		ci.bindings = { &cv, 1 };

		PH::Platform::GFX::DescriptorSetLayout result;

		PH::Platform::GFX::createDescriptorSetLayouts(&ci, &result, 1);
		return result;
	}
	
	GFX::Texture createAlbedoTarget(sizeptr width, sizeptr height) {
		
		PH::Platform::GFX::TextureCreateInfo ai{};
		ai.data = nullptr;
		ai.format = GFX::FORMAT_R8G8B8A8_UNORM;
		ai.height = height;
		ai.width = width;
		ai.usage = GFX::IMAGE_USAGE_SAMPLED_BIT | GFX::IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		GFX::Texture texture;

		if (!GFX::createTextures(&ai, &texture, 1)) {
			Engine::ERR << "Failed to create Albedo render target!\n";
			return PH_GFX_NULL;
		}

		return texture;
	}

	GFX::Texture createDepthTarget(sizeptr width, sizeptr height) {

		PH::Platform::GFX::TextureCreateInfo ai;
		ai.ArrayLayers = 1;
		ai.data = nullptr;
		ai.format = GFX::FORMAT_D24_UNORM_S8_UINT;
		ai.height = height;
		ai.width = width;
		ai.usage = GFX::IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		GFX::Texture texture;

		if (!GFX::createTextures(&ai, &texture, 1)) {
			Engine::ERR << "Failed to create Depth render target!\n";
			return PH_GFX_NULL;
		}

		return texture;
	}

	GFX::Texture createNormalsTarget(sizeptr width, sizeptr height) {
		PH::Platform::GFX::TextureCreateInfo ai;
		ai.ArrayLayers = 1;
		ai.data = nullptr;
		ai.format = GFX::FORMAT_R16G16B16A16_SFLOAT;
		ai.height = height;
		ai.width = width;
		ai.usage = GFX::IMAGE_USAGE_COLOR_ATTACHMENT_BIT | GFX::IMAGE_USAGE_SAMPLED_BIT;

		GFX::Texture texture;

		if (!GFX::createTextures(&ai, &texture, 1)) {
			Engine::ERR << "Failed to create Depth render target!\n";
			return PH_GFX_NULL;
		}

		return texture;
	}

	PH::Platform::GFX::RenderpassDescription DefferedRenderer::createLightingRendererpass() {

		Platform::GFX::RenderpassDescription renderpass;

		//albedo attachment
		Platform::GFX::AttachmentDescription lightcolorattachment;
		lightcolorattachment.initiallayout = Platform::GFX::IMAGE_LAYOUT_UNDEFINED;
		lightcolorattachment.finallayout = Platform::GFX::IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		lightcolorattachment.format = Platform::GFX::FORMAT_R8G8B8A8_UNORM;
		lightcolorattachment.loadop = Platform::GFX::ATTACHMENT_LOAD_OP_CLEAR;
		lightcolorattachment.storeop = Platform::GFX::ATTACHMENT_STORE_OP_STORE;

		Platform::GFX::AttachmentDescription attachmentdescriptions[] = { lightcolorattachment };

		Platform::GFX::AttachmentReference lightcolorattachmentref{};
		lightcolorattachmentref.attachmentindex = 0;
		lightcolorattachmentref.layout = Platform::GFX::IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		Platform::GFX::AttachmentReference colorattachmentrefs[] = {
			lightcolorattachmentref
		};

		Platform::GFX::SubPass subpass{};
		subpass.bindpoint = Platform::GFX::PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorattachments = { colorattachmentrefs, 1 };
		subpass.depthstencilattachment = nullptr;

		Platform::GFX::SubpassDependency dependency{};
		dependency.srcsubpass = GFX_SUBPASS_EXTERNAL;
		dependency.dstsubpass = 0;
		dependency.srcstagemask = PH::Platform::GFX::PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dststagemask = PH::Platform::GFX::PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependency.srcaccessmask = PH::Platform::GFX::ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dstaccessmask = PH::Platform::GFX::ACCESS_SHADER_READ_BIT;

		Platform::GFX::RenderpassDescriptionCreateinfo renderpasscreate{};
		renderpasscreate.attachments = { attachmentdescriptions, ARRAY_LENGTH(attachmentdescriptions) };
		renderpasscreate.subpasses = { &subpass, 1 };
		renderpasscreate.dependencies = { &dependency, 1 };

		Platform::GFX::createRenderpassDescriptions(&renderpasscreate, &renderpass, 1);

		return renderpass;
	}

	GFX::RenderpassDescription DefferedRenderer::createGeometryRenderpass() {

		Platform::GFX::RenderpassDescription renderpass;

		//albedo attachment
		Platform::GFX::AttachmentDescription albedoattachment;
		albedoattachment.initiallayout = Platform::GFX::IMAGE_LAYOUT_UNDEFINED;
		albedoattachment.finallayout = Platform::GFX::IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		albedoattachment.format = Platform::GFX::FORMAT_R8G8B8A8_UNORM;
		albedoattachment.loadop = Platform::GFX::ATTACHMENT_LOAD_OP_CLEAR;
		albedoattachment.storeop = Platform::GFX::ATTACHMENT_STORE_OP_STORE;

		//normals
		Platform::GFX::AttachmentDescription normalsattachment{};
		normalsattachment.initiallayout = Platform::GFX::IMAGE_LAYOUT_UNDEFINED;
		normalsattachment.finallayout = Platform::GFX::IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		normalsattachment.format = Platform::GFX::FORMAT_R16G16B16A16_SFLOAT;
		normalsattachment.loadop = Platform::GFX::ATTACHMENT_LOAD_OP_CLEAR;
		normalsattachment.storeop = Platform::GFX::ATTACHMENT_STORE_OP_STORE;

		//depth
		Platform::GFX::AttachmentDescription depthattachment{};
		depthattachment.initiallayout = GFX::IMAGE_LAYOUT_UNDEFINED;
		depthattachment.finallayout = GFX::IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthattachment.format = GFX::FORMAT_D24_UNORM_S8_UINT;
		depthattachment.loadop = GFX::ATTACHMENT_LOAD_OP_CLEAR;
		depthattachment.storeop = GFX::ATTACHMENT_STORE_OP_DONT_CARE;

		Platform::GFX::AttachmentDescription attachmentdescriptions[] = { albedoattachment, normalsattachment, depthattachment };

		Platform::GFX::AttachmentReference albedoattachmentref{};
		albedoattachmentref.attachmentindex = 0;
		albedoattachmentref.layout = Platform::GFX::IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		Platform::GFX::AttachmentReference normalsattachmentref{};
		normalsattachmentref.attachmentindex = 1;
		normalsattachmentref.layout = Platform::GFX::IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		Platform::GFX::AttachmentReference depthattachmentref{};
		depthattachmentref.attachmentindex = 2;
		depthattachmentref.layout = Platform::GFX::IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		Platform::GFX::AttachmentReference gbufferattachmentrefs[] = {
			albedoattachmentref, normalsattachmentref
		};

		Platform::GFX::SubPass subpass{};
		subpass.bindpoint = Platform::GFX::PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorattachments = { gbufferattachmentrefs, 2 };
		subpass.depthstencilattachment = &depthattachmentref;

		Platform::GFX::SubpassDependency dependency{};
		dependency.srcsubpass = GFX_SUBPASS_EXTERNAL;
		dependency.dstsubpass = 0;
		dependency.dststagemask = PH::Platform::GFX::PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcstagemask = PH::Platform::GFX::PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstaccessmask = PH::Platform::GFX::ACCESS_MEMORY_WRITE_BIT;
		dependency.srcaccessmask = PH::Platform::GFX::ACCESS_MEMORY_WRITE_BIT;

		Platform::GFX::RenderpassDescriptionCreateinfo renderpasscreate{};
		renderpasscreate.attachments = { attachmentdescriptions, ARRAY_LENGTH(attachmentdescriptions) };
		renderpasscreate.subpasses = { &subpass, 1 };
		renderpasscreate.dependencies = { &dependency, 1 };

		Platform::GFX::createRenderpassDescriptions(&renderpasscreate, &renderpass, 1);

		return renderpass;

	}

	GFX::Framebuffer createGeometryFrameBuffer(GFX::Texture albedo, GFX::Texture normals, GFX::Texture depth, sizeptr width, sizeptr height) {
		GFX::FramebufferCreateInfo fbi;
		GFX::Texture attachments[] = { albedo, normals, depth };
		fbi.attachments = { attachments, 3 };
		fbi.width = width;
		fbi.height = height;

		fbi.renderpassdescription = DefferedRenderer::geometryrenderpass;

		GFX::Framebuffer r;

		if (!GFX::createFramebuffers(&fbi, &r, 1)) {
			Engine::ERR << "failed to create deffered renderer framebuffers\n";
		}

		return r;
	}

	GBuffer createGBuffer(sizeptr width, sizeptr height) {
		GBuffer g;
		g.widhth = width;
		g.height = height;

		g.albedo = createAlbedoTarget(width, height);
		g.depth = createDepthTarget(width, height);
		g.normal = createNormalsTarget(width, height);

		g.fb = createGeometryFrameBuffer(g.albedo, g.normal, g.depth, width, height);

		return g;
	}

	GFX::Texture createLightAttachment(sizeptr width, sizeptr height) {

		PH::Platform::GFX::TextureCreateInfo ai{};
		ai.data = nullptr;
		ai.format = GFX::FORMAT_R8G8B8A8_UNORM;
		ai.height = height;
		ai.width = width;
		ai.usage = GFX::IMAGE_USAGE_SAMPLED_BIT | GFX::IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		GFX::Texture texture;

		if (!GFX::createTextures(&ai, &texture, 1)) {
			Engine::ERR << "Failed to create Albedo render target!\n";
			return PH_GFX_NULL;
		}

		return texture;
	}

	GFX::Framebuffer createLightBuffer(GFX::Texture light, sizeptr width, sizeptr height) {
		GFX::FramebufferCreateInfo fbi;
		GFX::Texture attachments[] = { light };
		fbi.attachments = { attachments, 1 };
		fbi.width = width;
		fbi.height = height;

		fbi.renderpassdescription = DefferedRenderer::lightingrenderpass;

		GFX::Framebuffer r;

		if (!GFX::createFramebuffers(&fbi, &r, 1)) {
			Engine::ERR << "failed to create deffered renderer framebuffers\n";
		}

		return r;
	}


	DefferedRenderer DefferedRenderer::create(const DefferedRendererCreateInfo& dci) {

		DefferedRenderer r;

		r.albedoclearcolor = dci.clearcolor;
		r.viewport = dci.viewport;

		r.gbuffer = createGBuffer(dci.gbufferwidth, dci.gbufferheight);

		r.cameravariables = Engine::DynamicGPUBuffer<CameraVariables>::create(1, Platform::GFX::BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		r.instancebuffer = Engine::DynamicGPUBuffer<MeshInstance>::create(dci.maxinstancecount, Platform::GFX::BUFFER_USAGE_VERTEX_BUFFER_BIT);

		r.lightpassattachment = createLightAttachment(dci.gbufferwidth, dci.gbufferheight);
		r.lightpassbuffer = createLightBuffer(r.lightpassattachment, dci.gbufferwidth, dci.gbufferheight);

		//should be a more simple function in engine
		PH::Platform::GFX::DescriptorSetCreateinfo ci;
		ci.dynamic = true;
		ci.layout = r.scenedescriptorlayout;
		PH::Platform::GFX::createDescriptorSets(&ci, &r.scenedescriptorset, 1);

		PH::Platform::GFX::DescriptorBufferInfo bi;
		bi.buffer = r.cameravariables.buffer;
		bi.offset = 0;
		bi.range = sizeof(CameraVariables);

		PH::Platform::GFX::DescriptorWrite wr;

		wr.dstset = r.scenedescriptorset;
		wr.arrayelement = 0;
		wr.dstbinding = 0;
		wr.descriptorcount = 1;
		wr.dynamicwrite = false;
		wr.descriptorinfo = &bi;
		wr.type = Platform::GFX::DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		PH::Platform::GFX::updateDescriptorSets(&wr, 1);

		return r;
	}

	DefferedRenderer DefferedRenderer::create() {
		DefferedRendererCreateInfo ci;
		ci.clearcolor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
		ci.gbufferheight = 1080;
		ci.gbufferwidth = 1920;
		ci.maxinstancecount = 4096;
		ci.viewport = { 0.0f, 0.0f, 1920.0f, 1080.0f };

		return create(ci);
	}

	void DefferedRenderer::beginGeometryPass() {

		//begin the scene render pass
		Platform::GFX::RenderpassbeginInfo renderpassbegin{};
		renderpassbegin.description = geometryrenderpass;
		renderpassbegin.framebuffer = gbuffer.fb;
		renderpassbegin.renderarea = { (uint32)gbuffer.widhth, (uint32)gbuffer.height };

		glm::vec4 cc = albedoclearcolor;
		glm::vec4 ncc = { 0.0f, 0.0f, 0.0f, 1.0f };

		Platform::GFX::ClearValue clearvalues[3];
		clearvalues[0] = { cc.r, cc.g, cc.b, cc.a };
		clearvalues[1] = { ncc.r, ncc.g, ncc.b, ncc.a };
		clearvalues[2].depthStencil.depth = 1.0f;
		clearvalues[2].depthStencil.stencil = 0.0f;

		renderpassbegin.clearvalues = { clearvalues, ARRAY_LENGTH(clearvalues) };

		Platform::GFX::beginRenderpass(&renderpassbegin);

		Platform::GFX::setViewports(&this->viewport, 1);

		Platform::GFX::Scissor scissor;
		scissor.width = (uint32)this->viewport.width;
		scissor.height = (uint32)this->viewport.height;
		scissor.x = 0;
		scissor.y = 0;
		Platform::GFX::setScissors(&scissor, 1);

		begin();
	}

	void DefferedRenderer::endGeometryPass() {
		Platform::GFX::endRenderpass();
	}

	void beginInternalRenderpass(DefferedRenderer* this_, GFX::RenderpassDescription renderpass, GFX::Framebuffer framebuffer) {
		GFX::RenderpassbeginInfo bi{};

		glm::vec4 cc = { 0.3f, 0.3f, 0.3f, 1.0f };

		Platform::GFX::ClearValue clearvalues[1];
		clearvalues[0] = { cc.r, cc.g, cc.b, cc.a };

		bi.clearvalues = { clearvalues, 1 };

		bi.description = renderpass;
		bi.framebuffer = framebuffer;
		bi.renderarea.x = 0.0f;
		bi.renderarea.y = 0.0f;
		bi.renderarea.width = this_->gbuffer.widhth;
		bi.renderarea.height = this_->gbuffer.height;

		GFX::beginRenderpass(&bi);

		Platform::GFX::Viewport viewport;
		viewport.width = (real32)this_->gbuffer.widhth;
		viewport.height = (real32)this_->gbuffer.height;
		viewport.x = 0;
		viewport.y = 0;

		Platform::GFX::setViewports(&viewport, 1);

		Platform::GFX::Scissor scissor;
		scissor.width = (uint32)this_->viewport.width;
		scissor.height = (uint32)this_->viewport.height;
		scissor.x = 0;
		scissor.y = 0;
		Platform::GFX::setScissors(&scissor, 1);
	}

	void endInternalRenderpass() {
		GFX::endRenderpass();
	}

	void DefferedRenderer::applyLightpass(GraphicsPipeline* pipe) {
		beginInternalRenderpass(this, lightingrenderpass, lightpassbuffer);

		auto* renderer2d = LabCore::getRenderer2D();
		renderer2d->begin();

		renderer2d->pushProjection(glm::mat4(1.0f));
		renderer2d->pushView(glm::mat4(1.0f));

		renderer2d->pushGraphicsPipeline(pipe->internalpipeline, { &pipe->defaultuserdescriptorset, 1 });
		renderer2d->pushTexture(gbuffer.albedo);
		renderer2d->pushTexture(gbuffer.normal);

		renderer2d->drawQuadWithID({ 0.0f, 0.0f, 0.0f }, { 2.0f, 2.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 1);

		renderer2d->end();
		endInternalRenderpass();
	}

	void DefferedRenderer::applyPostProcess() {
	}


	void DefferedRenderer::output(Engine::Display* target, OutputSource source) {
		auto* renderer2d = LabCore::getRenderer2D();

		beginInternalRenderpass(this, target->renderpass, target->fb);

		renderer2d->begin();

		auto default2dpipe = LabCore::getDefault2DGraphicsPipeline();

		renderer2d->pushGraphicsPipeline(default2dpipe);

		renderer2d->pushProjection(glm::mat4(1.0f));
		renderer2d->pushView(glm::mat4(1.0f));

		GFX::Texture srctexture = lightpassattachment;
		if (source == DEFFERED_ALBEDO) {
			srctexture = gbuffer.albedo;
		}

		if (source == DEFFERED_NORMAL) {
			srctexture = gbuffer.normal;
		}

		//just renderer albedo to the display for now!
		renderer2d->drawTexturedQuad({ 0.0f, 0.0f, 0.0f }, { 2.0f, 2.0f }, srctexture);

		renderer2d->end();

		endInternalRenderpass();
	}

	

	void DefferedRenderer::setProjection(glm::mat4 projection) {
		cameravariables[0].projection = projection;
	}

	void DefferedRenderer::setView(glm::mat4 view) {
		cameravariables[0].view = view;
	}

	void DefferedRenderer::begin() {
		instanceptr = 0;
	}


	void DefferedRenderer::end() {

	}

	void DefferedRenderer::drawMesh(Mesh* mesh, GraphicsPipeline* pipeline, const glm::mat4& transform) {
		if (!pipeline->isvalid) {
			return;
		}

		PH::Platform::GFX::bindGraphicsPipeline(pipeline->internalpipeline);

		//bind userset and scene set
		GFX::DescriptorSet descriptorsets[2] = { scenedescriptorset, pipeline->defaultuserdescriptorset };
		PH::Platform::GFX::bindDescriptorSets(descriptorsets, 0, 2);

		PH::Platform::GFX::bindIndexBuffer(mesh->indexbuffer, 0);

		sizeptr offsets[] = { 0, 0 };

		glm::mat4 tf = glm::transpose(transform);

		MeshInstance* instance = &instancebuffer[instanceptr];
		instance->materialindex = 0;
		instance->trow0 = tf[0];
		instance->trow1 = tf[1];
		instance->trow2 = tf[2];

		PH::Platform::GFX::Buffer vertexbuffers[] = { mesh->vertexbuffer, instancebuffer.buffer };
		PH::Platform::GFX::bindVertexBuffers(0, 2, vertexbuffers, offsets);

		PH::Platform::GFX::drawIndexed(mesh->indexcount, 1, 0, instanceptr);

		instanceptr++;
	}

	void DefferedRenderer::drawMesh(Mesh* mesh, GraphicsPipeline* pipeline) {
		drawMesh(mesh, pipeline, glm::mat4(1.0f));
	}


}