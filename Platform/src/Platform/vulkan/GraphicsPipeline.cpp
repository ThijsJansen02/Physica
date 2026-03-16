#include "vulkanAPI.h"
#include "shaderc/shaderc.hpp"

namespace PH::Vulkan {

	PH::bool32 createShaders(
		VulkanAppContext* context, 
		PH::Platform::GFX::ShaderCreateinfo* createinfos, 
		PH::Platform::GFX::Shader* shaders, 
		PH::uint32 count
	) {

		bool32 resultstatus = true;

		for (uint32 i = 0; i < count; i++) {
			const PH::Platform::GFX::ShaderCreateinfo& createinfo = createinfos[i];

			if (createinfo.sourcetype == PH::Platform::GFX::ShaderSourceType::VULKAN_GLSL) {

				shaderc::Compiler compiler;
				shaderc::CompileOptions options;
				options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
				const bool optimize = true;
				if (optimize)
					options.SetOptimizationLevel(shaderc_optimization_level_performance);


				std::string sourcecode = (const char*)createinfo.sourcecode;

				shaderc_shader_kind kind{};
				switch (createinfo.stage) {
				case Platform::GFX::SHADER_STAGE_FRAGMENT_BIT:
					kind = shaderc_shader_kind::shaderc_fragment_shader;
					break;
				case Platform::GFX::SHADER_STAGE_VERTEX_BIT:
					kind = shaderc_shader_kind::shaderc_vertex_shader;
					break;
				}

				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(sourcecode, kind, "ref");
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					WARN << "Shader compilation failed!\n";
					WARN << module.GetErrorMessage().c_str() << "\n";
					resultstatus = false;
					continue;
				}

				sizeptr size = (module.cend() - module.cbegin()) * sizeof(uint32);
				
				if (createinfo.chachedir) {
					Platform::FileBuffer file;
					file.data = (void*)module.cbegin();
					file.size = size;

					PH::Platform::writeFile(file, createinfo.chachedir);
				}

				Platform::GFX::ShaderCreateinfo createfrombin{};
				createfrombin.chachedir = nullptr;
				createfrombin.size = size;
				createfrombin.sourcecode = (void*)module.cbegin();
				createfrombin.stage = createinfo.stage;
				createfrombin.sourcetype = Platform::GFX::ShaderSourceType::VULKAN_BINARIES;

				createShaders(context, &createfrombin, &shaders[i], 1);
			}


			if (createinfo.sourcetype == PH::Platform::GFX::ShaderSourceType::VULKAN_BINARIES) {
				VkShaderModuleCreateInfo vkcreateinfo{};
				vkcreateinfo.pCode = (uint32*)createinfo.sourcecode;
				vkcreateinfo.codeSize = createinfo.size;
				vkcreateinfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			
				VkShaderModule vkshader{};
				if (vkCreateShaderModule(context->device, &vkcreateinfo, nullptr, &vkshader) != VK_SUCCESS) {
					WARN << "failed to create Shader module! \n";
					resultstatus = false;
					continue;
				}

				Shader result{};
				result.vkshadermodule = vkshader;
				result.shaderstage = convertShaderStageFlags(createinfo.stage);

				shaders[i] = context->shaders.add(result);
			}
		}
		return resultstatus;
	}

	PH_GFX_CREATE_SHADERS(createShaders) {
		return createShaders(context_s, createinfos, shaders, count);
	}

	bool32 destroyShaders(VulkanAppContext* context, PH::Platform::GFX::Shader* shaders, uint32 count) {
		for (uint32 i = 0; i < count; i++) {
			vkDestroyShaderModule(context->device, context->shaders[i].vkshadermodule, nullptr);
		}
		return true;
	}

	PH_GFX_DESTROY_SHADERS(destroyShaders) {
		return destroyShaders(context_s, shaders, count);
	}

#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(*arr))

	PH::bool32 createGraphicsPipelines(
		VulkanAppContext* context, 
		PH::Platform::GFX::GraphicsPipelineCreateinfo* createinfos, 
		PH::Platform::GFX::GraphicsPipeline* graphicspipelines, 
		PH::uint32 count
	) {
		PH::Base::MemoryArenaReset resetpoint = ArenaAllocator::getResetPoint();


		for (uint32 i = 0; i < count; i++) {
			const auto& createinfo = createinfos[i];

			auto shaderstages = PH::Base::ArrayList<VkPipelineShaderStageCreateInfo, ArenaAllocator>::create(createinfo.shaderstages.count);

			for (PH::Platform::GFX::Shader shaderid : createinfo.shaderstages) {

				Shader* shader = &context->shaders[shaderid];

				VkPipelineShaderStageCreateInfo stagecreateinfo{};
				stagecreateinfo.module = shader->vkshadermodule;
				stagecreateinfo.stage = (VkShaderStageFlagBits)shader->shaderstage;
				stagecreateinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				stagecreateinfo.pName = "main";

				shaderstages.pushBack(stagecreateinfo);
			}

			VkDynamicState dynamicstates[] = {
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR
			};

			VkPipelineDynamicStateCreateInfo dynamicstate{};
			dynamicstate.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicstate.dynamicStateCount = ARRAY_LENGTH(dynamicstates);
			dynamicstate.pDynamicStates = dynamicstates;

			auto inputattributes = PH::Base::ArrayList<VkVertexInputAttributeDescription, ArenaAllocator>::create(createinfo.vertexattributedescriptions.count);

			for (auto& attribute : createinfo.vertexattributedescriptions) {

				VkVertexInputAttributeDescription vkattribute;
				vkattribute.binding = attribute.binding;
				vkattribute.format = convertFormat(attribute.format);
				vkattribute.location = attribute.location;
				vkattribute.offset = attribute.offset;

				inputattributes.pushBack(vkattribute);
			}

			auto inputbindings = PH::Base::ArrayList<VkVertexInputBindingDescription, ArenaAllocator>::create(createinfo.vertexbindingdescriptions.count);

			for (auto& binding : createinfo.vertexbindingdescriptions) {

				VkVertexInputBindingDescription vkbinding{};
				vkbinding.binding = binding.binding;
				vkbinding.inputRate = convertVertexInputrate(binding.inputrate);
				vkbinding.stride = binding.stride;

				inputbindings.pushBack(vkbinding);
			}


			VkPipelineVertexInputStateCreateInfo vertexinputinfo{};
			vertexinputinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexinputinfo.vertexBindingDescriptionCount = inputbindings.getCount();
			vertexinputinfo.pVertexBindingDescriptions = inputbindings.raw(); // Optional
			vertexinputinfo.vertexAttributeDescriptionCount = inputattributes.getCount();
			vertexinputinfo.pVertexAttributeDescriptions = inputattributes.raw(); // Optional

			VkPipelineInputAssemblyStateCreateInfo inputassembly{};
			inputassembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputassembly.topology = convertPrimitiveTopology(createinfo.topology);
			inputassembly.primitiveRestartEnable = VK_FALSE;

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)context->swapchainextent.width;
			viewport.height = (float)context->swapchainextent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = context->swapchainextent;

			VkPipelineViewportStateCreateInfo viewportstate{};
			viewportstate.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportstate.viewportCount = 1;
			viewportstate.scissorCount = 1;

			VkPipelineRasterizationStateCreateInfo rasterizer{};
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.depthClampEnable = VK_FALSE;

			rasterizer.rasterizerDiscardEnable = VK_FALSE;
			rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizer.lineWidth = 1.0f;
			rasterizer.cullMode = (VkCullModeFlags)createinfo.cullmode;
			rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

			rasterizer.depthBiasEnable = VK_FALSE;
			rasterizer.depthBiasConstantFactor = 0.0f; // Optional
			rasterizer.depthBiasClamp = 0.0f; // Optional
			rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

			VkPipelineMultisampleStateCreateInfo multisampling{};
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisampling.minSampleShading = 1.0f; // Optional
			multisampling.pSampleMask = nullptr; // Optional
			multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
			multisampling.alphaToOneEnable = VK_FALSE; // Optional

			VkPipelineColorBlendAttachmentState colorblendattachment{};
			colorblendattachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorblendattachment.blendEnable = VK_TRUE;
			colorblendattachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorblendattachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorblendattachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorblendattachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorblendattachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorblendattachment.alphaBlendOp = VK_BLEND_OP_ADD;
			
			VkPipelineColorBlendStateCreateInfo colorblending{};
			colorblending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorblending.logicOpEnable = VK_FALSE;
			colorblending.logicOp = VK_LOGIC_OP_COPY; // Optional
			colorblending.attachmentCount = 1;
			colorblending.pAttachments = &colorblendattachment;
			colorblending.blendConstants[0] = 0.0f; // Optional
			colorblending.blendConstants[1] = 0.0f; // Optional
			colorblending.blendConstants[2] = 0.0f; // Optional
			colorblending.blendConstants[3] = 0.0f; // Optional


			//convert from platform level ids to an array of vulkan structs
			auto vkdescriptorsetlayouts = Base::ArrayList<VkDescriptorSetLayout, ArenaAllocator>::create(createinfo.layouts.count);
			for (const auto& layoutid : createinfo.layouts) {

				Vulkan::DescriptorSetLayout layout = context->descriptorsetlayouts[layoutid];
				vkdescriptorsetlayouts.pushBack((VkDescriptorSetLayout)layout.layout);
			}

			VkPipelineLayout pipelinelayout;
			VkPipelineLayoutCreateInfo pipelinelayoutinfo{};
			pipelinelayoutinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelinelayoutinfo.setLayoutCount = vkdescriptorsetlayouts.getCount(); // Optional
			pipelinelayoutinfo.pSetLayouts = vkdescriptorsetlayouts.raw(); // Optional
			pipelinelayoutinfo.pushConstantRangeCount = 0; // Optional
			pipelinelayoutinfo.pPushConstantRanges = nullptr; // Optional

			if (vkCreatePipelineLayout(context->device, &pipelinelayoutinfo, nullptr, &pipelinelayout) != VK_SUCCESS) {
				ERR << "failed to create pipeline layout! count = " << count << "\n";
				return false;
			}

			VkPipelineDepthStencilStateCreateInfo depthstencil{};
			depthstencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			if (createinfo.depthtest) {
				depthstencil.depthTestEnable = VK_TRUE;
				depthstencil.depthWriteEnable = VK_TRUE;
			}
			else {
				depthstencil.depthTestEnable = VK_FALSE;
				depthstencil.depthWriteEnable = VK_FALSE;
			}
			depthstencil.depthCompareOp = VK_COMPARE_OP_LESS;

			depthstencil.depthBoundsTestEnable = VK_FALSE;
			depthstencil.minDepthBounds = 0.0f; // Optional
			depthstencil.maxDepthBounds = 1.0f; // Optional

			depthstencil.stencilTestEnable = VK_FALSE;
			depthstencil.front = {}; // Optional
			depthstencil.back = {}; // Optional

			VkGraphicsPipelineCreateInfo pipelineinfo{};
			pipelineinfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineinfo.stageCount = shaderstages.getCount();
			pipelineinfo.pStages = shaderstages.raw();

			pipelineinfo.pVertexInputState = &vertexinputinfo;
			pipelineinfo.pInputAssemblyState = &inputassembly;
			pipelineinfo.pViewportState = &viewportstate;
			pipelineinfo.pRasterizationState = &rasterizer;
			pipelineinfo.pMultisampleState = &multisampling;
			pipelineinfo.pDepthStencilState = &depthstencil; 
			pipelineinfo.pColorBlendState = &colorblending;
			pipelineinfo.pDynamicState = &dynamicstate;

			pipelineinfo.layout = pipelinelayout;

			pipelineinfo.renderPass = context->renderpasses[createinfo.renderpass].renderpass;
			pipelineinfo.subpass = 0;

			pipelineinfo.basePipelineHandle = VK_NULL_HANDLE;
			pipelineinfo.basePipelineIndex = -1;

			GraphicsPipeline pipeline;

			if (vkCreateGraphicsPipelines(context->device, VK_NULL_HANDLE, 1, &pipelineinfo, nullptr, (VkPipeline*) & pipeline.vkpipeline) != VK_SUCCESS) {
				ERR << "failed to create pipeline! \n";
				return false;
			}

			pipeline.layout = pipelinelayout;

			graphicspipelines[i] = context->graphicspipelines.add(pipeline);
		}

		ArenaAllocator::resetArena(resetpoint);
		return true;
	}

	PH_GFX_CREATE_GRAPHICS_PIPELINES(createGraphicsPipelines) {
		return createGraphicsPipelines(context_s, createinfos, graphicspipelines, count);
	}

	bool32 bindGraphicsPipeline(VulkanAppContext* context, PH::Platform::GFX::GraphicsPipeline pipeline) {
		context->boundpipeline = pipeline;

		VkCommandBuffer vkbuffer = context->commandbuffers[context->frameindex];
		VkPipeline vkpipeline = context->graphicspipelines[pipeline].vkpipeline;
		vkCmdBindPipeline(vkbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkpipeline);
		return true;
	}

	PH_GFX_BIND_GRAPHICS_PIPELINE(bindGraphicsPipeline) {
		return bindGraphicsPipeline(context_s, pipeline);
	}


}