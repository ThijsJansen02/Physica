#include "BatchRenderer2D.h"
#include <Engine/shaders.h>
namespace PH::Engine {

	using namespace PH::Platform;

	glm::mat4 invertY() {
		return glm::mat4(
			{ 1.0f, 0.0f, 0.0f, 0.0f },
			{ 0.0f, -1.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 1.0f, 0.0 },
			{ 0.0f, 0.0f, 0.0f, 1.0f });
	}

#define MAX_TEXTURE_COUNT 32

	Platform::GFX::DescriptorSetLayout BatchRenderer2D::globaldescriptorsetlayout = PH_GFX_NULL;
	Platform::GFX::DescriptorSetLayout BatchRenderer2D::createGlobalDescriptorsetLayout() {

		GFX::DescriptorBinding texturebinding;
		texturebinding.binding = 0;
		texturebinding.descriptorcount = MAX_TEXTURE_COUNT;
		texturebinding.shaderstages = GFX::SHADER_STAGE_FRAGMENT_BIT;
		texturebinding.type = GFX::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		GFX::DescriptorSetLayoutCreateinfo i{};
		i.bindings = { &texturebinding, 1 };

		GFX::DescriptorSetLayout r;

		if (!GFX::createDescriptorSetLayouts(&i, &r, 1)) {
			return PH_GFX_NULL;
		}

		return r;
	}

	Platform::GFX::RenderpassDescription BatchRenderer2D::geometryrenderpass = PH_GFX_NULL;
	Platform::GFX::RenderpassDescription BatchRenderer2D::createGeometryRenderpass() {



		Platform::GFX::RenderpassDescription renderpass;

		//color attachment
		Platform::GFX::AttachmentDescription colorattachment;
		colorattachment.initiallayout = Platform::GFX::IMAGE_LAYOUT_UNDEFINED;
		colorattachment.finallayout = Platform::GFX::IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		colorattachment.format = Platform::GFX::FORMAT_R8G8B8A8_UNORM;
		colorattachment.loadop = Platform::GFX::ATTACHMENT_LOAD_OP_CLEAR;
		colorattachment.storeop = Platform::GFX::ATTACHMENT_STORE_OP_STORE;

		Platform::GFX::AttachmentDescription depthattachment{};
		depthattachment.initiallayout = GFX::IMAGE_LAYOUT_UNDEFINED;
		depthattachment.finallayout = GFX::IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthattachment.format = GFX::FORMAT_D24_UNORM_S8_UINT;
		depthattachment.loadop = GFX::ATTACHMENT_LOAD_OP_CLEAR;
		depthattachment.storeop = GFX::ATTACHMENT_STORE_OP_STORE;

		Platform::GFX::AttachmentDescription attachmentdescriptions[] = { colorattachment, depthattachment };

		Platform::GFX::AttachmentReference colorattachmentref{};
		colorattachmentref.attachmentindex = 0;
		colorattachmentref.layout = Platform::GFX::IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		Platform::GFX::AttachmentReference depthattachementref;
		depthattachementref.attachmentindex = 1;
		depthattachementref.layout = GFX::IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		Platform::GFX::AttachmentReference colorattachmentrefs [] = {
			colorattachmentref
		};

		Platform::GFX::SubPass subpass{};
		subpass.bindpoint = Platform::GFX::PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorattachments = { colorattachmentrefs, 1 };
		subpass.depthstencilattachment = &depthattachementref;

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

	//have to be called at the begin of the frame and at the end of the frame
	void BatchRenderer2D::beginFrame() {
		instance_begin = 0;
		instance_end = 0;

		camera.projection = invertY();
		camera.view = glm::mat4(1.0f);
	}

	void BatchRenderer2D::endFrame() {

	}

	//begins a new instance
	void BatchRenderer2D::begin() {
		instance_begin = instance_end;
		textures.clear();
		textures.pushBack(nulltexture);
	}

	void BatchRenderer2D::end() {

	}

	//if framebuffer is specified, begingeometrypass uses that framebuffer otherwise it will use the internal fb.
	void BatchRenderer2D::beginGeometryPass(bool32 clearcolor, bool32 cleardepth, Framebuffer* framebuffer) {
		if (!framebuffer) {
			framebuffer = &internalframebuffer;
		}

		GFX::RenderpassbeginInfo rpbi{};

		GFX::ClearValue c[2];
		c[0].color = {0.0, 0.0, 0.0, 1.0};
		c[1].depthStencil = { 1.0f, 0 };

		rpbi.clearvalues = { c, 2 };
		rpbi.framebuffer = framebuffer->fb;
		rpbi.renderarea = { (uint32)framebuffer->width, (uint32)framebuffer->height };
		rpbi.description = PH_GFX_NULL;
		GFX::beginRenderpass(&rpbi);

		Platform::GFX::setViewports(&this->viewport, 1);

		Platform::GFX::Scissor scissor;
		scissor.width = (uint32)this->viewport.width;
		scissor.height = (uint32)this->viewport.height;
		scissor.x = 0;
		scissor.y = 0;
		Platform::GFX::setScissors(&scissor, 1);

		begin();
	}

	void BatchRenderer2D::pushView(glm::mat4 view) {
		end();
		flush();
		begin();

		camera.view = view;
	}

	void BatchRenderer2D::pushProjection(glm::mat4 proj) {
		end();
		flush();
		begin();

		camera.projection = invertY() * proj;
	}

	void BatchRenderer2D::pushPipeline(GFX::GraphicsPipeline pipe) {
		if (currentpipeline != pipe) {
			end();
			flush();
			begin();
		}
		currentpipeline = pipe;
	}

	void BatchRenderer2D::drawRectangle(const glm::vec3& position, real32 rotation, const glm::vec2& size, const glm::vec4 color) {

		pushPipeline(quadshader);

		real32 cosp = cosf(rotation);
		real32 sinp = sinf(rotation);

		glm::mat2 scalerot = glm::mat2{ {size.x, 0.0f}, {0.0f, size.y} } * glm::mat2({ cosp, sinp }, { -sinp, cosp });

		drawQuad(position, scalerot, color, 0, 0);
	}

	uint32 BatchRenderer2D::pushTexture(GFX::Texture texture) {

		for (int32 i = ((int32)textures.getCount() - 1); i >= 0; i--) {
			if (textures[i] == texture) {
				return i;
			}
		}

		textures.pushBack(texture);
		PH_DEBUG_ASSERT(textures.getCount() <= MAX_TEXTURE_COUNT, "to much destinct textures used!");
		return textures.getCount() - 1;
	}

	void BatchRenderer2D::drawRectangle(const glm::vec3& position, real32 rotation, const glm::vec2& scale, GFX::Texture texture, const glm::vec4 color) {

		uint32 i = pushTexture(texture);

		real32 cosp = cosf(rotation);
		real32 sinp = sinf(rotation);

		glm::mat2 scalerot = glm::mat2{ {scale.x, 0.0f}, {0.0f, scale.y} } * glm::mat2({ cosp, sinp }, { -sinp, cosp });

		drawQuad(position, scalerot, color, i, 0);
	}

	void BatchRenderer2D::drawCircle(const glm::vec3& position, real32 rotation, const glm::vec2& radius, const glm::vec4 color) {
		
		pushPipeline(circleshader);

		real32 cosp = cosf(rotation);
		real32 sinp = sinf(rotation);

		glm::mat2 scalerot = glm::mat2{ {radius.x * 2.0f, 0.0f}, {0.0f, radius.y * 2.0f} } * glm::mat2({ cosp, sinp }, { -sinp, cosp });
		
		drawQuad(position, scalerot, color, 0, 0);
	}

	void BatchRenderer2D::drawLine(const glm::vec2& v1, const glm::vec2& v2, real32 depth, real32 thickness, const glm::vec4& color) {
		
		pushPipeline(lineshader);

		glm::vec2 midpoint = (v1 + v2) / 2.0f;
		glm::vec2 v1v2 = v2 - v1;
		glm::vec2 tangent = glm::normalize(v1v2);
		glm::vec2 v_x = v1v2 + tangent * thickness * 2.0f;
		glm::vec2 bitangent = { -tangent.y, tangent.x };
		glm::vec2 v_y = bitangent * thickness * 2.0f;

		glm::mat2 scalerot = glm::mat2(v_x, v_y);

		drawQuad(glm::vec3(midpoint, depth), scalerot, color, 0, 0);
	}

	void BatchRenderer2D::drawQuad(const glm::vec3& position, const glm::mat2& rotscale, const glm::vec4& color, uint32 matindex, uint32 objectid) {
		ColoredQuadInstance inst{};
		inst.color = color;
		inst.position = position;
		inst.scalerot = rotscale;
		inst.objectid = objectid;
		inst.textureindex = matindex;

		instancebuffer[instance_end++] = inst;
	}

	void updateGlobalDescriptorset(GFX::DescriptorSet set, Base::Array<GFX::Texture> textures) {
		Engine::ArrayList<GFX::DescriptorImageInfo> writes = Engine::ArrayList<GFX::DescriptorImageInfo>::create();
		for (auto t : textures) {
			GFX::DescriptorImageInfo ii;
			ii.texture = t;
			writes.push(ii);
		}

		GFX::DescriptorWrite wr;
		wr.arrayelement = 0;
		wr.descriptorcount = writes.getCount();
		wr.descriptorinfo = writes.raw();
		wr.dstbinding = 0;
		wr.dstset = set;
		wr.dynamicwrite = true;
		wr.type = GFX::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		
		GFX::updateDescriptorSets(&wr, 1);

		Engine::ArrayList<GFX::DescriptorImageInfo>::destroy(&writes);
	}

	//can be called manually when the user wants to flush the geometry to the fb, is called in endgeometrypass
	void BatchRenderer2D::flush() {

		sizeptr drawcount = instance_end - instance_begin;
		if (drawcount <= 0) {
			return;
		}

		PH_DEBUG_ASSERT(currentpipeline != PH_GFX_NULL, "currentpipeline is not yet set");

		GFX::bindGraphicsPipeline(currentpipeline);

		updateGlobalDescriptorset(globaldescriptorset, textures.getArray());
		GFX::bindDescriptorSets(&globaldescriptorset, 0, 1);

		sizeptr indexcount = 6;
		sizeptr firstindex = 0;
		sizeptr firstinstance = 0;

		GFX::bindIndexBuffer(quadindexbuffer.buffer, 0);

		GFX::Buffer buffers[] = { quadvertexbuffer.buffer, instancebuffer.buffer };
		sizeptr offsets[] = { 0, instance_begin * sizeof(BatchRenderer2D::ColoredQuadInstance) };

		GFX::pushConstants(currentpipeline, 0, sizeof(BatchRenderer2D::Camera), &camera, GFX::SHADER_STAGE_VERTEX_BIT);

		GFX::bindVertexBuffers(0, 2, buffers, offsets);
		GFX::drawIndexed(indexcount, drawcount, firstindex, firstinstance);
	}

	void BatchRenderer2D::endGeometryPass() {
		end();
		flush();

		GFX::endRenderpass();
	}

	void BatchRenderer2D::output(Engine::Display* output) {
		GFX::ImageToImageBlitInfo info{};
		info.dst = output->colorattachment;
		info.src = internalframebuffer.colorattachment;
		info.dstextent = { (uint32)output->viewport.x, (uint32)output->viewport.y };
		info.srcextent = { (uint32)viewport.width, (uint32)viewport.height };
		info.dstoffset = { 0, 0 };
		info.srcoffset = { 0, 0 };

		GFX::blitImageToImage(info);
	}

	BatchRenderer2D::Framebuffer createInternalFramebuffer(sizeptr width, sizeptr height) {
		BatchRenderer2D::Framebuffer r;

		PH::Platform::GFX::TextureCreateInfo cci{};
		cci.ArrayLayers = 1;
		cci.format = GFX::FORMAT_R8G8B8A8_UNORM;
		cci.data = nullptr;
		cci.viewtype = GFX::ImageViewType::IMAGE_VIEW_TYPE_2D;
		cci.width = width;
		cci.height = height;
		cci.usage = GFX::IMAGE_USAGE_TRANSFER_SRC_BIT | GFX::IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		PH::Platform::GFX::TextureCreateInfo dci{};
		dci.ArrayLayers = 1;
		dci.format = GFX::FORMAT_D24_UNORM_S8_UINT;
		dci.data = nullptr;
		dci.viewtype = GFX::ImageViewType::IMAGE_VIEW_TYPE_2D;
		dci.width = width;
		dci.height = height;
		dci.usage = GFX::IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		PH::Platform::GFX::TextureCreateInfo ci[] = { cci, dci };

		GFX::Texture attachments[2] = {};
		GFX::createTextures(ci, attachments, 2);

		GFX::FramebufferCreateInfo fbci;
		fbci.attachments = { attachments, 2 };
		fbci.height = height;
		fbci.width = width;
		fbci.renderpassdescription = BatchRenderer2D::geometryrenderpass;

		r.colorattachment = attachments[0];
		r.depthattchment = attachments[1];

		r.width = width;
		r.height = height;

		GFX::createFramebuffers(&fbci, &r.fb, 1);

		return r;
	}

	PH::Platform::GFX::GraphicsPipeline BatchRenderer2D::createBatchPipeline(const char* srcpath, const char* binpath, Base::Array<GFX::DescriptorSetLayout> userlayouts, bool32 blend, bool32 depthtest) {
		
		Engine::Binaries b;
		if (!Engine::loadBinaries(&b, srcpath, binpath)) {
			return PH_GFX_NULL;
		}

		GFX::PushConstantRange pcr;
		pcr.offset = 0;
		pcr.shaderstages = GFX::SHADER_STAGE_VERTEX_BIT;
		pcr.size = sizeof(BatchRenderer2D::Camera);

		GFX::GraphicsPipeline p = Engine::createGraphicsPipelineFromBinaries(
			geometryrenderpass,
			b,
			{ &globaldescriptorsetlayout, 1 },
			{ (GFX::VertexInputAttributeDescription*)quadvertexinputattributes, ARRAY_LENGTH(quadvertexinputattributes) },
			{ (GFX::VertexInputBindingDescription*)quadvertexbindingdescription, ARRAY_LENGTH(quadvertexbindingdescription) },
			{ &pcr, 1},
			depthtest,
			blend
		);

		return p;
	}

	BatchRenderer2D::QuadVertex quadvertices[] = {
			{{-0.5f, -0.5f}, {0.0f, 0.0f}},
			{{ 0.5f, -0.5f}, {1.0f, 0.0f}},
			{{ 0.5f,  0.5f}, {1.0f, 1.0f}},
			{{-0.5f,  0.5f}, {0.0f, 1.0f}}
	};

	uint32 quadindices[] = {
		0, 1, 2,
		0, 2, 3
	};

	StaticGPUBuffer<BatchRenderer2D::QuadVertex> createVertexBuffer() {
		return StaticGPUBuffer<BatchRenderer2D::QuadVertex>::create({ quadvertices, ARRAY_LENGTH(quadvertices) }, GFX::BUFFER_USAGE_VERTEX_BUFFER_BIT);
	}

	StaticGPUBuffer<uint32> createIndexBuffer() {
		return StaticGPUBuffer<uint32>::create({ quadindices, ARRAY_LENGTH(quadindices) }, GFX::BUFFER_USAGE_INDEX_BUFFER_BIT);
	}
	
	GFX::Texture createNullTexture() {
		GFX::TextureCreateInfo ti{};
		uint32 data = 0xFFFFFFFF;

		ti.ArrayLayers = 1;
		ti.data = &data;
		ti.width = 1;
		ti.height = 1;
		ti.format = GFX::FORMAT_R8G8B8A8_UNORM;
		ti.viewtype = GFX::IMAGE_VIEW_TYPE_2D;
		ti.usage = GFX::IMAGE_USAGE_SAMPLED_BIT;
		
		GFX::Texture r;

		if (!GFX::createTextures(&ti, &r, 1)) {
			return PH_GFX_NULL;
		}

		return r;
	}

	GFX::DescriptorSet createGlobalDescriptorSet(GFX::DescriptorSetLayout globallayout, GFX::Texture nulltexture) {
		GFX::DescriptorSetCreateinfo ci{};
		ci.dynamic = true;
		ci.layout = globallayout;

		GFX::DescriptorSet r = PH_GFX_NULL;

		if (!GFX::createDescriptorSets(&ci, &r, 1)) {
			Engine::WARN << "failed to create global descriptorset!\n";
			return PH_GFX_NULL;
		}

		GFX::DescriptorImageInfo iis[MAX_TEXTURE_COUNT];
		for (uint32 i = 0; i < MAX_TEXTURE_COUNT; i++) {
			iis[i].texture = nulltexture;
		}
		
		GFX::DescriptorWrite wr;
		wr.arrayelement = 0;
		wr.descriptorcount = MAX_TEXTURE_COUNT;
		wr.dstbinding = 0;
		wr.dstset = r;
		wr.dynamicwrite = false;
		wr.type = GFX::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		wr.descriptorinfo = iis;

		GFX::updateDescriptorSets(&wr, 1);

		return r;
	}

	BatchRenderer2D BatchRenderer2D::create(const CreateInfo& create) {
		
		BatchRenderer2D r;
		r.currentpipeline = PH_GFX_NULL;
		r.fontshader = create.fontshader;
		r.lineshader = create.lineshader;
		r.quadshader = create.quadshader;
		r.circleshader = create.circleshader;

		r.quadindexbuffer = createIndexBuffer();
		r.quadvertexbuffer = createVertexBuffer();

		r.nulltexture = createNullTexture();
		r.globaldescriptorset = createGlobalDescriptorSet(r.globaldescriptorsetlayout, r.nulltexture);

		r.textures = Engine::ArrayList<GFX::Texture>::create(1);

		r.instancebuffer = DynamicGPUBuffer<ColoredQuadInstance>::create(create.instancebuffersize, GFX::BUFFER_USAGE_VERTEX_BUFFER_BIT);

		if (create.internalfb) {
			r.internalframebuffer = createInternalFramebuffer(create.internalwidth, create.internalheight);
		}

		r.viewport = {};

		return r;

		
	}

}