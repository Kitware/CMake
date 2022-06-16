enable_language(CSharp)
if(NOT CMAKE_CSharp_COMPILER)
    return()
endif()

set(CMAKE_DOTNET_SDK "Microsoft.NET.Sdk")
set(CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION "net5.0")

add_executable(foo csharponly.cs lib1.cs)

# Issue 23376
target_compile_definitions(
  foo
    PUBLIC
      MY_FOO_DEFINE
      "MY_BAR_ASSIGNMENT=bar"
      $<$<CONFIG:Debug>:DEFINE_ONLY_FOR_DEBUG>
      $<$<CONFIG:Release>:DEFINE_ONLY_FOR_RELEASE>
)
