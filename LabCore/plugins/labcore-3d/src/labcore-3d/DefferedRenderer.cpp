#include "DefferredRenderer.h"
namespace PH::LabCore3D {

	PH::Platform::GFX::DescriptorSetLayout DefferedRenderer::scenedescriptorlayout = PH_GFX_NULL;

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

	
	DefferedRenderer DefferedRenderer::create(sizeptr maxinstancecount) {
		DefferedRenderer r;
		r.cameravariables = Engine::DynamicGPUBuffer<CameraVariables>::create(1, Platform::GFX::BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		r.instancebuffer = Engine::DynamicGPUBuffer<MeshInstance>::create(maxinstancecount, Platform::GFX::BUFFER_USAGE_VERTEX_BUFFER_BIT);

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
		return create(4096);
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

	DefferedRenderer renderer = {};

	void DefferedRenderer::end() {

	}

	void DefferedRenderer::drawMesh(Mesh* mesh, GraphicsPipeline* pipeline) {
		
		PH::Platform::GFX::bindGraphicsPipeline(pipeline->internalpipeline);

		PH::Platform::GFX::bindDescriptorSets(&scenedescriptorset, 0, 1);

		PH::Platform::GFX::bindIndexBuffer(mesh->indexbuffer, 0);

		sizeptr offsets[] = { 0, 0 };

		glm::mat4 tf = glm::transpose(glm::mat4(1.0f));

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


}