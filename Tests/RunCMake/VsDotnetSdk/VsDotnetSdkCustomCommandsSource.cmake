enable_language(CSharp)

if(NOT CMAKE_CSharp_COMPILER)
    return()
endif()

set(CMAKE_DOTNET_SDK "Microsoft.NET.Sdk")
add_custom_command(
  OUTPUT bar.cs
  COMMAND copy /A ${CMAKE_CURRENT_SOURCE_DIR}/lib1.cs
             bar.cs
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib1.cs
  VERBATIM)

add_library(foo SHARED bar.cs)
