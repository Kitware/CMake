cmake_minimum_required(VERSION 3.22)

# a simple CSharp only test case
project (DotNetSdk CSharp)

set(CMAKE_DOTNET_TARGET_FRAMEWORK net472)
set(CMAKE_DOTNET_SDK "Microsoft.NET.Sdk")

add_library(dotNetSdkLib1 SHARED lib1.cs)
set_target_properties(dotNetSdkLib1
    PROPERTIES
        VS_GLOBAL_RuntimeIdentifier win10-x64)

add_executable(DotNetSdk csharponly.cs)
target_link_libraries(DotNetSdk dotNetSdkLib1)
set_target_properties(DotNetSdk
    PROPERTIES
        VS_GLOBAL_RuntimeIdentifier win10-x64

        VS_DOTNET_REFERENCE_SomeDll
            ${PROJECT_SOURCE_DIR}/SomeDll.dll)
