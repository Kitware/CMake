include(Platform/Windows-MSVC)
set(_COMPILE_CXX " /TP")
if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 18.0)
  set(_FS_CXX " /FS")
endif()

cmake_policy(GET CMP0138 _cmp0138)
if(_cmp0138 STREQUAL "NEW")
  if(NOT _MSVC_CXX_ARCHITECTURE_FAMILY STREQUAL "ARM" AND NOT _MSVC_CXX_ARCHITECTURE_FAMILY STREQUAL "ARM64")
    set(_ZiOrZI "-ZI")
  endif()
endif()
unset(_cmp0138)

__windows_compiler_msvc(CXX)

if((NOT DEFINED CMAKE_DEPENDS_USE_COMPILER OR CMAKE_DEPENDS_USE_COMPILER)
    AND CMAKE_GENERATOR MATCHES "Makefiles|WMake"
    AND CMAKE_DEPFILE_FLAGS_CXX)
  # dependencies are computed by the compiler itself
  set(CMAKE_CXX_DEPENDS_USE_COMPILER TRUE)
endif()
