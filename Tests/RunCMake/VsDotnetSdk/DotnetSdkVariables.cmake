enable_language(CSharp)

if(NOT CMAKE_CSharp_COMPILER)
    return()
endif()

set(CMAKE_DOTNET_SDK "Microsoft.NET.Sdk")
add_library(foo SHARED lib1.cs)

set(CMAKE_DOTNET_SDK "Microsoft.NET.Sdk.Web")
add_library(bar SHARED lib1.cs)

set(CMAKE_DOTNET_SDK "")
add_library(baz SHARED lib1.cs)
