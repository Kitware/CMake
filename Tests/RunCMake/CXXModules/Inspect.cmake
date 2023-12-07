enable_language(CXX)

set(info "")

# See `Modules/Compiler/MSVC-CXX.cmake` for this. If there is explicitly no
# default, the feature list is populated to be everything.
if (DEFINED CMAKE_CXX_STANDARD_DEFAULT AND
    CMAKE_CXX_STANDARD_DEFAULT STREQUAL "")
  set(CMAKE_CXX_COMPILE_FEATURES "")
endif ()

# Detect if the environment forces a C++ standard, let the test selection know.
set(forced_cxx_standard 0)
if (CMAKE_CXX_FLAGS MATCHES "-std=")
  set(forced_cxx_standard 1)
endif ()

# Forward information about the C++ compile features.
string(APPEND info "\
set(CMAKE_CXX_COMPILE_FEATURES \"${CMAKE_CXX_COMPILE_FEATURES}\")
set(CMAKE_MAKE_PROGRAM \"${CMAKE_MAKE_PROGRAM}\")
set(forced_cxx_standard \"${forced_cxx_standard}\")
set(CMAKE_CXX_COMPILER_VERSION \"${CMAKE_CXX_COMPILER_VERSION}\")
set(CMAKE_CXX_OUTPUT_EXTENSION \"${CMAKE_CXX_OUTPUT_EXTENSION}\")
set(CXXModules_default_build_type \"${CMAKE_BUILD_TYPE}\")
set(CMAKE_CXX_STANDARD_DEFAULT \"${CMAKE_CXX_STANDARD_DEFAULT}\")
set(CMAKE_CXX20_STANDARD_COMPILE_OPTION \"${CMAKE_CXX20_STANDARD_COMPILE_OPTION}\")
")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/info.cmake" "${info}")
