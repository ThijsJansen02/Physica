#pragma once
#include <Base/Datastructures/String.h>
#include <Base/Datastructures/Array.h>
#include <Base/Base.h>
#include <Base/Basetypes.h>

#include "Events.h"
#include "enums.h"

#include <imgui.h>

//should be removed in the future once threads are moved to the platform side
#include <Windows.h>

//used for a function that must be imported via a dll
#define PH_DLL_IMPORT 

//used for a function that must be exported
#define PH_DLL_EXPORT extern "C" __declspec(dllexport)

//platform export functions
//
//
///////////////////////////
#define PH_CONSOLE_WRITE(name) void name(const PH::Base::SubString& str)
#define PH_LOAD_FILE(name) PH::bool32 name(PH::Platform::FileBuffer* file, const char* filepath)
#define PH_UNLOAD_FILE(name) PH::bool32 name(PH::Platform::FileBuffer* file)
#define PH_WRITE_FILE(name) PH::bool32 name(const PH::Platform::FileBuffer& file, const char* filepath)

#define PH_GET_TIME_MS(name) PH::real64 name()
#define PH_CREATE_THREAD(name) PH::bool32 name(const PH::Platform::ThreadCreateInfo& info, PH::Platform::Thread* thread)

#define PH_OPEN_FILE_DIALOG(name) PH::bool32 name(const PH::Platform::OpenFileDialogInfo& info)
#define PH_LIST_DIRECTORIES(name) PH::sizeptr name(char* buffer, PH::sizeptr buffersize, const char* filepath)

//platform import definitions
//
//
//
///////////////////////////
#define PH_APPLICATION_UPDATE(name) bool __cdecl name(PH::Platform::Context context)
#define PH_APPLICATION_INITIALIZE(name) bool __cdecl name(PH::Platform::Context context)
#define PH_APPLICATION_DESTROY(name) bool __cdecl name(PH::Platform::Context context)


namespace PH::Platform {

	typedef int32(ThreadProc)(void* userdata);

	struct ThreadCreateInfo {
		ThreadProc* threadproc;
		bool32 usegfx;
		void* userdata;

		sizeptr threadworkmemorysize = KILO_BYTE;
	};

	struct Thread {
		uint32 id;
		HANDLE handle;
	};

	struct OpenFileDialogInfo {
		PH::sizeptr resultbuffersize;
		char* resultbuffer;
		char* filter;
		PH::bool32 read;
	};

	struct Context {
		Base::Array<PH::Platform::Event> events;

		uint32 windowwidth;
		uint32 windowheight;

		PH::sizeptr appmemsize;
		void* appmemory;

		//why does the imgui context need to be stored here? well, the imgui context needs to be created on the same thread as the rendering context, which is the main thread. but the application entry point is called from a different thread, so we need to store the imgui context here so that it can be accessed from the application entry point.
		ImGuiContext* imguicontext;

		//the ellapsed time of the program since startup in seconds
		real64 ellapsedtimeseconds;

	};

	struct FileBuffer {
		void* data;
		PH::sizeptr size;
	};

	namespace GFX {
		typedef uint64 id;
		typedef id RenderpassDescription;
		typedef id GraphicsPipeline;
		typedef id Shader;
		typedef id Framebuffer;
		typedef id Buffer;
		typedef id DescriptorSetLayout;
		typedef id DescriptorSet;
		typedef id Texture;


		/// <summary>
		/// describes the binding of a vertexbuffer, the binding specifies for which vertexbinding we want to give a description. 
		/// Stride is the per vertex/instance size. 
		/// inputrate describes if the pipeline should read the data per vertex or per instance.
		/// </summary>
		struct VertexInputBindingDescription {

			PH::uint32 binding;
			PH::uint32 stride;
			VertexInputRate inputrate;
		};

		/// <summary>
		/// describes the input attributes for a binding.
		/// 
		/// location is the location within the binding.
		/// format is the format that is used in the.
		/// </summary>
		struct VertexInputAttributeDescription {

			PH::uint32 binding;
			PH::uint32 location;
			GFX::Format format;
			PH::uint32 offset;
		};

		struct AttachmentReference {

			//the index of the attachment that is referenced
			uint32 attachmentindex;

			//the layout that the image should have when this subpass is performed
			ImageLayout layout;
		};

		struct SubPass {
			Base::Array<AttachmentReference> colorattachments;
			AttachmentReference* depthstencilattachment;
			PipelineBindPoint bindpoint;
		};

		struct SubpassDependency {
			uint32 srcsubpass;
			uint32 dstsubpass;

			PipelineStageFlags srcstagemask;
			AccessFlags srcaccessmask;

			PipelineStageFlags dststagemask;
			AccessFlags dstaccessmask;
		};

		struct AttachmentDescription {
			ImageLayout initiallayout;
			ImageLayout finallayout;

			Format format;
			AttachmentLoadOp loadop;
			AttachmentStoreOp storeop;
		};

		struct RenderpassDescriptionCreateinfo {
			PH::Base::Array<AttachmentDescription> attachments;
			PH::Base::Array<SubPass> subpasses;
			PH::Base::Array<SubpassDependency> dependencies;
		};

		struct FramebufferCreateInfo {

			PH::Base::Array<Texture> attachments;
			
			uint32 height;
			uint32 width;

			RenderpassDescription renderpassdescription;
		};

		enum struct ShaderSourceType {
			VULKAN_GLSL,
			VULKAN_BINARIES
		};

		struct ShaderCreateinfo {
			void* sourcecode;
			sizeptr size;
			ShaderStageFlags stage;

			//the type of the source code
			ShaderSourceType sourcetype;

			//set to a directory if you want to save a binary in a location
			const char* chachedir;
		};



		struct GraphicsPipelineCreateinfo {
			PH::Base::Array<Shader> shaderstages;
			PH::Base::Array<DescriptorSetLayout> layouts;

			PrimitiveTopology topology;
			RenderpassDescription renderpass;

			CullModeFlags cullmode;
			bool32 depthtest;

			PH::Base::Array<VertexInputBindingDescription> vertexbindingdescriptions;
			PH::Base::Array<VertexInputAttributeDescription> vertexattributedescriptions;
		};

		struct uivec2 {
			union {
				struct {
					uint32 x;
					uint32 y;
				};

				struct {
					uint32 width;
					uint32 height;
				};
			};
		};

		union ClearColorValue {
			float       float32[4];
			int32_t     int32[4];
			uint32_t    uint32[4];
		};

		struct ClearDepthStencilValue {
			float       depth;
			uint32_t    stencil;
		};

		union ClearValue {
			ClearColorValue           color;
			ClearDepthStencilValue    depthStencil;
		};

		struct RenderpassbeginInfo {
			RenderpassDescription description;
			Framebuffer framebuffer;

			PH::Base::Array<ClearValue> clearvalues;

			uivec2 renderarea;
		};

		struct DescriptorSetCreateinfo {
			bool32 dynamic;
			DescriptorSetLayout layout;
		};

		struct DescriptorBufferInfo {
			Buffer buffer;
			sizeptr offset;
			sizeptr range;
		};

		struct DescriptorImageInfo {
			Texture texture;
		};

		struct DescriptorWrite {
			DescriptorSet dstset;
			uint32 dstbinding;
			uint32 arrayelement;
			uint32 descriptorcount;

			bool32 dynamicwrite;

			void* descriptorinfo;

			DescriptorType type;
		};

		struct Viewport {
			real32 x;
			real32 y;
			real32 width;
			real32 height;
		};

		struct Scissor {
			uint32 x;
			uint32 y;
			uint32 width; 
			uint32 height;
		};

		struct BufferCreateinfo {
			sizeptr size;
			void* data;
			BufferUsageFlags bufferusage;
			MemoryPropertyFlags memoryproperties;
			bool32 dynamic;
		};

		struct DescriptorBinding {
			PH::uint32 binding;
			PH::Platform::GFX::DescriptorType type;
			PH::uint32 descriptorcount;
			PH::Platform::GFX::ShaderStageFlags shaderstages;
		};

		struct DescriptorSetLayoutCreateinfo {
			Base::Array<DescriptorBinding> bindings;
		};

		struct TextureCreateInfo {
			uint32 width;
			uint32 height;

			uint32 ArrayLayers = 1;
			ImageViewType viewtype = ImageViewType::IMAGE_VIEW_TYPE_2D;

			void* data;
			PH::Platform::GFX::ImageUsageFlags usage;
			PH::Platform::GFX::Format format;
		};
	}

#define PH_GFX_NULL 0

//platform gfx definitions
#define PH_GFX_DRAW(name) PH::bool32 name(PH::uint32 vertexcount, PH::uint32 instancecount, PH::uint32 firstvertex, PH::uint32 firstinstance)
#define PH_GFX_DRAW_INDEXED(name) PH::bool32 name(PH::uint32 indexcount, PH::uint32 instancecount, PH::uint32 firstindex, PH::uint32 firstinstance)

#define PH_GFX_SET_VIEWPORTS(name) PH::bool32 name(PH::Platform::GFX::Viewport* viewports, PH::uint32 count)
#define PH_GFX_SET_SCISSORS(name) PH::bool32 name(PH::Platform::GFX::Scissor* scissors, PH::uint32 count)

#define PH_GFX_CREATE_BUFFERS(name) PH::bool32 name(PH::Platform::GFX::BufferCreateinfo* createinfos, PH::Platform::GFX::Buffer* buffers, PH::uint32 count)
#define PH_GFX_DESTROY_BUFFERS(name) PH::bool32 name(PH::Platform::GFX::Buffer* buffers, PH::uint32 count)
#define PH_GFX_BIND_INDEXBUFFER(name) PH::bool32 name(PH::Platform::GFX::Buffer buffer, PH::sizeptr offset)
#define PH_GFX_BIND_VERTEXBUFFERS(name) PH::bool32 name(PH::uint32 firstbinding, PH::uint32 nbindings, PH::Platform::GFX::Buffer* buffers, PH::sizeptr* offsets)
#define PH_GFX_COPY_BUFFER(name) PH::bool32 name(PH::Platform::GFX::Buffer srcbuffer, PH::Platform::GFX::Buffer dstbuffer, PH::sizeptr size, PH::sizeptr srcoffset, PH::sizeptr dstoffset)
#define PH_GFX_MAP_BUFFER(name) PH::bool32 name(void** memory, PH::Platform::GFX::Buffer buffer)
#define PH_GFX_UNMAP_BUFFER(name) PH::bool32 name(PH::Platform::GFX::Buffer buffer)

#define PH_GFX_CREATE_DESCRIPTOR_SET_LAYOUT(name) PH::bool32 name(PH::Platform::GFX::DescriptorSetLayoutCreateinfo* createinfos, PH::Platform::GFX::DescriptorSetLayout* layouts, uint32 count)
#define PH_GFX_CREATE_DESCRIPTOR_SETS(name) PH::bool32 name(PH::Platform::GFX::DescriptorSetCreateinfo* createinfos, PH::Platform::GFX::DescriptorSet* descriptorsets, PH::uint32 count)
#define PH_GFX_DESTROY_DESCIPTOR_SET_LAYOUTS(name) PH::bool32 name(PH::Platform::GFX::DescriptorSetLayout* layouts, PH::uint32 count)
#define PH_GFX_DESTROY_DESCRIPTOR_SETS(name) PH::bool32 name(PH::Platform::GFX::DescriptorSet* descriptorsets, PH::uint32 count)
#define PH_GFX_UPDATE_DESCRIPTOR_SETS(name) PH::bool32 name(PH::Platform::GFX::DescriptorWrite* writes, PH::uint32 count)
#define PH_GFX_BIND_DESCRIPTOR_SETS(name) PH::bool32 name(PH::Platform::GFX::DescriptorSet* sets, PH::uint32 firstset, PH::uint32 count)

#define PH_GFX_BEGIN_RENDERPASS(name) PH::bool32 name(PH::Platform::GFX::RenderpassbeginInfo* renderpassbegininfo)
#define PH_GFX_END_RENDERPASS(name) PH::bool32 name()
#define PH_GFX_CREATE_RENDERPASS_DESCRIPTIONS(name) PH::bool32 name(PH::Platform::GFX::RenderpassDescriptionCreateinfo* createinfos, PH::Platform::GFX::RenderpassDescription* descriptions, PH::uint32 count)

#define PH_GFX_CREATE_FRAMEBUFFERS(name) PH::bool32 name(PH::Platform::GFX::FramebufferCreateInfo* createinfos, PH::Platform::GFX::Framebuffer* framebuffers, PH::uint32 count)

#define PH_GFX_CREATE_TEXTURES(name) PH::bool32 name(PH::Platform::GFX::TextureCreateInfo* createinfos, PH::Platform::GFX::Texture* textures, PH::uint32 count)

#define PH_GFX_CREATE_SHADERS(name) PH::bool32 name(PH::Platform::GFX::ShaderCreateinfo* createinfos, PH::Platform::GFX::Shader* shaders, PH::uint32 count)
#define PH_GFX_DESTROY_SHADERS(name) PH::bool32 name(PH::Platform::GFX::Shader* shaders, PH::uint32 count)
#define PH_GFX_CREATE_GRAPHICS_PIPELINES(name) PH::bool32 name(PH::Platform::GFX::GraphicsPipelineCreateinfo* createinfos, PH::Platform::GFX::GraphicsPipeline* graphicspipelines, PH::uint32 count)
#define PH_GFX_BIND_GRAPHICS_PIPELINE(name) PH::bool32 name(PH::Platform::GFX::GraphicsPipeline pipeline)

#define PH_GFX_CREATE_IMGUI_IMAGE(name) ImTextureID name(PH::Platform::GFX::Texture texture)
#define PH_GFX_DRAW_IMGUI_WIDGETS(name) PH::bool32 name(ImDrawData* drawdata)

#if defined GFX_IMPORT
#define PLATFORM_GFX_API extern "c" __declspec(dllexport)
#elif defined GFX_EXPORT
#define PLATFORM_GFX_API extern "c" __declspec(dllimport)
#else 
#define PLATFORM_GFX_API
#endif




	/// <summary>
	/// all types that need not be visible to the user of the API
	/// </summary>
	namespace Intern {

		typedef PH_CONSOLE_WRITE(ConsoleWriteFN);
		typedef PH_LOAD_FILE(LoadFileFN);
		typedef PH_UNLOAD_FILE(UnloadFileFN);
		typedef PH_WRITE_FILE(WriteFileFN);

		typedef PH_OPEN_FILE_DIALOG(OpenFileDialogFN);
		typedef PH_LIST_DIRECTORIES(ListFilesFN);

		typedef PH_GET_TIME_MS(getTimeMsFN);

		typedef PH_CREATE_THREAD(createThreadFN);

		//gfx function typedefs
		typedef PH_GFX_DRAW(gfx_DrawFN);
		typedef PH_GFX_DRAW_INDEXED(gfx_DrawIndexedFN);
		typedef PH_GFX_SET_VIEWPORTS(gfx_SetViewportsFN);
		typedef PH_GFX_SET_SCISSORS(GFX_SetScissorsFN);

		typedef PH_GFX_CREATE_BUFFERS(gfx_CreateBuffersFN);
		typedef PH_GFX_DESTROY_BUFFERS(gfx_DestroyBuffersFN);
		typedef PH_GFX_BIND_VERTEXBUFFERS(gfx_BindVertexBuffersFN);
		typedef PH_GFX_BIND_INDEXBUFFER(gfx_BindIndexBufferFN);
		typedef PH_GFX_COPY_BUFFER(gfx_CopyBufferFN);
		typedef PH_GFX_MAP_BUFFER(gfx_MapBufferFN);
		typedef PH_GFX_UNMAP_BUFFER(gfx_UnmapBufferFN);

		typedef PH_GFX_CREATE_DESCRIPTOR_SET_LAYOUT(gfx_CreateDescriptorSetLayoutsFN);
		typedef PH_GFX_CREATE_DESCRIPTOR_SETS(gfx_createDescriptorSetsFN);
		typedef PH_GFX_DESTROY_DESCIPTOR_SET_LAYOUTS(gfx_destroyDesctiptorSetLayoutsFN);
		typedef PH_GFX_DESTROY_DESCRIPTOR_SETS(gfx_DestroyDescriptorSetsFN);
		typedef PH_GFX_UPDATE_DESCRIPTOR_SETS(gfx_updateDescriptorSetsFN);
		typedef PH_GFX_BIND_DESCRIPTOR_SETS(gfx_bindDescriptorSetsFN);

		typedef PH_GFX_CREATE_SHADERS(gfx_CreateShadersFN);
		typedef PH_GFX_DESTROY_SHADERS(gfx_DestroyShadersFN);
		typedef PH_GFX_CREATE_GRAPHICS_PIPELINES(gfx_CreateGraphicsPipelinesFN);
		typedef PH_GFX_BIND_GRAPHICS_PIPELINE(gfx_BindGraphicsPipelineFN);

		typedef PH_GFX_CREATE_TEXTURES(gfx_CreateTexturesFN);

		typedef PH_GFX_CREATE_RENDERPASS_DESCRIPTIONS(gfx_CreateRenderpassDescriptionFN);
		typedef PH_GFX_CREATE_FRAMEBUFFERS(gfx_CreateFramebuffersFN);

		typedef PH_GFX_BEGIN_RENDERPASS(gfx_BeginRenderpassFN);
		typedef PH_GFX_END_RENDERPASS(gfx_EndRenderpassFN);

		
		typedef PH_GFX_DRAW_IMGUI_WIDGETS(gfx_DrawImguiWidgetsFN);
		typedef PH_GFX_CREATE_IMGUI_IMAGE(gfx_CreateImguiImageFN);

		struct PlatformAPI {

			PH::Platform::Intern::ConsoleWriteFN* consolewrite;
			PH::Platform::Intern::LoadFileFN* loadfile;
			PH::Platform::Intern::UnloadFileFN* unloadfile;
			PH::Platform::Intern::WriteFileFN* writefile;

			PH::Platform::Intern::getTimeMsFN* gettimems;

			PH::Platform::Intern::OpenFileDialogFN* openfiledialog;
			PH::Platform::Intern::ListFilesFN* listfiles;

			PH::Platform::Intern::createThreadFN* createThread;
			
			//gfx functions
			PH::Platform::Intern::gfx_DrawFN* gfxdraw;
			PH::Platform::Intern::gfx_DrawIndexedFN* gfxdrawindexed;
			PH::Platform::Intern::gfx_SetViewportsFN* gfxsetviewports;
			PH::Platform::Intern::GFX_SetScissorsFN* gfxsetscissors;

			PH::Platform::Intern::gfx_CreateBuffersFN* gfxcreatebuffers;
			PH::Platform::Intern::gfx_DestroyBuffersFN* gfxdestroybuffers;
			PH::Platform::Intern::gfx_BindVertexBuffersFN* gfxbindvertexbuffers;
			PH::Platform::Intern::gfx_BindIndexBufferFN* gfxbindindexbuffer;
			PH::Platform::Intern::gfx_CopyBufferFN* gfxcopybuffers;
			PH::Platform::Intern::gfx_MapBufferFN* gfxmapbuffer;
			PH::Platform::Intern::gfx_UnmapBufferFN* gfxunmapbuffer;

			PH::Platform::Intern::gfx_CreateDescriptorSetLayoutsFN* gfxcreatedescriptorsetlayouts;
			PH::Platform::Intern::gfx_destroyDesctiptorSetLayoutsFN* gfxdestroydescriptorsetlayouts;
			PH::Platform::Intern::gfx_createDescriptorSetsFN* gfxcreatedescriptorsets;
			PH::Platform::Intern::gfx_DestroyDescriptorSetsFN* gfxdestroydescriptorsets;
			PH::Platform::Intern::gfx_updateDescriptorSetsFN* gfxupdatedescriptorsets;
			PH::Platform::Intern::gfx_bindDescriptorSetsFN* gfxbinddescriptorsets;

			PH::Platform::Intern::gfx_CreateRenderpassDescriptionFN* gfxcreaterenderpassdescription;
			PH::Platform::Intern::gfx_CreateFramebuffersFN* gfxcreateframebuffers;

			PH::Platform::Intern::gfx_BeginRenderpassFN* gfxbeginrenderpass;
			PH::Platform::Intern::gfx_EndRenderpassFN* gfxendrenderpass;

			PH::Platform::Intern::gfx_CreateTexturesFN* gfxcreatetextures;

			PH::Platform::Intern::gfx_CreateShadersFN* gfxcreateshaders;
			PH::Platform::Intern::gfx_DestroyShadersFN* gfxdestroyshaders;
			PH::Platform::Intern::gfx_CreateGraphicsPipelinesFN* gfxcreategraphicspipelines;
			PH::Platform::Intern::gfx_BindGraphicsPipelineFN* gfxbindgraphicspipeline;

			PH::Platform::Intern::gfx_DrawImguiWidgetsFN* gfxdrawimguiwidgets;
			PH::Platform::Intern::gfx_CreateImguiImageFN* gfxcreateimguiimage;
		};
	}

	namespace GFX {
		extern Intern::gfx_DrawFN* draw;
		extern Intern::gfx_DrawIndexedFN* drawIndexed;
		extern Intern::gfx_SetViewportsFN* setViewports;
		extern Intern::GFX_SetScissorsFN* setScissors;

		extern Intern::gfx_CreateBuffersFN* createBuffers;
		extern Intern::gfx_DestroyBuffersFN* destroyBuffers;
		extern Intern::gfx_BindVertexBuffersFN* bindVertexBuffers;
		extern Intern::gfx_BindIndexBufferFN* bindIndexBuffer;
		extern Intern::gfx_CopyBufferFN* copyBuffer;	
		extern Intern::gfx_MapBufferFN* mapBuffer;
		extern Intern::gfx_UnmapBufferFN* unmapBuffer;

		extern Intern::gfx_CreateRenderpassDescriptionFN* createRenderpassDescriptions;
		extern Intern::gfx_CreateFramebuffersFN* createFramebuffers;

		extern Intern::gfx_BeginRenderpassFN* beginRenderpass;
		extern Intern::gfx_EndRenderpassFN* endRenderpass;

		extern Intern::gfx_CreateDescriptorSetLayoutsFN* createDescriptorSetLayouts;
		extern Intern::gfx_destroyDesctiptorSetLayoutsFN* destroyDescriptorSetLayouts;
		extern Intern::gfx_createDescriptorSetsFN* createDescriptorSets;
		extern Intern::gfx_DestroyDescriptorSetsFN* destroyDescriptorSets;
		extern Intern::gfx_updateDescriptorSetsFN* updateDescriptorSets;
		extern Intern::gfx_bindDescriptorSetsFN* bindDescriptorSets;

		extern Intern::gfx_CreateShadersFN* createShaders;
		extern Intern::gfx_DestroyShadersFN* destroyShaders;
		extern Intern::gfx_BindGraphicsPipelineFN* bindGraphicsPipeline;
		extern Intern::gfx_CreateGraphicsPipelinesFN* createGraphicsPipelines;

		extern Intern::gfx_CreateTexturesFN* createTextures;

		extern Intern::gfx_DrawImguiWidgetsFN* drawImguiWidgets;
		extern Intern::gfx_CreateImguiImageFN* createImGuiImage;
	}

	extern Intern::LoadFileFN* loadFile;
	extern Intern::UnloadFileFN* unloadFile;
	extern Intern::ConsoleWriteFN* consoleWrite;
	extern Intern::WriteFileFN* writeFile;

	extern Intern::getTimeMsFN* getTimeMs;

	extern Intern::createThreadFN* createThread;
	extern Intern::ListFilesFN* listFiles;

	extern Intern::OpenFileDialogFN* openFileDialog;
}