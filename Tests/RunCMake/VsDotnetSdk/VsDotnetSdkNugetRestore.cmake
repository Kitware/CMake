enable_language(CSharp)
if(NOT CMAKE_CSharp_COMPILER)
    return()
endif()

set(CMAKE_DOTNET_SDK "Microsoft.NET.Sdk")

add_executable(foo csharponly.cs lib1.cs)
