enable_language(CSharp)
if(NOT CMAKE_CSharp_COMPILER)
    return()
endif()

set(CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION "v4.6.1")

add_executable(foo foo.cs)

set_target_properties(foo PROPERTIES VS_DOTNET_STARTUP_OBJECT "MyCompany.Package.MyStarterClass")
