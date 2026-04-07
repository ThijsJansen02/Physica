
VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["platform"] = "%{wks.location}/Platform/src"
IncludeDir["base"] = "%{wks.location}/Base/src/"
IncludeDir["engine"] = "%{wks.location}/Engine/src/"
IncludeDir["glm"] = "%{wks.location}/dep/glm/"
IncludeDir["imgui"] = "%{wks.location}/dep/imgui"
IncludeDir["dep"] = "%{wks.location}/dep"
IncludeDir["entt"] = "%{wks.location}/dep/entt/src"
IncludeDir["yaml"] = "%{wks.location}/dep/yaml/include"
IncludeDir["assimp"] = "%{wks.location}/dep/assimp/include"
IncludeDir["vulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["imguizmo"] = "%{wks.location}/dep/ImGuizmo"
IncludeDir["zep"] = "%{wks.location}/dep/zep/include"
IncludeDir["editor"] = "%{wks.location}/Editor/src"
IncludeDir["stb"] = "%{wks.location}/dep/stb_"

--openssl and libssh for remote red pitaya access
IncludeDir["openssl"] = "%{wks.location}/dep/openssl/include"
IncludeDir["libssh"] = "%{wks.location}/dep/libssh/include"
IncludeDir["zlib"] = "%{wks.location}/dep/zlib/include"

LibraryDir = {}
LibraryDir["vulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["openssl"] = "%{wks.location}/dep/openssl/lib/VC/x64"
LibraryDir["libssh"] = "%{wks.location}/dep/libssh/lib"
LibraryDir["zlib"] = "%{wks.location}/dep/zlib/lib"

Library = {}
Library["vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"

Library["openssl_debug"] = "%{LibraryDir.openssl}/MTd/libssl.lib"
Library["openssl"] = "%{LibraryDir.openssl}/MT/libssl.lib"

Library["libcrypto_debug"] = "%{LibraryDir.openssl}/MTd/libcrypto.lib"
Library["libcrypto"] = "%{LibraryDir.openssl}/MT/libcrypto.lib"

Library["libssh_debug"] = "%{LibraryDir.libssh}/Debug/ssh.lib"
Library["libssh"] = "%{LibraryDir.libssh}/Release/ssh.lib"

Library["zlib_debug"] = "%{LibraryDir.zlib}/Debug/zsd.lib"
Library["zlib"] = "%{LibraryDir.zlib}/Release/zs.lib"

Library["shaderC_Debug"] = "%{LibraryDir.vulkanSDK}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.vulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.vulkanSDK}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.vulkanSDK}/SPIRV-Toolsd.lib"

Library["shaderC_Release"] = "%{LibraryDir.vulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.vulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.vulkanSDK}/spirv-cross-glsl.lib"
Library["SPIRV_Tools_Release"] = "%{LibraryDir.vulkanSDK}/SPIRV-Tools.lib"