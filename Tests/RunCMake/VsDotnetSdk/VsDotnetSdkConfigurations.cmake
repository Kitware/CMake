enable_language(CSharp)

if(NOT CMAKE_CSharp_COMPILER)
    return()
endif()

set(CMAKE_SHARED_LINKER_FLAGS_EXTRATESTCONFIG "${CMAKE_SHARED_LINKER_FLAGS_Debug}")
list(APPEND CMAKE_CONFIGURATION_TYPES ExtraTestConfig)

set(CMAKE_DOTNET_SDK "Microsoft.NET.Sdk")
add_library(foo SHARED lib1.cs)
