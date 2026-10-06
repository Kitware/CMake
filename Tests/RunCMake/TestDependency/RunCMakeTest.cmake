include(RunCMake)

# Verify test_prep behavior
function(run_testdependency_case CASE_NAME EXPECT_PRESENT)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${CASE_NAME}-build)

  # 1. Configure
  run_cmake(${CASE_NAME})
  set(RunCMake_TEST_NO_CLEAN 1)

  # 2. Verify build targets exist
  run_cmake_command(${CASE_NAME}-check
    ${CMAKE_COMMAND}
    -DRunCMake_TEST_BINARY_DIR=${RunCMake_TEST_BINARY_DIR}
    -Dexpect_present=${EXPECT_PRESENT}
    -P ${RunCMake_SOURCE_DIR}/check-targets.cmake)

  # If build targets enabled...
  if (EXPECT_PRESENT)

    # Build test_prep target
    run_cmake_command(${CASE_NAME}-build
      ${CMAKE_COMMAND} --build . ${TestDependency_BUILD_CONFIG_ARG}
      --target test_prep/TargetBuildTest)

    # Verify building test_prep target built test dependencies
    run_cmake_command(${CASE_NAME}-build-check
      ${CMAKE_COMMAND}
      -DRunCMake_TEST_BINARY_DIR=${RunCMake_TEST_BINARY_DIR}
      -P ${RunCMake_SOURCE_DIR}/build-check.cmake)
  endif()
  unset(RunCMake_TEST_NO_CLEAN)
endfunction()

# Ensure generated test file includes BUILD_DEPENDs
function(check_discovered_build_depends CASE_NAME TESTS_FILE)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${CASE_NAME}-build-depends
    ${CMAKE_COMMAND}
    -Dtests_file=${TESTS_FILE}
    -Ddependency_manifest=${TestDependency_DEPENDENCY_MANIFEST}
    -P ${RunCMake_SOURCE_DIR}/check-discovered-build-depends.cmake)
  unset(RunCMake_TEST_NO_CLEAN)
endfunction()

# Find discovered test file for GTest
function(find_discovered_tests_file OUT_VAR BINARY_DIR TARGET_NAME)
  file(GLOB discovered_tests_files
    "${BINARY_DIR}/${TARGET_NAME}*_tests.cmake")
  list(FILTER discovered_tests_files EXCLUDE REGEX [[.*_include\.cmake$]])
  list(LENGTH discovered_tests_files discovered_tests_files_length)
  if(NOT discovered_tests_files_length EQUAL 1)
    message(FATAL_ERROR
      "Expected exactly one discovered tests file for ${TARGET_NAME}, got: ${discovered_tests_files}")
  endif()
  set(${OUT_VAR} "${discovered_tests_files}" PARENT_SCOPE)
endfunction()

# Verify ctest --out-of-date behavior
function(run_discover_out_of_date_case CASE_NAME TARGET_NAME EXPECTED_TESTS_REGEX)
  cmake_parse_arguments(arg
    "ADD_TEST;GTEST"
    ""
    ""
    ${ARGN})

  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${CASE_NAME}-build)

  # 1. Configure
  run_cmake(${CASE_NAME})
  set(RunCMake_TEST_NO_CLEAN 1)

  # 2. Build dependencies needed for the tests.
  run_cmake_command(${CASE_NAME}-build
    ${CMAKE_COMMAND} --build . ${TestDependency_BUILD_CONFIG_ARG}
    --target test_prep/all)

  # 3. Run ctest once, including any PRE_TEST discovery.
  # Ensure test stamps are newer than build dependencies even on filesystems
  # with one-second timestamp resolution.  Equal timestamps are out of date.
  run_cmake_command(${CASE_NAME}-build-sleep
    ${CMAKE_COMMAND} -E sleep 1.1)
  set(RunCMake_TEST_EXPECT_stderr ".*")
  run_cmake_command(${CASE_NAME}-initial
    ${CMAKE_CTEST_COMMAND} ${TestDependency_CTEST_CONFIG_ARG})

  # 4. Check that generated files include dependencies.
  if(arg_GTEST)
    find_discovered_tests_file(discovered_tests_file
      ${RunCMake_TEST_BINARY_DIR} ${TARGET_NAME})
    check_discovered_build_depends(${CASE_NAME} ${discovered_tests_file})
  else()
    set(discovered_tests_file
      "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake")
    check_discovered_build_depends(${CASE_NAME} ${discovered_tests_file})
  endif()

  if(TestDependency_DEPENDENCY_MANIFEST)
    set(RunCMake_TEST_EXPECT_stdout "Total Tests: 0")
    run_cmake_command(${CASE_NAME}-up-to-date
      ${CMAKE_CTEST_COMMAND} ${TestDependency_CTEST_CONFIG_ARG} --out-of-date -N)
    unset(RunCMake_TEST_EXPECT_stdout)
  endif()

  # 5. Touch the actual generated artifact, after the last test timestamp.
  file(READ "${RunCMake_TEST_BINARY_DIR}/${TARGET_NAME}-${TestDependency_CONFIG}.txt"
    dependent_files)
  foreach(dependent_file IN LISTS dependent_files)
    if(NOT EXISTS "${dependent_file}")
      message(FATAL_ERROR "Dependency was not built: ${dependent_file}")
    endif()
  endforeach()
  run_cmake_command(${CASE_NAME}-sleep
    ${CMAKE_COMMAND} -E sleep 1.1)
  run_cmake_command(${CASE_NAME}-touch
    ${CMAKE_COMMAND} -E touch ${dependent_files})

  # 6. Run ctest --out-of-date and ensure test reruns
  set(RunCMake_TEST_EXPECT_stdout
    "${EXPECTED_TESTS_REGEX}")
  run_cmake_command(${CASE_NAME}-out-of-date
    ${CMAKE_CTEST_COMMAND} ${TestDependency_CTEST_CONFIG_ARG} --out-of-date)
  unset(RunCMake_TEST_NO_CLEAN)
  unset(RunCMake_TEST_EXPECT_stderr)
  unset(RunCMake_TEST_EXPECT_stdout)
endfunction()

if(RunCMake_GENERATOR MATCHES "Ninja|FASTBuild|Makefiles")
  block()
    # Use the same configuration for generated manifests, builds, and CTest,
    # including single-config generators whose default build type varies.
    set(TestDependency_CONFIG Debug)
    set(TestDependency_BUILD_CONFIG_ARG --config ${TestDependency_CONFIG})
    set(TestDependency_CTEST_CONFIG_ARG -C ${TestDependency_CONFIG})
    if(NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
      list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=${TestDependency_CONFIG})
    endif()

    # Don't run tests with missing dependencies
    block()
      set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/MissingBuildDependency-build)
      run_cmake(MissingBuildDependency)
      set(RunCMake_TEST_NO_CLEAN 1)
      block()
        set(RunCMake_TEST_EXPECT_RESULT 8)
        set(RunCMake_TEST_EXPECT_stdout "Direct[^\n]*Not Run.*Discovered[^\n]*Not Run")
        set(RunCMake_TEST_EXPECT_stderr "Unable to find build dependency: [^\n]*dependency.txt")
        run_cmake_command(MissingBuildDependency-test
          ${CMAKE_CTEST_COMMAND} ${TestDependency_CTEST_CONFIG_ARG})
        run_cmake_command(MissingBuildDependency-out-of-date
          ${CMAKE_CTEST_COMMAND} ${TestDependency_CTEST_CONFIG_ARG} --out-of-date)
      endblock()
      file(WRITE "${RunCMake_TEST_BINARY_DIR}/dependency.txt" "ready\n")
      set(RunCMake_TEST_EXPECT_stdout "100% tests passed out of 2")
      run_cmake_command(MissingBuildDependency-available
        ${CMAKE_CTEST_COMMAND} ${TestDependency_CTEST_CONFIG_ARG} --out-of-date)
      # Missing dependencies make tests out of date even with existing stamps.
      file(REMOVE "${RunCMake_TEST_BINARY_DIR}/dependency.txt")
      set(RunCMake_TEST_EXPECT_RESULT 8)
      set(RunCMake_TEST_EXPECT_stdout "Direct[^\n]*Not Run.*Discovered[^\n]*Not Run")
      set(RunCMake_TEST_EXPECT_stderr "Unable to find build dependency: [^\n]*dependency.txt")
      run_cmake_command(MissingBuildDependency-removed
        ${CMAKE_CTEST_COMMAND} ${TestDependency_CTEST_CONFIG_ARG} --out-of-date)
    endblock()

    # "Failure" cases, do not generate test dependencies
    block()
      run_testdependency_case(DEFAULT FALSE)
      run_testdependency_case(OFF FALSE)
      run_cmake(ON-invalid-test-name)
    endblock()

    # A header-only INTERFACE library dependency is filtered out by the
    # generator-independent dependency resolution, so building the
    # test_prep target must not fail on a missing rule.
    block()
      set(RunCMake_TEST_BINARY_DIR
        ${RunCMake_BINARY_DIR}/ON-interface-build)
      run_cmake(ON-interface)
      set(RunCMake_TEST_NO_CLEAN 1)
      set(RunCMake_TEST_OUTPUT_MERGE 1)
      run_cmake_command(ON-interface-build
        ${CMAKE_COMMAND} --build . ${TestDependency_BUILD_CONFIG_ARG}
        --target test_prep/InterfaceTest)
      unset(RunCMake_TEST_OUTPUT_MERGE)
    endblock()

    # Imported dependencies are accepted without requiring a build rule.
    block()
      set(RunCMake_TEST_BINARY_DIR
        ${RunCMake_BINARY_DIR}/ON-imported-build)
      run_cmake(ON-imported)
      set(RunCMake_TEST_NO_CLEAN 1)
      set(RunCMake_TEST_OUTPUT_MERGE 1)
      run_cmake_command(ON-imported-build
        ${CMAKE_COMMAND} --build . ${TestDependency_BUILD_CONFIG_ARG}
        --target test_prep/ImportedTest)
      unset(RunCMake_TEST_OUTPUT_MERGE)
      run_cmake_command(ON-imported-test
        ${CMAKE_CTEST_COMMAND} ${TestDependency_CTEST_CONFIG_ARG}
        -R "^ImportedTest$")
    endblock()

    # Test names that differ only in case each get a working test_prep target.
    block()
      set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/ON-case-build)
      run_cmake(ON-case)
      set(RunCMake_TEST_NO_CLEAN 1)
      foreach(case IN ITEMS "Mixed;upper" "mixed;lower" "All;all")
        list(GET case 0 test)
        list(GET case 1 dependency)
        set(RunCMake-check-file ON-case-build-check.cmake)
        run_cmake_command(ON-case-${dependency}-build
          ${CMAKE_COMMAND} --build . ${TestDependency_BUILD_CONFIG_ARG}
          --target test_prep/${test})
      endforeach()
    endblock()

    # Exceptions for Makefile generator
    if(RunCMake_GENERATOR MATCHES Makefiles)
      # Diagnostics specific to the Makefile generators.
      block()
        # A ':' in a test name cannot be expressed as a Makefile target.
        set(RunCMake_TEST_BINARY_DIR
          ${RunCMake_BINARY_DIR}/ON-colon-name-build)
        run_cmake(ON-colon-name)
      endblock()
      block()
        # A byproduct file cannot be built through a single owning target.
        set(RunCMake_TEST_BINARY_DIR
          ${RunCMake_BINARY_DIR}/ON-byproduct-build)
        run_cmake(ON-byproduct)
      endblock()
      block()
        # A custom-command output owned by no target cannot be built.
        set(RunCMake_TEST_BINARY_DIR
          ${RunCMake_BINARY_DIR}/ON-orphan-build)
        run_cmake(ON-orphan)
      endblock()
    endif()

    # Verify successful behavior
    block()
      foreach(case IN ITEMS DiscoverTestsUtility DiscoverTestsCommands
          DiscoverTestsGeneratedFile DirectoryGeneratedFile)
        set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-build)
        run_cmake(${case})
        set(RunCMake_TEST_NO_CLEAN 1)
        run_cmake_command(${case}-build
          ${CMAKE_COMMAND} --build . ${TestDependency_BUILD_CONFIG_ARG}
          --target test_prep/all)
        set(RunCMake_TEST_EXPECT_stdout "100% tests passed")
        run_cmake_command(${case}-test
          ${CMAKE_CTEST_COMMAND} ${TestDependency_CTEST_CONFIG_ARG})
        unset(RunCMake_TEST_EXPECT_stdout)
        unset(RunCMake_TEST_NO_CLEAN)
      endforeach()

      if(RunCMake_GENERATOR STREQUAL "Ninja Multi-Config")
        set(RunCMake_TEST_BINARY_DIR
          ${RunCMake_BINARY_DIR}/ON-file-generate-config-build)
        run_cmake(ON-file-generate-config)
        set(RunCMake_TEST_NO_CLEAN 1)
        run_cmake_command(ON-file-generate-config-build
          ${CMAKE_COMMAND} --build . --config Debug
          --target test_prep/FileGenerateConfigTest)
        run_cmake_command(ON-file-generate-config-test
          ${CMAKE_CTEST_COMMAND} -C Debug -R "^FileGenerateConfigTest$")
        unset(RunCMake_TEST_NO_CLEAN)
      endif()

      block()
        set(TestDependency_DEPENDENCY_MANIFEST
          "${RunCMake_BINARY_DIR}/ObjectLibrary-build/dependency_objects-${TestDependency_CONFIG}.txt")
        run_discover_out_of_date_case(ObjectLibrary dependency_objects
          "OBJECT\\.direct.*OBJECT\\.case_foo.*OBJECT\\.case_bar")
      endblock()

      # Verify test_depends
      run_testdependency_case(ON TRUE)

      # Verify test_depends + --out-of-date for add_test
      run_discover_out_of_date_case(AddTest fake_discovery
        "ADD_TEST\\.command_genex.*ADD_TEST\\.command_target.*ADD_TEST\\.build_depends"
        ADD_TEST)

      # Verify test_depends + --out-of-date for discover_tests
      run_discover_out_of_date_case(DiscoverTests fake_discovery
        "DISCOVER\\.case_foo.*DISCOVER\\.case_bar")

      # Verify explicit BUILD_DEPENDS for discover_tests
      run_discover_out_of_date_case(DiscoverTestsBuildDepends fake_discovery
        "DISCOVER\\.case_foo.*DISCOVER\\.case_bar")

      # Verify test_depends + --out-of-date for gtest_discover_tests
      run_discover_out_of_date_case(GoogleTest-PRE_TEST fake_gtest
        "PRE:basic\\.case_foo.*PRE:basic\\.case_bar"
        GTEST)
      run_discover_out_of_date_case(GoogleTest-POST_BUILD fake_gtest
        "POST:basic\\.case_foo.*POST:basic\\.case_bar"
        GTEST)
    endblock()
  endblock()
endif()
