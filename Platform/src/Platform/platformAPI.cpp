#pragma once
#include "Platform/platformAPI.h"
#include <Platform/vulkan/vulkanAPI.h>

namespace PH::Platform {


	/// <summary>
	/// all the platform functions are defined here as opposed to platformapi where they are declared.
	/// </summary>
	PH_PLATFORM_API Intern::ConsoleWriteFN* consoleWrite = nullptr;
	PH_PLATFORM_API Intern::LoadFileFN* loadFile = nullptr;
	PH_PLATFORM_API Intern::UnloadFileFN* unloadFile = nullptr;
	PH_PLATFORM_API Intern::WriteFileFN* writeFile = nullptr;

	PH_PLATFORM_API Intern::createThreadFN* createThread = nullptr;

	PH_PLATFORM_API Intern::getTimeMsFN* getTimeMs = nullptr;

	PH_PLATFORM_API Intern::OpenFileDialogFN* openFileDialog = nullptr;
	PH_PLATFORM_API Intern::ListFilesFN* listFiles = nullptr;

	PH_PLATFORM_API Intern::gfx_CreateBuffersFN* GFX::createBuffers = nullptr;
	PH_PLATFORM_API Intern::gfx_DestroyBuffersFN* GFX::destroyBuffers = nullptr;
	PH_PLATFORM_API Intern::gfx_BindVertexBuffersFN* GFX::bindVertexBuffers = nullptr;
	PH_PLATFORM_API Intern::gfx_BindIndexBufferFN* GFX::bindIndexBuffer = nullptr;
	PH_PLATFORM_API Intern::gfx_CopyBufferFN* GFX::copyBuffer = nullptr;
	PH_PLATFORM_API Intern::gfx_UnmapBufferFN* GFX::unmapBuffer = nullptr;
	PH_PLATFORM_API Intern::gfx_MapBufferFN* GFX::mapBuffer = nullptr;

	PH_PLATFORM_API Intern::gfx_CreateDescriptorSetLayoutsFN* GFX::createDescriptorSetLayouts = nullptr;
	PH_PLATFORM_API Intern::gfx_createDescriptorSetsFN* GFX::createDescriptorSets = nullptr;
	PH_PLATFORM_API Intern::gfx_DestroyDescriptorSetsFN* GFX::destroyDescriptorSets = nullptr;
	PH_PLATFORM_API Intern::gfx_destroyDesctiptorSetLayoutsFN* GFX::destroyDescriptorSetLayouts = nullptr;
	PH_PLATFORM_API Intern::gfx_bindDescriptorSetsFN* GFX::bindDescriptorSets = nullptr;
	PH_PLATFORM_API Intern::gfx_updateDescriptorSetsFN* GFX::updateDescriptorSets = nullptr;

	PH_PLATFORM_API Intern::gfx_DrawFN* GFX::draw = nullptr;
	PH_PLATFORM_API Intern::gfx_DrawIndexedFN* GFX::drawIndexed = nullptr;

	PH_PLATFORM_API Intern::gfx_CreateRenderpassDescriptionFN* GFX::createRenderpassDescriptions = nullptr;
	PH_PLATFORM_API Intern::gfx_CreateFramebuffersFN* GFX::createFramebuffers = nullptr;

	PH_PLATFORM_API Intern::gfx_BeginRenderpassFN* GFX::beginRenderpass = nullptr;
	PH_PLATFORM_API Intern::gfx_EndRenderpassFN* GFX::endRenderpass = nullptr;

	PH_PLATFORM_API Intern::gfx_CreateShadersFN* GFX::createShaders = nullptr;
	PH_PLATFORM_API Intern::gfx_DestroyShadersFN* GFX::destroyShaders = nullptr;
	PH_PLATFORM_API Intern::gfx_CreateGraphicsPipelinesFN* GFX::createGraphicsPipelines = nullptr;
	PH_PLATFORM_API Intern::gfx_BindGraphicsPipelineFN* GFX::bindGraphicsPipeline = nullptr;

	PH_PLATFORM_API Intern::gfx_CreateTexturesFN* GFX::createTextures = nullptr;
	PH_PLATFORM_API Intern::gfx_DestroyTexturesFN* GFX::destroyTextures = nullptr;

	PH_PLATFORM_API Intern::GFX_SetScissorsFN* GFX::setScissors = nullptr;
	PH_PLATFORM_API Intern::gfx_SetViewportsFN* GFX::setViewports = nullptr;

	PH_PLATFORM_API Intern::gfx_DrawImguiWidgetsFN* GFX::drawImguiWidgets = nullptr;
	PH_PLATFORM_API Intern::gfx_CreateImguiImageFN* GFX::createImGuiImage = nullptr;
	
	namespace GFX {

		PH_PLATFORM_API bool32 destroyRenderpassDescriptions(GFX::RenderpassDescription* descriptions, uint32 count) {
			return Vulkan::destroyRenderpassDescriptions(Vulkan::context_s, descriptions, count);
		}

		PH_PLATFORM_API bool32 destroyFrameBuffers(GFX::Framebuffer* framebuffers, uint32 count) {
			return Vulkan::postDestroyFrameBuffers(Vulkan::context_s, framebuffers, count);
		}
	}
}

bool loadPlatformAPI(PH::Platform::Intern::PlatformAPI* platform) {
	
	PH::Platform::writeFile = platform->writefile;
	PH::Platform::consoleWrite = platform->consolewrite;
	PH::Platform::loadFile = platform->loadfile;
	PH::Platform::unloadFile = platform->unloadfile;

	PH::Platform::getTimeMs = platform->gettimems;

	PH::Platform::openFileDialog = platform->openfiledialog;
	PH::Platform::listFiles = platform->listfiles;

	PH::Platform::createThread = platform->createThread;

	PH::Platform::GFX::createRenderpassDescriptions = platform->gfxcreaterenderpassdescription;
	PH::Platform::GFX::createFramebuffers = platform->gfxcreateframebuffers;

	PH::Platform::GFX::beginRenderpass = platform->gfxbeginrenderpass;
	PH::Platform::GFX::endRenderpass = platform->gfxendrenderpass;

	PH::Platform::GFX::createShaders = platform->gfxcreateshaders;
	PH::Platform::GFX::destroyShaders = platform->gfxdestroyshaders;
	PH::Platform::GFX::createGraphicsPipelines = platform->gfxcreategraphicspipelines;
	PH::Platform::GFX::bindGraphicsPipeline = platform->gfxbindgraphicspipeline;

	PH::Platform::GFX::createDescriptorSetLayouts = platform->gfxcreatedescriptorsetlayouts;
	PH::Platform::GFX::destroyDescriptorSetLayouts = platform->gfxdestroydescriptorsetlayouts;
	PH::Platform::GFX::destroyDescriptorSets = platform->gfxdestroydescriptorsets;
	PH::Platform::GFX::createDescriptorSets = platform->gfxcreatedescriptorsets;
	PH::Platform::GFX::updateDescriptorSets = platform->gfxupdatedescriptorsets;
	PH::Platform::GFX::bindDescriptorSets = platform->gfxbinddescriptorsets;

	PH::Platform::GFX::createBuffers = platform->gfxcreatebuffers;
	PH::Platform::GFX::destroyBuffers = platform->gfxdestroybuffers;
	PH::Platform::GFX::bindVertexBuffers = platform->gfxbindvertexbuffers;
	PH::Platform::GFX::bindIndexBuffer = platform->gfxbindindexbuffer;
	PH::Platform::GFX::copyBuffer = platform->gfxcopybuffers;
	PH::Platform::GFX::mapBuffer = platform->gfxmapbuffer;
	PH::Platform::GFX::unmapBuffer = platform->gfxunmapbuffer;

	PH::Platform::GFX::draw = platform->gfxdraw;
	PH::Platform::GFX::drawIndexed = platform->gfxdrawindexed;
	PH::Platform::GFX::setViewports = platform->gfxsetviewports;
	PH::Platform::GFX::setScissors = platform->gfxsetscissors;

	PH::Platform::GFX::createTextures = platform->gfxcreatetextures;
	PH::Platform::GFX::destroyTextures = platform->gfxdestroytextures;

	PH::Platform::GFX::drawImguiWidgets = platform->gfxdrawimguiwidgets;
	PH::Platform::GFX::createImGuiImage = platform->gfxcreateimguiimage;
	return true;
}
