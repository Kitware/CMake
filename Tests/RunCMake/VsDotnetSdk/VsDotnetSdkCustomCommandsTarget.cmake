enable_language(CSharp)

if(NOT CMAKE_CSharp_COMPILER)
    return()
endif()

set(CMAKE_DOTNET_SDK "Microsoft.NET.Sdk")
add_library(foo SHARED lib1.cs)
add_custom_command(TARGET foo
  PRE_BUILD
  COMMAND echo "This shouldn't happen!"
  VERBATIM)
