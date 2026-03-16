#pragma once
#include "Platform/platformAPI.h"

namespace PH::Platform {


	/// <summary>
	/// all the platform functions are defined here as opposed to platformapi where they are declared.
	/// </summary>
	Intern::ConsoleWriteFN* consoleWrite = nullptr;
	Intern::LoadFileFN* loadFile = nullptr;
	Intern::UnloadFileFN* unloadFile = nullptr;
	Intern::WriteFileFN* writeFile = nullptr;

	Intern::createThreadFN* createThread = nullptr;

	Intern::getTimeMsFN* getTimeMs = nullptr;

	Intern::OpenFileDialogFN* openFileDialog = nullptr;
	Intern::ListFilesFN* listFiles = nullptr;

	Intern::gfx_CreateBuffersFN* GFX::createBuffers = nullptr;
	Intern::gfx_DestroyBuffersFN* GFX::destroyBuffers = nullptr;
	Intern::gfx_BindVertexBuffersFN* GFX::bindVertexBuffers = nullptr;
	Intern::gfx_BindIndexBufferFN* GFX::bindIndexBuffer = nullptr;
	Intern::gfx_CopyBufferFN* GFX::copyBuffer = nullptr;
	Intern::gfx_UnmapBufferFN* GFX::unmapBuffer = nullptr;
	Intern::gfx_MapBufferFN* GFX::mapBuffer = nullptr;

	Intern::gfx_CreateDescriptorSetLayoutsFN* GFX::createDescriptorSetLayouts = nullptr;
	Intern::gfx_createDescriptorSetsFN* GFX::createDescriptorSets = nullptr;
	Intern::gfx_DestroyDescriptorSetsFN* GFX::destroyDescriptorSets = nullptr;
	Intern::gfx_destroyDesctiptorSetLayoutsFN* GFX::destroyDescriptorSetLayouts = nullptr;
	Intern::gfx_bindDescriptorSetsFN* GFX::bindDescriptorSets = nullptr;
	Intern::gfx_updateDescriptorSetsFN* GFX::updateDescriptorSets = nullptr;

	Intern::gfx_DrawFN* GFX::draw = nullptr;
	Intern::gfx_DrawIndexedFN* GFX::drawIndexed = nullptr;

	Intern::gfx_CreateRenderpassDescriptionFN* GFX::createRenderpassDescriptions = nullptr;
	Intern::gfx_CreateFramebuffersFN* GFX::createFramebuffers = nullptr;

	Intern::gfx_BeginRenderpassFN* GFX::beginRenderpass = nullptr;
	Intern::gfx_EndRenderpassFN* GFX::endRenderpass = nullptr;

	Intern::gfx_CreateShadersFN* GFX::createShaders = nullptr;
	Intern::gfx_DestroyShadersFN* GFX::destroyShaders = nullptr;
	Intern::gfx_CreateGraphicsPipelinesFN* GFX::createGraphicsPipelines = nullptr;
	Intern::gfx_BindGraphicsPipelineFN* GFX::bindGraphicsPipeline = nullptr;

	Intern::gfx_CreateTexturesFN* GFX::createTextures = nullptr;

	Intern::GFX_SetScissorsFN* GFX::setScissors = nullptr;
	Intern::gfx_SetViewportsFN* GFX::setViewports = nullptr;

	Intern::gfx_DrawImguiWidgetsFN* GFX::drawImguiWidgets = nullptr;
	Intern::gfx_CreateImguiImageFN* GFX::createImGuiImage = nullptr;

}

PH_DLL_EXPORT bool applicationReload(PH::Platform::Intern::PlatformAPI* platform) {
	
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

	PH::Platform::GFX::drawImguiWidgets = platform->gfxdrawimguiwidgets;
	PH::Platform::GFX::createImGuiImage = platform->gfxcreateimguiimage;
	return true;
}
