include(Platform/Windows-MSVC)
if(NOT CMAKE_C_COMPILER_VERSION VERSION_LESS 18.0)
  set(_FS_C " /FS")
endif()
__windows_compiler_msvc(C)

set(CMAKE_C_STANDARD_DEFAULT 90)
