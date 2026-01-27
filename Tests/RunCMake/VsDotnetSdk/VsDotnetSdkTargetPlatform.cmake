enable_language(CSharp)

set(CMAKE_DOTNET_SDK "Microsoft.NET.Sdk")

add_executable(foo csharponly.cs lib1.cs)

set_target_properties(foo PROPERTIES
  VS_GLOBAL_Platforms "${CMAKE_VS_PLATFORM_NAME}"
  VS_GLOBAL_PlatformTarget "${CMAKE_VS_PLATFORM_NAME}"
)
