# a simple CSharp only test case
enable_language(CSharp)

set(CMAKE_DOTNET_TARGET_FRAMEWORK net472)
set(CMAKE_DOTNET_SDK "Microsoft.NET.Sdk")

if(CMAKE_VS_PLATFORM_NAME STREQUAL "ARM64")
    set(VS_RT_IDENTIFIER arm64)
else()
    set(VS_RT_IDENTIFIER win10-x64)
endif()

add_library(dotNetSdkLib1 SHARED lib1.cs)
set_target_properties(dotNetSdkLib1
    PROPERTIES
        VS_GLOBAL_RuntimeIdentifier ${VS_RT_IDENTIFIER})

add_executable(DotNetSdk csharponly.cs)
target_link_libraries(DotNetSdk dotNetSdkLib1)
set_target_properties(DotNetSdk
    PROPERTIES
        VS_GLOBAL_RuntimeIdentifier ${VS_RT_IDENTIFIER}

        VS_DOTNET_REFERENCE_SomeDll
            ${PROJECT_SOURCE_DIR}/SomeDll.dll)
