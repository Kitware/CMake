# Keep a scanning rule set before enable_language() (CMake's own tests set one
# to fake scanning); drop only the one Compiler/Clang-CXX adds.
if(DEFINED CMAKE_CXX_SCANDEP_SOURCE)
  set(__embt_scandep_source "${CMAKE_CXX_SCANDEP_SOURCE}")
endif()

include(Compiler/Clang-CXX)
include(Compiler/EMBTClang)

# The toolchain's clang-scan-deps does not apply the bcc64x driver's target,
# standard, and include paths, so C++ module scanning is not supported.
if(DEFINED __embt_scandep_source)
  set(CMAKE_CXX_SCANDEP_SOURCE "${__embt_scandep_source}")
  unset(__embt_scandep_source)
else()
  unset(CMAKE_CXX_SCANDEP_SOURCE)
endif()

string(APPEND CMAKE_CXX_FLAGS_INIT " -q")
