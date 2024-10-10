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

macro (cxx_check_import_std version)
  set(have_cxx${version}_import_std 0)
  if ("${version}" IN_LIST CMAKE_CXX_COMPILER_IMPORT_STD)
    set(have_cxx${version}_import_std 1)
  endif ()

  if (TARGET "__CMAKE:CXX${version}" AND NOT have_cxx${version}_import_std)
    message(FATAL_ERROR
      "The toolchain's C++${version} target exists, but the user variable does "
      "not indicate it.")
  endif ()
endmacro ()

cxx_check_import_std(23)
cxx_check_import_std(26)

# Forward information about the C++ compile features.
string(APPEND info "\
set(CMAKE_CXX_COMPILE_FEATURES \"${CMAKE_CXX_COMPILE_FEATURES}\")
set(CMAKE_MAKE_PROGRAM \"${CMAKE_MAKE_PROGRAM}\")
set(forced_cxx_standard \"${forced_cxx_standard}\")
set(have_cxx23_import_std \"${have_cxx23_import_std}\")
set(CMAKE_CXX_COMPILER_VERSION \"${CMAKE_CXX_COMPILER_VERSION}\")
set(CMAKE_CXX_OUTPUT_EXTENSION \"${CMAKE_CXX_OUTPUT_EXTENSION}\")
set(CXXModules_default_build_type \"${CMAKE_BUILD_TYPE}\")
set(CMAKE_CXX_STANDARD_DEFAULT \"${CMAKE_CXX_STANDARD_DEFAULT}\")
set(CMAKE_CXX20_STANDARD_COMPILE_OPTION \"${CMAKE_CXX20_STANDARD_COMPILE_OPTION}\")
")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/info.cmake" "${info}")
