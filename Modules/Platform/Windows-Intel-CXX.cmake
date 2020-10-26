include(Platform/Windows-Intel)
set(_COMPILE_CXX " /TP")
__windows_compiler_intel(CXX)
set(CMAKE_NINJA_DEPTYPE_CXX intel) # special value handled by CMake
set(CMAKE_DEPFILE_FLAGS_CXX "-QMMD -QMT <DEP_TARGET> -QMF <DEP_FILE>")
