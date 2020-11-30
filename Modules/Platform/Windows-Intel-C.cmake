include(Platform/Windows-Intel)
__windows_compiler_intel(C)
set(CMAKE_NINJA_DEPTYPE_C intel) # special value handled by CMake
set(CMAKE_DEPFILE_FLAGS_C "-QMMD -QMT <DEP_TARGET> -QMF <DEP_FILE>")

if((NOT DEFINED CMAKE_DEPENDS_USE_COMPILER OR CMAKE_DEPENDS_USE_COMPILER)
    AND CMAKE_GENERATOR MATCHES "Makefiles|WMake")
  # dependencies are computed by the compiler itself
  set(CMAKE_C_DEPFILE_FORMAT gcc)
  set(CMAKE_C_DEPENDS_USE_COMPILER TRUE)
endif()
