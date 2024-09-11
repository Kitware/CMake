enable_language(CSharp)

if(NOT CMAKE_CSharp_COMPILER)
    return()
endif()

set(CMAKE_DOTNET_SDK "Microsoft.NET.Sdk")
add_library(foo SHARED lib1.cs)
set_target_properties(foo PROPERTIES OUTPUT_NAME "longer name")
