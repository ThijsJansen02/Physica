#include "vulkanAPI.h"

namespace PH::Vulkan {

	PH::bool32 createDescriptorSetLayouts(
		Vulkan::VulkanAppContext* context, 
		PH::Platform::GFX::DescriptorSetLayoutCreateinfo* createinfos, 
		PH::Platform::GFX::DescriptorSetLayout* layouts, 
		uint32 count
	) {

		//get the reset point for the arena allocator to reset the allocator after completion
		PH::Base::MemoryArenaReset resetpoint = ArenaAllocator::getResetPoint();

		for (uint32 i = 0; i < count; i++) {

			Platform::GFX::DescriptorSetLayoutCreateinfo createinfo = createinfos[i];

			VkDescriptorSetLayoutCreateInfo layoutinfo{};
			layoutinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutinfo.bindingCount = createinfo.bindings.count;

			auto bindings = PH::Base::ArrayList<VkDescriptorSetLayoutBinding, ArenaAllocator>::create(createinfo.bindings.count);

			for (uint32 j = 0; j < createinfo.bindings.count; j++) {

				const auto& descriptorbinding = createinfo.bindings[j];

				VkDescriptorSetLayoutBinding vkbinding{};
				vkbinding.binding = descriptorbinding.binding;
				vkbinding.descriptorCount = descriptorbinding.descriptorcount;
				vkbinding.descriptorType = convertDescriptorType(descriptorbinding.type);
				vkbinding.stageFlags = convertShaderStageFlags(descriptorbinding.shaderstages);
				vkbinding.pImmutableSamplers = nullptr;

				bindings.pushBack(vkbinding);
			}

			layoutinfo.pBindings = bindings.raw();

			Vulkan::DescriptorSetLayout layout{};
			if (vkCreateDescriptorSetLayout(context->device, &layoutinfo, nullptr, (VkDescriptorSetLayout*) & layout.layout) != VK_SUCCESS) {
				return false;
			}

			layouts[i] = context->descriptorsetlayouts.add(layout);
		}
		ArenaAllocator::resetArena(resetpoint);
		return true;
	}

	PH_GFX_CREATE_DESCRIPTOR_SET_LAYOUT(createDescriptorSetLayouts) {
		return createDescriptorSetLayouts(context_s, createinfos, layouts, count);
	}

	bool32 destroyDesctiptorSetLayouts(VulkanAppContext* context, Platform::GFX::DescriptorSetLayout* layouts, uint32 count) { 
	
		for (uint32 i = 0; i < count; i++)
		{	
			vkDestroyDescriptorSetLayout(context->device, context->descriptorsetlayouts[layouts[i]].layout, nullptr);
		}
		return true;
	}

	PH_GFX_DESTROY_DESCIPTOR_SET_LAYOUTS(destroyDescriptorSetLayouts) {
		return destroyDesctiptorSetLayouts(context_s, layouts, count);
	}

	bool32 createDescriptorSets(
		VulkanAppContext* context, 
		PH::Platform::GFX::DescriptorSetCreateinfo* createinfos, 
		PH::Platform::GFX::DescriptorSet* descriptorsets,
		uint32 count
	) {
		//get the reset point for the arena allocator to reset the allocator after completion
		PH::Base::MemoryArenaReset resetpoint = ArenaAllocator::getResetPoint();

		VkDescriptorSetAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocinfo.descriptorPool = context->descriptorpool;
		
		uint32 vksetcount = 0;

		auto vkdescriptorsetlayouts = PH::Base::ArrayList<VkDescriptorSetLayout, ArenaAllocator>::create(count);

		for (uint32 i = 0; i < count; i++) {
			auto createinfo = createinfos[i];

			volatile DescriptorSetLayout* layoutobj = &context->descriptorsetlayouts[createinfo.layout];

			VkDescriptorSetLayout layout = layoutobj->layout;
			if (createinfo.dynamic) {
				for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
					vkdescriptorsetlayouts.pushBack(layout);
					vksetcount++;
				}
			}
			else {
				vkdescriptorsetlayouts.pushBack(layout);
				vksetcount++;
			}

		}

		allocinfo.descriptorSetCount = vksetcount;
		allocinfo.pSetLayouts = vkdescriptorsetlayouts.raw();

		auto vkdescriptorsets = PH::Base::DynamicArray<VkDescriptorSet, ArenaAllocator>::create(vksetcount);

		Base::lock(context->descriptorpoollock);
		if (vkAllocateDescriptorSets(context->device, &allocinfo, vkdescriptorsets.raw()) != VK_SUCCESS) {
			ERR << "failed to create descriptorsets!";
			PH_DEBUG_BREAK();
			return false;
		}

		_ReadWriteBarrier();
		Base::unlock(context->descriptorpoollock);

		vksetcount = 0;

		for (uint32 i = 0; i < count; i++) {
			auto createinfo = createinfos[i];
			
			Vulkan::DescriptorSet set;
			set.isdynamic = createinfo.dynamic;

			if (!createinfo.dynamic) {
				for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
					set.set[i] = vkdescriptorsets[vksetcount];
				}
				vksetcount++;
				descriptorsets[i] = context->descriptorsets.add(set);
			}
			else {
				for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
					set.set[i] = vkdescriptorsets[vksetcount];
					vksetcount++;
				}
				descriptorsets[i] = context->descriptorsets.add(set);
			}
		}

		//reset the arena after usage
		ArenaAllocator::resetArena(resetpoint);
		return true;
	}

	PH_GFX_CREATE_DESCRIPTOR_SETS(createDescriptorSets) {
		return createDescriptorSets(context_s, createinfos, descriptorsets, count);
	}

	//todo(Thijs) : add functionallity for when you want to update a set just for that frame.
	bool32 updateDescriptorSets(
		VulkanAppContext* context,
		PH::Platform::GFX::DescriptorWrite* writes,
		uint32 count
	) {
		PH::Base::MemoryArenaReset resetpoint = ArenaAllocator::getResetPoint();

		//the size of vkwrites is at least count, but can be more because dynamic buffers have more writes than static buffers
		auto vkwrites = PH::Base::ArrayList<VkWriteDescriptorSet, ArenaAllocator>::create(count);

		for (uint32 i = 0; i < count; i++) {
			auto write = writes[i];

			VkWriteDescriptorSet vkwrite{};
			vkwrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			vkwrite.descriptorCount = write.descriptorcount;
			vkwrite.descriptorType = convertDescriptorType(write.type);
			vkwrite.dstArrayElement = write.arrayelement;
			vkwrite.dstBinding = write.dstbinding;
			auto set = context->descriptorsets[write.dstset];

			//vkwrite.dstBinding = set.set;

			if (!set.isdynamic || write.dynamicwrite) {
				vkwrite.dstSet = set.set[context->frameindex];

				if (write.type == PH::Platform::GFX::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {

					//holds the image descriptors
					auto vkimageinfos = PH::Base::ArrayList<VkDescriptorImageInfo, ArenaAllocator>::create(write.descriptorcount);

					for (uint32 j = 0; j < write.descriptorcount; j++) {
						const auto& imageinfo = ((PH::Platform::GFX::DescriptorImageInfo*)write.descriptorinfo)[j];
						auto image = context->textureimages[imageinfo.texture];
						
						VkDescriptorImageInfo vkimageinfo{};
						vkimageinfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						vkimageinfo.imageView = image.view;
						vkimageinfo.sampler = image.sampler;

						vkimageinfos.pushBack(vkimageinfo);
					}
					vkwrite.pImageInfo = vkimageinfos.raw();
				}

				if (write.type == PH::Platform::GFX::DESCRIPTOR_TYPE_UNIFORM_BUFFER) {

					//keeps its place in memory because it is an arena allocated array.
					auto vkbufferinfos = PH::Base::ArrayList<VkDescriptorBufferInfo, ArenaAllocator>::create(write.descriptorcount);

					for (uint32 j = 0; j < write.descriptorcount; j++) {
						const auto& bufferinfo = ((PH::Platform::GFX::DescriptorBufferInfo*)write.descriptorinfo)[j];

						VkDescriptorBufferInfo vkbufferinfo{};
						INFO << bufferinfo.buffer << "\n";

						vkbufferinfo.buffer = context->buffers[bufferinfo.buffer].buffer[0];
						vkbufferinfo.offset = bufferinfo.offset;
						vkbufferinfo.range = bufferinfo.range;

						vkbufferinfos.pushBack(vkbufferinfo);
					}
					vkwrite.pBufferInfo = vkbufferinfos.raw();
				}

				vkwrites.pushBack(vkwrite);
			}
			else {
				for (uint32 j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
					vkwrite.dstSet = set.set[j];

					if (write.type == PH::Platform::GFX::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {

						//holds the image descriptors
						auto vkimageinfos = PH::Base::ArrayList<VkDescriptorImageInfo, ArenaAllocator>::create(write.descriptorcount);

						for (uint32 k = 0; k < write.descriptorcount; k++) {
							const auto& imageinfo = ((PH::Platform::GFX::DescriptorImageInfo*)write.descriptorinfo)[k];
							auto image = context->textureimages[imageinfo.texture];

							VkDescriptorImageInfo vkimageinfo{};
							vkimageinfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
							vkimageinfo.imageView = image.view;
							vkimageinfo.sampler = image.sampler;

							vkimageinfos.pushBack(vkimageinfo);
						}
						vkwrite.pImageInfo = vkimageinfos.raw();
					}
					

					if (write.type == PH::Platform::GFX::DESCRIPTOR_TYPE_UNIFORM_BUFFER) {

						//keeps its place in memory because it is an arena allocated array.
						auto vkbufferinfos = PH::Base::ArrayList<VkDescriptorBufferInfo, ArenaAllocator>::create(count);

						for (uint32 k = 0; k < write.descriptorcount; k++) {
							const auto& bufferinfo = ((PH::Platform::GFX::DescriptorBufferInfo*)write.descriptorinfo)[k];

							VkDescriptorBufferInfo vkbufferinfo{};
							vkbufferinfo.buffer = context->buffers[bufferinfo.buffer].buffer[j];
							vkbufferinfo.offset = bufferinfo.offset;
							vkbufferinfo.range = bufferinfo.range;

							vkbufferinfos.pushBack(vkbufferinfo);
						}
						vkwrite.pBufferInfo = vkbufferinfos.raw();
					}

					vkwrites.pushBack(vkwrite);
				}
			}
		}

		vkUpdateDescriptorSets(context->device, vkwrites.getCount(), vkwrites.raw(), 0, nullptr);
		ArenaAllocator::resetArena(resetpoint);
		return true;
	}

	PH_GFX_UPDATE_DESCRIPTOR_SETS(updateDescriptorSets) {
		return updateDescriptorSets(context_s, writes, count);
	}

	bool32 bindDescriptorSets(VulkanAppContext* context, PH::Platform::GFX::DescriptorSet* sets, uint32 firstset, uint32 count) {
		
		PH::Base::MemoryArenaReset resetpoint = ArenaAllocator::getResetPoint();
		auto vksets = PH::Base::ArrayList<VkDescriptorSet, ArenaAllocator>::create(count);

		for (uint32 i = 0; i < count; i++) {
			vksets.pushBack(context->descriptorsets[sets[i]].set[context->frameindex]);
		}

		vkCmdBindDescriptorSets(
			context->commandbuffers[context->frameindex],
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			context->graphicspipelines[context->boundpipeline].layout,
			firstset,
			count,
			vksets.raw(),
			0,
			nullptr
		);

		ArenaAllocator::resetArena(resetpoint);
		return true;
	}

	PH_GFX_BIND_DESCRIPTOR_SETS(bindDescriptorSets) {
		return bindDescriptorSets(context_s, sets, firstset, count);
	}

	bool32 destroyDescriptorSets(VulkanAppContext* context, Platform::GFX::DescriptorSet* desctiptorsets, uint32 count) {
		/*
		for (uint32 i = 0; i < count; i++) {

			vkFreeDescriptorSets()

		}
		*/
		return true;
	}

	PH_GFX_DESTROY_DESCRIPTOR_SETS(destroyDescriptorSets) {
		return destroyDescriptorSets(context_s, descriptorsets, count);
	}
}