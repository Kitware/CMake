include(Platform/Windows-Intel)
set(_COMPILE_CXX " /TP")
__windows_compiler_intel(CXX)
set(CMAKE_NINJA_DEPTYPE_CXX intel) # special value handled by CMake
set(CMAKE_DEPFILE_FLAGS_CXX "-QMMD -QMT <DEP_TARGET> -QMF <DEP_FILE>")

if((NOT DEFINED CMAKE_DEPENDS_USE_COMPILER OR CMAKE_DEPENDS_USE_COMPILER)
    AND CMAKE_GENERATOR MATCHES "Makefiles|WMake")
  # dependencies are computed by the compiler itself
  set(CMAKE_CXX_DEPFILE_FORMAT gcc)
  set(CMAKE_CXX_DEPENDS_USE_COMPILER TRUE)
endif()
