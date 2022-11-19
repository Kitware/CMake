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
  run_cmake("FileSet${fileset_type}InterfaceImported")

  # Test the error message when a non-C++ source file is found in the source
  # list.
  run_cmake("NotCXXSource${fileset_type}")
endforeach ()

run_cmake(InstallBMI)
run_cmake(InstallBMIGenericArgs)
run_cmake(InstallBMIIgnore)

run_cmake(ExportBuildCxxModules)
run_cmake(ExportInstallCxxModules)

# Generator-specific tests.
if (RunCMake_GENERATOR MATCHES "Ninja")
  run_cmake(NinjaDependInfoFileSet)
  run_cmake(NinjaDependInfoExport)
  run_cmake(NinjaDependInfoBMIInstall)
else ()
  message(FATAL_ERROR
    "Please add 'DependInfo' tests for the '${RunCMake_GENERATOR}' generator.")
endif ()

# Actual compilation tests.
if (NOT CMake_TEST_MODULE_COMPILATION)
  return ()
endif ()

function (run_cxx_module_test directory)
  set(test_name "${directory}")
  if (NOT ARGN STREQUAL "")
    list(POP_FRONT ARGN test_name)
  endif ()

  set(RunCMake_TEST_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/examples/${directory}")
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/examples/${test_name}-build")

  if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake_TEST_OPTIONS -DCMAKE_CONFIGURATION_TYPES=Debug)
  else ()
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
  endif ()

  if (RunCMake_CXXModules_INSTALL)
    set(prefix "${RunCMake_BINARY_DIR}/examples/${test_name}-install")
    file(REMOVE_RECURSE "${prefix}")
    list(APPEND RunCMake_TEST_OPTIONS
      "-DCMAKE_INSTALL_PREFIX=${prefix}")
  endif ()

  list(APPEND RunCMake_TEST_OPTIONS
    "-DCMake_TEST_MODULE_COMPILATION_RULES=${CMake_TEST_MODULE_COMPILATION_RULES}"
    ${ARGN})
  run_cmake("examples/${test_name}")
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command("examples/${test_name}-build" "${CMAKE_COMMAND}" --build . --config Debug)
  if (RunCMake_CXXModules_INSTALL)
    run_cmake_command("examples/${test_name}-install" "${CMAKE_COMMAND}" --build . --target install --config Debug)
  endif ()
  run_cmake_command("examples/${test_name}-test" "${CMAKE_CTEST_COMMAND}" -C Debug --output-on-failure)
endfunction ()

string(REPLACE "," ";" CMake_TEST_MODULE_COMPILATION "${CMake_TEST_MODULE_COMPILATION}")

# Tests which use named modules.
if ("named" IN_LIST CMake_TEST_MODULE_COMPILATION)
  run_cxx_module_test(simple)
  run_cxx_module_test(library library-static -DBUILD_SHARED_LIBS=OFF)
  run_cxx_module_test(generated)
  run_cxx_module_test(public-req-private)
  run_cxx_module_test(deep-chain)
endif ()

# Tests which use named modules in shared libraries.
if ("shared" IN_LIST CMake_TEST_MODULE_COMPILATION)
  run_cxx_module_test(library library-shared -DBUILD_SHARED_LIBS=ON)
endif ()

# Tests which use partitions.
if ("partitions" IN_LIST CMake_TEST_MODULE_COMPILATION)
  run_cxx_module_test(partitions)
endif ()

# Tests which use internal partitions.
if ("internal_partitions" IN_LIST CMake_TEST_MODULE_COMPILATION)
  run_cxx_module_test(internal-partitions)
endif ()

# Tests which install BMIs
if ("export_bmi" IN_LIST CMake_TEST_MODULE_COMPILATION)
  run_cxx_module_test(export-interface-build)
  run_cxx_module_test(export-bmi-and-interface-build)
endif ()

# All of the following tests perform installation.
set(RunCMake_CXXModules_INSTALL 1)

# Tests which install BMIs
if ("install_bmi" IN_LIST CMake_TEST_MODULE_COMPILATION)
  run_cxx_module_test(install-bmi)
  run_cxx_module_test(install-bmi-and-interfaces)

  if ("export_bmi" IN_LIST CMake_TEST_MODULE_COMPILATION)
    run_cxx_module_test(export-interface-install)
    run_cxx_module_test(export-bmi-and-interface-install)
  endif ()
endif ()
