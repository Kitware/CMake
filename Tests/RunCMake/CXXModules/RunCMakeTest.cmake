include(RunCMake)

# For `if (IN_LIST)`
cmake_policy(SET CMP0057 NEW)

run_cmake(Inspect)
include("${RunCMake_BINARY_DIR}/Inspect-build/info.cmake")

# Test negative cases where C++20 modules do not work.
run_cmake(NoCXX)
if ("cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
  # This test requires that the compiler be told to compile in an older-than-20
  # standard. If the compiler forces a standard to be used, skip it.
  if (NOT forced_cxx_standard)
    run_cmake(NoCXX20)
    if(CMAKE_CXX_STANDARD_DEFAULT AND CMAKE_CXX20_STANDARD_COMPILE_OPTION)
      run_cmake_with_options(ImplicitCXX20 -DCMAKE_CXX20_STANDARD_COMPILE_OPTION=${CMAKE_CXX20_STANDARD_COMPILE_OPTION})
    endif()
  endif ()

  run_cmake(NoScanningSourceFileProperty)
  run_cmake(NoScanningTargetProperty)
  run_cmake(NoScanningVariable)
  run_cmake(CMP0155-OLD)
  run_cmake(CMP0155-NEW)
  run_cmake(CMP0155-NEW-with-rule)
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

set(generator_supports_cxx_modules 0)
if (RunCMake_GENERATOR MATCHES "Ninja" AND
    ninja_version VERSION_GREATER_EQUAL "1.11" AND
    "cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
  set(generator_supports_cxx_modules 1)
endif ()

if (RunCMake_GENERATOR MATCHES "Visual Studio" AND
    CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.34")
  set(generator_supports_cxx_modules 1)
endif ()

# Test behavior when the generator does not support C++20 modules.
if (NOT generator_supports_cxx_modules)
  if ("cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    run_cmake(NoDyndepSupport)
  endif ()

  # Bail; the remaining tests require the generator to successfully generate
  # with C++20 modules in the source list.
  return ()
endif ()

set(fileset_types
  Modules)
set(scopes
  Interface
  Private
  Public)
foreach (fileset_type IN LISTS fileset_types)
  foreach (scope IN LISTS scopes)
    run_cmake("FileSet${fileset_type}${scope}")
  endforeach ()
  run_cmake("FileSet${fileset_type}InterfaceImported")

  # Test the error messages when a non-C++ source file is found in the source
  # list.
  run_cmake("NotCompiledSource${fileset_type}")
  run_cmake("NotCXXSource${fileset_type}")
endforeach ()

run_cmake(InstallBMI)
run_cmake(InstallBMIGenericArgs)
run_cmake(InstallBMIIgnore)

run_cmake(ExportBuildCxxModules)
run_cmake(ExportBuildCxxModulesTargets)
run_cmake(ExportInstallCxxModules)

# Generator-specific tests.
if (RunCMake_GENERATOR MATCHES "Ninja")
  run_cmake(NinjaDependInfoFileSet)
  run_cmake(NinjaDependInfoExport)
  run_cmake(NinjaDependInfoBMIInstall)
  run_cmake(NinjaForceResponseFile) # issue#25367
elseif (RunCMake_GENERATOR MATCHES "Visual Studio")
  run_cmake(VisualStudioNoSyntheticTargets)
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
  if (RunCMake_CXXModules_TARGET)
    run_cmake_command("examples/${test_name}-build" "${CMAKE_COMMAND}" --build . --config Debug --target "${RunCMake_CXXModules_TARGET}")
  else ()
    run_cmake_command("examples/${test_name}-build" "${CMAKE_COMMAND}" --build . --config Debug)
  endif ()
  if (RunCMake_CXXModules_INSTALL)
    run_cmake_command("examples/${test_name}-install" "${CMAKE_COMMAND}" --build . --target install --config Debug)
  endif ()
  if (NOT RunCMake_CXXModules_NO_TEST)
    run_cmake_command("examples/${test_name}-test" "${CMAKE_CTEST_COMMAND}" -C Debug --output-on-failure)
  endif ()
endfunction ()

function (run_cxx_module_test_target directory target)
  set(RunCMake_CXXModules_TARGET "${target}")
  set(RunCMake_CXXModules_NO_TEST 1)
  run_cxx_module_test("${directory}" ${ARGN})
endfunction ()

string(REPLACE "," ";" CMake_TEST_MODULE_COMPILATION "${CMake_TEST_MODULE_COMPILATION}")

if (RunCMake_GENERATOR MATCHES "Ninja")
  if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(ninja_cmp0154_target "CMakeFiles/ninja_cmp0154.dir/Debug/unrelated.cxx${CMAKE_CXX_OUTPUT_EXTENSION}")
  else ()
    set(ninja_cmp0154_target "CMakeFiles/ninja_cmp0154.dir/unrelated.cxx${CMAKE_CXX_OUTPUT_EXTENSION}")
  endif ()
  run_cxx_module_test_target(ninja-cmp0154 "${ninja_cmp0154_target}")
endif ()

run_cxx_module_test(scan-with-pch)

# Tests which use named modules.
if ("named" IN_LIST CMake_TEST_MODULE_COMPILATION)
  run_cxx_module_test(simple)
  run_cxx_module_test(vs-without-flags)
  run_cxx_module_test(library library-static -DBUILD_SHARED_LIBS=OFF)
  run_cxx_module_test(unity-build)
  run_cxx_module_test(object-library)
  run_cxx_module_test(generated)
  run_cxx_module_test(deep-chain)
  run_cxx_module_test(non-trivial-collation-order)
  run_cxx_module_test(non-trivial-collation-order-randomized)
  run_cxx_module_test(duplicate)
  set(RunCMake_CXXModules_NO_TEST 1)
  run_cxx_module_test(import-from-object)
  run_cxx_module_test(circular)
  run_cxx_module_test(try-compile)
  run_cxx_module_test(try-run)
  unset(RunCMake_CXXModules_NO_TEST)
  run_cxx_module_test(same-src-name)
  run_cxx_module_test(scan_properties)
endif ()

# Tests which require compile commands support.
if ("compile_commands" IN_LIST CMake_TEST_MODULE_COMPILATION)
  run_cxx_module_test(export-compile-commands)
endif ()

# Tests which require collation work.
if ("collation" IN_LIST CMake_TEST_MODULE_COMPILATION)
  run_cxx_module_test(public-req-private)
  set(RunCMake_CXXModules_NO_TEST 1)
  run_cxx_module_test(req-private-other-target)
  unset(RunCMake_CXXModules_NO_TEST)
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
  run_cxx_module_test(export-interface-no-properties-build)
  run_cxx_module_test(export-interface-build)
  run_cxx_module_test(export-include-directories-build)
  run_cxx_module_test(export-usage-build)
  run_cxx_module_test(export-bmi-and-interface-build)

  if ("collation" IN_LIST CMake_TEST_MODULE_COMPILATION AND
      "bmionly" IN_LIST CMake_TEST_MODULE_COMPILATION)
    set(test_suffix export-interface-build)
    run_cxx_module_test(import-modules "import-modules-${test_suffix}" "-DCMAKE_PREFIX_PATH=${RunCMake_BINARY_DIR}/examples/${test_suffix}-build")

    set(test_suffix export-interface-no-properties-build)
    run_cxx_module_test(import-modules "import-modules-${test_suffix}" "-DCMAKE_PREFIX_PATH=${RunCMake_BINARY_DIR}/examples/${test_suffix}-build" -DNO_PROPERTIES=1)

    set(test_suffix export-include-directories-build)
    run_cxx_module_test(import-modules "import-modules-${test_suffix}" "-DCMAKE_PREFIX_PATH=${RunCMake_BINARY_DIR}/examples/${test_suffix}-build" -DINCLUDE_PROPERTIES=1)

    set(test_suffix export-bmi-and-interface-build)
    run_cxx_module_test(import-modules "import-modules-${test_suffix}" "-DCMAKE_PREFIX_PATH=${RunCMake_BINARY_DIR}/examples/${test_suffix}-build" -DWITH_BMIS=1)
  endif ()
endif ()

# All of the following tests perform installation.
set(RunCMake_CXXModules_INSTALL 1)

# Tests which install BMIs
if ("install_bmi" IN_LIST CMake_TEST_MODULE_COMPILATION)
  run_cxx_module_test(install-bmi)
  run_cxx_module_test(install-bmi-and-interfaces)

  if ("export_bmi" IN_LIST CMake_TEST_MODULE_COMPILATION)
    run_cxx_module_test(export-interface-no-properties-install)
    run_cxx_module_test(export-interface-install)
    run_cxx_module_test(export-include-directories-install)
    run_cxx_module_test(export-usage-install)
    run_cxx_module_test(export-bmi-and-interface-install)

    if ("collation" IN_LIST CMake_TEST_MODULE_COMPILATION AND
        "bmionly" IN_LIST CMake_TEST_MODULE_COMPILATION)
      set(RunCMake_CXXModules_INSTALL 0)
      set(test_suffix export-interface-install)
      run_cxx_module_test(import-modules "import-modules-${test_suffix}" "-DCMAKE_PREFIX_PATH=${RunCMake_BINARY_DIR}/examples/${test_suffix}-install")

      set(test_suffix export-interface-no-properties-install)
      run_cxx_module_test(import-modules "import-modules-${test_suffix}" "-DCMAKE_PREFIX_PATH=${RunCMake_BINARY_DIR}/examples/${test_suffix}-install" -DNO_PROPERTIES=1)

      set(test_suffix export-include-directories-install)
      run_cxx_module_test(import-modules "import-modules-${test_suffix}" "-DCMAKE_PREFIX_PATH=${RunCMake_BINARY_DIR}/examples/${test_suffix}-install" -DINCLUDE_PROPERTIES=1)

      set(test_suffix export-bmi-and-interface-install)
      run_cxx_module_test(import-modules "import-modules-${test_suffix}" "-DCMAKE_PREFIX_PATH=${RunCMake_BINARY_DIR}/examples/${test_suffix}-install" -DWITH_BMIS=1)
      set(RunCMake_CXXModules_INSTALL 1)
    endif ()
  endif ()
endif ()
