include(RunCMake)

set(stdlib_custom_json)
if (CMake_TEST_CXX_STDLIB_MODULES_JSON)
  list(APPEND stdlib_custom_json
    -DCMAKE_CXX_STDLIB_MODULES_JSON=${CMake_TEST_CXX_STDLIB_MODULES_JSON})
endif ()

run_cmake(Inspect ${stdlib_custom_json})
include("${RunCMake_BINARY_DIR}/Inspect-build/info.cmake")

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
  # Bail; the remaining tests require the generator to successfully generate
  # with C++20 modules in the source list.
  return ()
endif ()

# This performs actual compilation tests; avoid it when not requested.
if (NOT CMake_TEST_MODULE_COMPILATION)
  return ()
endif ()

# Module compilation features:
# Compiler-based:
# - `named`: basic support for named modules is available
# - `shared`: shared libraries are supported
# - `partitions`: module partitions are supported
# - `internal_partitions`: internal module partitions are supported
# - `bmionly`: the compiler supports BMI-only builds
# - `import_std23`: the compiler supports `import std` for C++23
#
# Generator-based:
# - `compile_commands`: the generator supports `compile_commands.json`
# - `collation`: the generator supports module collation features
# - `export_bmi`: the generator supports exporting BMIs
# - `ninja`: a `ninja` binary is available to perform `Ninja`-only testing
#   (assumed if the generator matches `Ninja`).
string(REPLACE "," ";" CMake_TEST_MODULE_COMPILATION "${CMake_TEST_MODULE_COMPILATION}")
if (RunCMake_GENERATOR MATCHES "Ninja")
  list(APPEND CMake_TEST_MODULE_COMPILATION
    "ninja")
endif ()

function (run_cxx_module_test directory)
  set(test_name "${directory}")
  if (NOT ARGN STREQUAL "")
    list(POP_FRONT ARGN test_name)
  endif ()

  set(RunCMake_TEST_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/${directory}")
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${test_name}-build")

  if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake_TEST_OPTIONS -DCMAKE_CONFIGURATION_TYPES=Debug)
  else ()
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
  endif ()
  if (directory MATCHES "imp-std")
    list(APPEND RunCMake_TEST_OPTIONS
      ${stdlib_custom_json})
  endif ()

  if (RunCMake_CXXModules_INSTALL)
    set(prefix "${RunCMake_BINARY_DIR}/${test_name}-install")
    file(REMOVE_RECURSE "${prefix}")
    list(APPEND RunCMake_TEST_OPTIONS
      "-DCMAKE_INSTALL_PREFIX=${prefix}")
  endif ()

  list(APPEND RunCMake_TEST_OPTIONS
    "-DCMake_TEST_MODULE_COMPILATION_RULES=${CMake_TEST_MODULE_COMPILATION_RULES}"
    ${ARGN})
  run_cmake("${test_name}")
  set(RunCMake_TEST_NO_CLEAN 1)
  if (RunCMake_CXXModules_TARGET)
    run_cmake_command("${test_name}-build" "${CMAKE_COMMAND}" --build . --config Debug --target "${RunCMake_CXXModules_TARGET}")
  else ()
    run_cmake_command("${test_name}-build" "${CMAKE_COMMAND}" --build . --config Debug)
    foreach (RunCMake_CXXModules_TARGET IN LISTS RunCMake_CXXModules_TARGETS)
      set(RunCMake_CXXModules_CONFIG "Debug")
      set(RunCMake_CXXModules_NAME_SUFFIX "")
      if (RunCMake_CXXModules_TARGET MATCHES "(.*)@(.*)")
        set(RunCMake_CXXModules_TARGET "${CMAKE_MATCH_1}")
        set(RunCMake_CXXModules_CONFIG "${CMAKE_MATCH_2}")
        set(RunCMake_CXXModules_NAME_SUFFIX "-${RunCMake_CXXModules_CONFIG}")
      endif ()
      run_cmake_command("${test_name}-target-${RunCMake_CXXModules_TARGET}${RunCMake_CXXModules_NAME_SUFFIX}" "${CMAKE_COMMAND}" --build . --target "${RunCMake_CXXModules_TARGET}" --config "${RunCMake_CXXModules_CONFIG}")
    endforeach ()
  endif ()
  if (RunCMake_CXXModules_INSTALL)
    run_cmake_command("${test_name}-install" "${CMAKE_COMMAND}" --build . --target install --config Debug)
  endif ()
  if (NOT RunCMake_CXXModules_NO_TEST)
    run_cmake_command("${test_name}-test" "${CMAKE_CTEST_COMMAND}" -C Debug --output-on-failure)
  endif ()
  if (RunCMake_CXXModules_REBUILD)
    execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 1.125) # handle 1s resolution
    include("${RunCMake_TEST_SOURCE_DIR}/pre-rebuild.cmake")
    execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 1.125) # handle 1s resolution
    run_cmake_command("${test_name}-rebuild" "${CMAKE_COMMAND}" --build . --config Debug)
  endif ()
endfunction ()

function (run_cxx_module_test_target directory target)
  set(RunCMake_CXXModules_TARGET "${target}")
  set(RunCMake_CXXModules_NO_TEST 1)
  run_cxx_module_test("${directory}" ${ARGN})
endfunction ()

function (run_cxx_module_test_rebuild directory)
  set(RunCMake_CXXModules_INSTALL 0)
  set(RunCMake_CXXModules_NO_TEST 1)
  set(RunCMake_CXXModules_REBUILD 1)
  run_cxx_module_test("${directory}" ${ARGN})
endfunction ()

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
  run_cxx_module_test(file-sets-with-dot)
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
  run_cxx_module_test(imp-from-object)
  run_cxx_module_test(circular)
  run_cxx_module_test(try-compile)
  run_cxx_module_test(try-run)
  unset(RunCMake_CXXModules_NO_TEST)
  run_cxx_module_test(same-src-name)
  run_cxx_module_test(scan_props)
  run_cxx_module_test(target-objects)

  # mixed-bmi-compatibility requires a generator that implements per-importer
  # BMI generation
  if ("cxx_std_23" IN_LIST CMAKE_CXX_COMPILE_FEATURES AND
      RunCMake_GENERATOR MATCHES "Ninja")
    run_cxx_module_test(mixed-bmi-compatibility)
  endif()

  if ("cxx_std_23" IN_LIST CMAKE_CXX_COMPILE_FEATURES AND
      "import_std23" IN_LIST CMake_TEST_MODULE_COMPILATION)
    run_cxx_module_test(imp-std)
    set(RunCMake_CXXModules_NO_TEST 1)
    run_cxx_module_test(imp-std-no-std-prop)
    unset(RunCMake_CXXModules_NO_TEST)
    run_cxx_module_test(imp-std-exp-no-std-build)
    set(RunCMake_CXXModules_INSTALL 1)
    run_cxx_module_test(imp-std-exp-no-std-install)
    unset(RunCMake_CXXModules_INSTALL)

    if ("collation" IN_LIST CMake_TEST_MODULE_COMPILATION)
      run_cxx_module_test(imp-std-not-in-exp-build)
      run_cxx_module_test(imp-std-trans imp-std-trans-not-in-exp-build "-DCMAKE_PREFIX_PATH=${RunCMake_BINARY_DIR}/imp-std-not-in-exp-build-build")

      set(RunCMake_CXXModules_INSTALL 1)
      run_cxx_module_test(imp-std-not-in-exp-install)
      unset(RunCMake_CXXModules_INSTALL)
      run_cxx_module_test(imp-std-trans imp-std-trans-not-in-exp-install "-DCMAKE_PREFIX_PATH=${RunCMake_BINARY_DIR}/imp-std-not-in-exp-install-install")

      run_cxx_module_test(imp-std-trans imp-std-trans-exp-no-std-build "-DCMAKE_PREFIX_PATH=${RunCMake_BINARY_DIR}/imp-std-exp-no-std-build-build" -DEXPORT_NO_STD=1)
      run_cxx_module_test(imp-std-trans imp-std-trans-exp-no-std-install "-DCMAKE_PREFIX_PATH=${RunCMake_BINARY_DIR}/imp-std-exp-no-std-install-install" -DEXPORT_NO_STD=1)
    endif ()
  endif ()
endif ()

# Tests which require compile commands support.
if ("compile_commands" IN_LIST CMake_TEST_MODULE_COMPILATION)
  run_cxx_module_test(exp-compile-commands)
endif ()

macro (setup_export_build_database_targets)
  set(RunCMake_CXXModules_TARGETS
    cmake_build_database-CXX
    cmake_build_database)

  if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
    list(INSERT RunCMake_CXXModules_TARGETS 0
      cmake_build_database-CXX-Debug
      cmake_build_database-Debug
      # Other config targets.
      cmake_build_database-CXX-Release@Release
      cmake_build_database-Release@Release)
  endif ()
endmacro ()

# Tests which require build database support.
if ("build_database" IN_LIST CMake_TEST_MODULE_COMPILATION)
  setup_export_build_database_targets()
  set(RunCMake_CXXModules_NO_TEST 1)

  run_cxx_module_test(exp-builddb)

  unset(RunCMake_CXXModules_NO_TEST)
  unset(RunCMake_CXXModules_TARGETS)
endif ()

# Tests which require collation work.
if ("collation" IN_LIST CMake_TEST_MODULE_COMPILATION)
  run_cxx_module_test(duplicate-sources)
  run_cxx_module_test(public-req-priv)
  set(RunCMake_CXXModules_NO_TEST 1)
  run_cxx_module_test(req-priv-other-target)
  unset(RunCMake_CXXModules_NO_TEST)
  run_cxx_module_test_rebuild(depchain-modmap)
  run_cxx_module_test_rebuild(depchain-mods-json-file)
  if (RunCMake_GENERATOR MATCHES "Ninja")
    run_cxx_module_test_rebuild(depchain-collation-restat)
  endif ()
endif ()

# Tests which use named modules in shared libraries.
if ("shared" IN_LIST CMake_TEST_MODULE_COMPILATION)
  run_cxx_module_test(library library-shared -DBUILD_SHARED_LIBS=ON)
  run_cxx_module_test(shared-library-symbol-visibility)
endif ()

# Tests which use partitions.
if ("partitions" IN_LIST CMake_TEST_MODULE_COMPILATION)
  run_cxx_module_test(partitions)
endif ()

# Tests which use internal partitions.
if ("internal_partitions" IN_LIST CMake_TEST_MODULE_COMPILATION)
  run_cxx_module_test(internal-partitions)
endif ()

function (run_cxx_module_import_test type name)
  set(RunCMake_CXXModules_INSTALL 0)

  if ("EXPORT_BUILD_DATABASE" IN_LIST ARGN)
    list(REMOVE_ITEM ARGN EXPORT_BUILD_DATABASE)
    list(APPEND ARGN -DCMAKE_EXPORT_BUILD_DATABASE=1)
  endif ()

  run_cxx_module_test(imp-mods "imp-mods-${name}" "-DCMAKE_PREFIX_PATH=${RunCMake_BINARY_DIR}/${name}-${type}" ${ARGN})
endfunction ()

set(export_cps "-DCMAKE_EXPERIMENTAL_EXPORT_PACKAGE_INFO=7fa7d13b-8308-4dc7-af39-9e450456d68f")
set(import_cps "-DCMAKE_EXPERIMENTAL_FIND_CPS_PACKAGES=e82e467b-f997-4464-8ace-b00808fff261")

# Tests which install BMIs
if ("export_bmi" IN_LIST CMake_TEST_MODULE_COMPILATION)
  run_cxx_module_test(exp-iface-no-props-build)
  run_cxx_module_test(exp-iface-build exp-iface-build ${export_cps})
  run_cxx_module_test(exp-incdirs-build exp-incdirs-build ${export_cps})
  run_cxx_module_test(exp-incdirs-old-cmake-build)
  run_cxx_module_test(exp-usage-build)
  run_cxx_module_test(exp-bmi-and-iface-build)
  run_cxx_module_test(exp-command-sepdir-build exp-command-sepdir-build ${export_cps})
  run_cxx_module_test(exp-trans-targets-build)
  run_cxx_module_test(exp-trans-mods1-build exp-trans-mods1-build ${export_cps})
  run_cxx_module_test(exp-trans-mods-build exp-trans-mods-build "-DCMAKE_PREFIX_PATH=${RunCMake_BINARY_DIR}/exp-trans-mods1-build-build" "-Dexport_transitive_modules1_cps_DIR=${RunCMake_BINARY_DIR}/exp-trans-mods1-build-build" ${export_cps} ${import_cps})
  run_cxx_module_test(exp-with-headers-build exp-with-headers-build ${export_cps})

  if ("collation" IN_LIST CMake_TEST_MODULE_COMPILATION AND
      "bmionly" IN_LIST CMake_TEST_MODULE_COMPILATION)
    run_cxx_module_import_test(build exp-iface-build)
    run_cxx_module_import_test(build exp-iface-no-props-build -DNO_PROPERTIES=1)
    run_cxx_module_import_test(build exp-incdirs-build -DINCLUDE_PROPERTIES=1)
    run_cxx_module_import_test(build exp-bmi-and-iface-build -DWITH_BMIS=1)
    run_cxx_module_import_test(build exp-command-sepdir-build -DEXPORT_COMMAND_SEPDIR=1)
    run_cxx_module_import_test(build exp-trans-targets-build -DTRANSITIVE_TARGETS=1)
    run_cxx_module_import_test(build exp-trans-mods-build -DTRANSITIVE_MODULES=1)
    run_cxx_module_import_test(build exp-with-headers-build -DWITH_HEADERS=1)

    if ("build_database" IN_LIST CMake_TEST_MODULE_COMPILATION)
      setup_export_build_database_targets()

      run_cxx_module_import_test(build exp-builddb -DBUILD_DATABASE=1 EXPORT_BUILD_DATABASE)

      unset(RunCMake_CXXModules_TARGETS)
    endif ()
  endif ()
endif ()

# All of the following tests perform installation.
set(RunCMake_CXXModules_INSTALL 1)

# Tests which install BMIs
if ("install_bmi" IN_LIST CMake_TEST_MODULE_COMPILATION)
  run_cxx_module_test(install-bmi)
  run_cxx_module_test(install-bmi-and-ifaces)

  if ("export_bmi" IN_LIST CMake_TEST_MODULE_COMPILATION)
    run_cxx_module_test(exp-iface-no-props-install)
    run_cxx_module_test(exp-iface-install exp-iface-install ${export_cps})
    run_cxx_module_test(exp-incdirs-install exp-incdirs-install ${export_cps})
    run_cxx_module_test(exp-incdirs-old-cmake-install)
    run_cxx_module_test(exp-usage-install)
    run_cxx_module_test(exp-bmi-and-iface-install)
    run_cxx_module_test(exp-command-sepdir-install exp-command-sepdir-install ${export_cps})
    run_cxx_module_test(exp-trans-targets-install)
    run_cxx_module_test(exp-trans-mods1-install exp-trans-mods1-install ${export_cps})
    run_cxx_module_test(exp-trans-mods-install exp-trans-mods-install "-DCMAKE_PREFIX_PATH=${RunCMake_BINARY_DIR}/exp-trans-mods1-install-install" "-Dexport_transitive_modules1_cps_DIR=${RunCMake_BINARY_DIR}/exp-trans-mods1-install-install/lib/cmake/export_transitive_modules1_cps" ${export_cps} ${import_cps})
    run_cxx_module_test(exp-with-headers-install exp-with-headers-install ${export_cps})

    if ("collation" IN_LIST CMake_TEST_MODULE_COMPILATION AND
        "bmionly" IN_LIST CMake_TEST_MODULE_COMPILATION)
      run_cxx_module_import_test(install exp-iface-install)
      run_cxx_module_import_test(install exp-iface-no-props-install -DNO_PROPERTIES=1)
      run_cxx_module_import_test(install exp-incdirs-install -DINCLUDE_PROPERTIES=1)
      run_cxx_module_import_test(install exp-bmi-and-iface-install -DWITH_BMIS=1)
      run_cxx_module_import_test(install exp-command-sepdir-install -DEXPORT_COMMAND_SEPDIR=1)
      run_cxx_module_import_test(install exp-trans-targets-install -DTRANSITIVE_TARGETS=1)
      run_cxx_module_import_test(install exp-trans-mods-install -DTRANSITIVE_MODULES=1)
      run_cxx_module_import_test(install exp-with-headers-install -DWITH_HEADERS=1)
    endif ()
  endif ()
endif ()

# All remaining tests require a working `Ninja` generator to set up a test case
# for the current generator.
if (NOT "ninja" IN_LIST CMake_TEST_MODULE_COMPILATION)
  return ()
endif ()
# All remaining tests require `bmionly` in order to consume from the `ninja`
# build.
if (NOT "bmionly" IN_LIST CMake_TEST_MODULE_COMPILATION)
  return ()
endif ()

function (run_cxx_module_test_ninja directory)
  set(RunCMake_GENERATOR "Ninja")
  set(RunCMake_CXXModules_NO_TEST 1)
  set(RunCMake_CXXModules_INSTALL 1)
  # `Ninja` is not a multi-config generator.
  set(RunCMake_GENERATOR_IS_MULTI_CONFIG 0)
  run_cxx_module_test("${directory}" "${directory}-ninja" ${ARGN})
endfunction ()

# Installation happens within `run_cxx_module_test_ninja`.
set(RunCMake_CXXModules_INSTALL 0)

set(test_set mods-from-ninja)
run_cxx_module_test_ninja("exp-${test_set}")
run_cxx_module_test(imp-mods "imp-${test_set}" "-DCMAKE_PREFIX_PATH=${RunCMake_BINARY_DIR}/exp-${test_set}-ninja-install" -DFROM_NINJA=1)
