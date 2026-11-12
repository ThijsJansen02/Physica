
VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["platform"] = "%{PhysicaLocation}/Platform/src"
IncludeDir["base"] = "%{PhysicaLocation}/Base/src/"
IncludeDir["engine"] = "%{PhysicaLocation}/Engine/src/"
IncludeDir["glm"] = "%{PhysicaLocation}/dep/glm/"
IncludeDir["imgui"] = "%{PhysicaLocation}/dep/imgui"
IncludeDir["dep"] = "%{PhysicaLocation}/dep"
IncludeDir["entt"] = "%{PhysicaLocation}/dep/entt/src"
IncludeDir["yaml"] = "%{PhysicaLocation}/dep/yaml/include"
IncludeDir["assimp"] = "%{PhysicaLocation}/dep/assimp/include"
IncludeDir["vulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["imguizmo"] = "%{PhysicaLocation}/dep/ImGuizmo_/ImGuizmo/src"
IncludeDir["zep"] = "%{PhysicaLocation}/dep/zep/include"
IncludeDir["editor"] = "%{PhysicaLocation}/Editor/src"
IncludeDir["stb"] = "%{PhysicaLocation}/dep/stb_"
IncludeDir["pybind11"] = "%{PhysicaLocation}/dep/pybind11/include"
IncludeDir["labcore"] = "%{PhysicaLocation}/LabCore/src"
IncludeDir["labcore_scene"] = "%{PhysicaLocation}/LabCore/plugins/labcore-scene/src"
IncludeDir["labcore_ssh"] = "%{PhysicaLocation}/LabCore/plugins/labcore-ssh/src"
IncludeDir["labcore_3d"] = "%{PhysicaLocation}/LabCore/plugins/labcore-3d/src"

--IncludeDir["cpython"] = "C:/Users/Thijs/AppData/Local/Programs/Python/Python314/include"
IncludeDir["cpython"] = "%{PhysicaLocation}/dep/embeddedpython/include"
--IncludeDir["cpythonPCheaders"] = "%{PhysicaLocation}/dep/cpython/PC"

--openssl and libssh for remote red pitaya access
IncludeDir["openssl"] = "%{PhysicaLocation}/dep/openssl/include"
IncludeDir["libssh"] = "%{PhysicaLocation}/dep/libssh/include"
IncludeDir["zlib"] = "%{PhysicaLocation}/dep/zlib/include"


LibraryDir = {}

LibraryDir["vulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["openssl"] = "%{PhysicaLocation}/dep/openssl/lib/VC/x64"
LibraryDir["libssh"] = "%{PhysicaLocation}/dep/libssh/lib"
LibraryDir["zlib"] = "%{PhysicaLocation}/dep/zlib/lib"

LibraryDir["cpython"] = "%{PhysicaLocation}/dep/embeddedpython/libs"
--LibraryDir["cpython"] = "C:/Users/Thijs/AppData/Local/Programs/Python/Python314/libs"

Library = {}
Library["platform"] = "%{wks.location}/bin/" .. outputdir .. "/Platform/Platform.lib"

Library["cpython_debug"] = "%{LibraryDir.cpython}/python313.lib"
Library["cpython"] = "%{LibraryDir.cpython}/python313.lib"

Library["vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"

Library["openssl_debug"] = "%{LibraryDir.openssl}/MTd/libssl_static.lib"
Library["openssl"] = "%{LibraryDir.openssl}/MT/libssl_static.lib"


Library["libcrypto_debug"] = "%{LibraryDir.openssl}/MTd/libcrypto_static.lib"
Library["libcrypto"] = "%{LibraryDir.openssl}/MT/libcrypto_static.lib"

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