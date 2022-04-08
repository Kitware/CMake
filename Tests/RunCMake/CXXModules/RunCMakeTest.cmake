include(RunCMake)

# For `if (IN_LIST)`
cmake_policy(SET CMP0057 NEW)

run_cmake(compiler_introspection)
include("${RunCMake_BINARY_DIR}/compiler_introspection-build/info.cmake")

# Test negative cases where C++20 modules do not work.
run_cmake(NoCXX)
if ("cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
  # This test requires that the compiler be told to compile in an older-than-20
  # standard. If the compiler forces a standard to be used, skip it.
  if (NOT forced_cxx_standard)
    run_cmake(NoCXX20)
  endif ()

  # This test uses C++20, but another prerequisite is missing, so forced
  # standards don't matter.
  run_cmake(NoCXX20ModuleFlag)
endif ()

if (RunCMake_GENERATOR MATCHES "Ninja")
  execute_process(
    COMMAND "${CMAKE_MAKE_PROGRAM}" --version
    RESULT_VARIABLE res
    OUTPUT_VARIABLE ninja_version
    ERROR_VARIABLE err
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE)

  if (res)
    message(WARNING
      "Failed to determine `ninja` version: ${err}")
    set(ninja_version "0")
  endif ()
endif ()

# Test behavior when the generator does not support C++20 modules.
if (NOT RunCMake_GENERATOR MATCHES "Ninja" OR
    ninja_version VERSION_LESS "1.10" OR
    NOT "cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
  if ("cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    run_cmake(NoDyndepSupport)
  endif ()

  # Bail; the remaining tests require the generator to successfully generate
  # with C++20 modules in the source list.
  return ()
endif ()

set(fileset_types
  Modules
  ModuleHeaderUnits)
set(scopes
  Interface
  Private
  Public)
foreach (fileset_type IN LISTS fileset_types)
  foreach (scope IN LISTS scopes)
    run_cmake("FileSet${fileset_type}${scope}")
  endforeach ()

  # Test the error message when a non-C++ source file is found in the source
  # list.
  run_cmake("NotCXXSource${fileset_type}")
endforeach ()
