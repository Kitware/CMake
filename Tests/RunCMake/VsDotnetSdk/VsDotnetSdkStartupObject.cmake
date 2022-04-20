enable_language(CSharp)
if(NOT CMAKE_CSharp_COMPILER)
    return()
endif()

set(CMAKE_DOTNET_SDK "Microsoft.NET.Sdk")
set(CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION "net5.0")

add_executable(foo csharponly.cs lib1.cs)

set_target_properties(foo PROPERTIES VS_DOTNET_STARTUP_OBJECT "CSharpOnly.CSharpOnly")
