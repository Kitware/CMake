enable_language(CSharp)
if(NOT CMAKE_CSharp_COMPILER)
  return()
endif()

set(CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION "v4.6.1")
add_library(foo SHARED foo.cs)

set(CMAKE_DOTNET_TARGET_FRAMEWORK "netcoreapp3.1")
add_library(bar SHARED foo.cs)
