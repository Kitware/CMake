include(Platform/Windows-MSVC)
set(_COMPILE_CXX " /TP")
if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 18.0)
  set(_FS_CXX " /FS")
endif()
__windows_compiler_msvc(CXX)

set(_cmake_feature_test_msvcxx_sealed "_MSC_VER >= 1400")
