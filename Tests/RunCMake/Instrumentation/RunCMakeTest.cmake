cmake_minimum_required(VERSION 4.3)
include(RunCMake)

function(instrument test)
  # Set Paths Variables
  set(config "${CMAKE_CURRENT_LIST_DIR}/config")
  set(ENV{CMAKE_CONFIG_DIR} ${config})
  set(OPTIONS
    "BUILD"
    "BUILD_MAKE_PROGRAM"
    "BUILD_MAKE_PROGRAM_CHANGE_DIR"
    "INTERRUPT"
    "INTERRUPT_SEAM"
    "INSTALL"
    "INSTALL_PARALLEL"
    "INSTALL_SEAM"
    "INSTALL_INTERRUPT"
    "TEST"
    "CTEST_SEAM"
    "CTEST_INTERRUPT"
    "CTEST_FAILOVER"
    "WORKFLOW"
    "COPY_QUERIES"
    "COPY_QUERIES_GENERATED"
    "STATIC_QUERY"
    "DYNAMIC_QUERY"
    "CAPTURE_OUTPUT_QUERY"
    "COMPILE_TRACE_QUERY"
    "COMPILE_TRACE_QUERY_NULL"
    "TRACE_QUERY"
    "MANUAL_HOOK"
    "PRESERVE_DATA"
    "NO_CONFIGURE"
    "DISABLE_TEST"
    "FAIL"
    "BAD_QUERY"
  )
  cmake_parse_arguments(ARGS "${OPTIONS}" "CHECK_SCRIPT" "CONFIGURE_ARGS" ${ARGN})
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test})
  set(v1 ${RunCMake_TEST_BINARY_DIR}/.cmake/instrumentation/v1)
  set(v1 ${v1} PARENT_SCOPE)
  set(query_dir ${CMAKE_CURRENT_LIST_DIR}/query)
  configure_file(${RunCMake_SOURCE_DIR}/initial.cmake.in ${RunCMake_BINARY_DIR}/initial.cmake)

  # Clear previous instrumentation data
  # We can't use RunCMake_TEST_NO_CLEAN 0 because we preserve queries placed in the build tree after
  if (ARGS_PRESERVE_DATA)
    file(REMOVE_RECURSE ${RunCMake_TEST_BINARY_DIR}/CMakeFiles)
  else()
    file(REMOVE_RECURSE ${RunCMake_TEST_BINARY_DIR})
  endif()

  if (ARGS_BAD_QUERY)
    set(schema_validate_result 1)
  else()
    set(schema_validate_result 0)
  endif()
  set(schema_validate_result ${schema_validate_result} PARENT_SCOPE)

  # Set hook command
  set(static_query_hook_arg 0)
  if (ARGS_STATIC_QUERY)
    set(static_query_hook_arg 1)
  endif()
  set(trace_query_hook_arg 0)
  if (ARGS_TRACE_QUERY)
    set(trace_query_hook_arg 1)
  endif()
  set(ARGS_COMPILE_TRACE_QUERY ${ARGS_COMPILE_TRACE_QUERY} PARENT_SCOPE)
  set(ARGS_COMPILE_TRACE_QUERY_NULL ${ARGS_COMPILE_TRACE_QUERY_NULL} PARENT_SCOPE)
  set(GET_HOOK
    "\\\"${CMAKE_COMMAND}\\\""
    "-DSTATIC_QUERY=${static_query_hook_arg}"
    "-DTRACE_QUERY=${trace_query_hook_arg}"
    "-DPython_EXECUTABLE=${Python_EXECUTABLE}"
    "-DCMake_TEST_JSON_SCHEMA=${CMake_TEST_JSON_SCHEMA}"
    "-P \\\"${RunCMake_SOURCE_DIR}/hook.cmake\\\""
  )
  list(JOIN GET_HOOK " " GET_HOOK)

  # Load query JSON and cmake (with cmake_instrumentation(...)) files
  set(query ${query_dir}/${test}.json.in)
  set(cmake_file ${query_dir}/${test}.cmake)
  if (EXISTS ${query})
    file(MAKE_DIRECTORY ${v1}/query)
    configure_file(${query} ${v1}/query/${test}.json)
  else ()
    if (NOT EXISTS ${cmake_file})
      if (EXISTS ${cmake_file}.in)
        cmake_path(GET cmake_file FILENAME cmake_filename)
        configure_file(
          "${cmake_file}.in"
          "${RunCMake_TEST_BINARY_DIR}/${cmake_filename}"
          @ONLY
        )
        set(cmake_file "${RunCMake_TEST_BINARY_DIR}/${cmake_filename}")
      else ()
        set(cmake_file "${query_dir}/default.cmake")
      endif()
    endif()
    list(APPEND ARGS_CONFIGURE_ARGS "-DINSTRUMENT_COMMAND_FILE=${cmake_file}")
  endif()

  set(copy_loc ${RunCMake_TEST_BINARY_DIR}/query)
  if (ARGS_COPY_QUERIES_GENERATED)
    set(ARGS_COPY_QUERIES TRUE)
    set(copy_loc ${v1}/query/generated) # Copied files here should be cleared on configure
  endif()
  if (ARGS_COPY_QUERIES)
    set(CMAKE_COMMAND_QUOTE ${CMAKE_COMMAND})
    if (CMAKE_COMMAND MATCHES " ")
      set(CMAKE_COMMAND_QUOTE "\\\"${CMAKE_COMMAND}\\\"")
    endif()
    file(MAKE_DIRECTORY ${copy_loc})
    set(generated_queries "0;1;2")
    foreach(n IN LISTS generated_queries)
      configure_file(
        "${query_dir}/generated/query-${n}.json.in"
        "${copy_loc}/query-${n}.json"
      )
    endforeach()
  endif()

  # Configure Test Case
  set(RunCMake_TEST_NO_CLEAN 1)
  if (ARGS_FAIL)
    list(APPEND ARGS_CONFIGURE_ARGS "-DFAIL=ON")
  endif()
  if (ARGS_DISABLE_TEST)
    list(APPEND ARGS_CONFIGURE_ARGS "-DDISABLE_TEST=ON")
  endif()
  if (ARGS_INTERRUPT)
    list(APPEND ARGS_CONFIGURE_ARGS
      "-DINTERRUPT_BUILD_SRC=${RunCMake_SOURCE_DIR}/InterruptBuild.c")
  endif()
  if (ARGS_INSTALL_INTERRUPT)
    list(APPEND ARGS_CONFIGURE_ARGS
      "-DINTERRUPT_BUILD_SRC=${RunCMake_SOURCE_DIR}/InterruptBuild.c"
      "-DINSTALL_INTERRUPT=ON")
  endif()
  if (ARGS_CTEST_INTERRUPT OR ARGS_CTEST_FAILOVER)
    list(APPEND ARGS_CONFIGURE_ARGS
      "-DINTERRUPT_BUILD_SRC=${RunCMake_SOURCE_DIR}/InterruptBuild.c"
      "-DCTEST_INTERRUPT=ON")
  endif()
  set(RunCMake_TEST_SOURCE_DIR ${RunCMake_SOURCE_DIR}/project)
  if(NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(maybe_CMAKE_BUILD_TYPE -DCMAKE_BUILD_TYPE=Debug)
  endif()
  if (ARGS_WORKFLOW)
    configure_file(
      "${RunCMake_TEST_SOURCE_DIR}/CMakePresets.json.in"
      "${RunCMake_TEST_BINARY_DIR}/CMakePresets.json"
      @ONLY
    )
    foreach(f IN ITEMS CMakeLists.txt main.c lib.c lib.h shell_redirect.txt)
      configure_file(
        "${RunCMake_TEST_SOURCE_DIR}/${f}"
        "${RunCMake_TEST_BINARY_DIR}/${f}"
        COPYONLY
      )
    endforeach()
    set(RunCMake_QUIET_ERROR 1)
    set(v1 ${RunCMake_TEST_BINARY_DIR}/build/.cmake/instrumentation/v1)
    run_cmake_command(${test}-workflow ${CMAKE_COMMAND} --workflow default)
    set(ARGS_NO_CONFIGURE TRUE)
    unset(RunCMake_QUIET_ERROR)
  endif()
  if (NOT ARGS_NO_CONFIGURE)
    run_cmake_with_options(${test} ${ARGS_CONFIGURE_ARGS} ${maybe_CMAKE_BUILD_TYPE})
  endif()

  # Follow-up Commands
  if (ARGS_BUILD)
    set(cmake_build_args --config Debug)
    set(additional_build_args)
    if (ARGS_FAIL)
      # Tests with ARGS_FAIL expect all targets to build, including the ones
      # which should succeed and those which should fail.
      if (RunCMake_GENERATOR MATCHES "Ninja")
        set(keep_going_arg -k 0)
      elseif (RunCMake_GENERATOR MATCHES "FASTBuild")
        set(keep_going_arg -nostoponerror)
      else()
        set(keep_going_arg -k)
      endif()
      string(APPEND additional_build_args ${keep_going_arg})
      # Merge stdout and stderr because different compilers will throw their
      # errors to different places.
      set(RunCMake_TEST_OUTPUT_MERGE 1)
    endif()
    set(RunCMake_QUIET_ERROR 1)
    run_cmake_command(${test}-build
      ${CMAKE_COMMAND} --build . ${cmake_build_args} -- ${additional_build_args}
    )
    unset(RunCMake_QUIET_ERROR)
    if (ARGS_FAIL)
      unset(RunCMake_TEST_OUTPUT_MERGE)
    endif()
  endif()
  if (ARGS_INTERRUPT)
    # Build just the interrupt helper so it exists for the interrupted build.
    # This uninterrupted build runs the postCMakeBuild hook, so remove the
    # postCMakeBuild.hook file it produces; its absence after the interrupted
    # build below then proves that build's hook was skipped.
    run_cmake_command(${test}-helper
      ${CMAKE_COMMAND} --build . --config Debug --target InterruptBuild)
    file(REMOVE ${v1}/postCMakeBuild.hook)

    # Run an instrumented build and interrupt it after a few seconds, while the
    # slow target is still running.  Multi-config generators place the helper
    # under a per-config subdirectory; the build below uses --config Debug.
    set(helper_dir ${RunCMake_TEST_BINARY_DIR})
    if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
      set(helper_dir ${helper_dir}/Debug)
    endif()
    set(helper ${helper_dir}/InterruptBuild${CMAKE_EXECUTABLE_SUFFIX})
    set(RunCMake_QUIET_ERROR 1)
    run_cmake_command(${test}-signal
      ${helper} 3
      ${CMAKE_COMMAND} --build . --config Debug)
    unset(RunCMake_QUIET_ERROR)
  endif()
  if (ARGS_INTERRUPT_SEAM)
    # Drive the cmakeBuild interrupt path deterministically via the test-only
    # injection seam, with no OS signal, so it runs on every generator.  First
    # build normally so the postCMakeBuild hook runs and creates its marker
    # file; remove it so its absence after the injected build proves that
    # build's hook was skipped.
    set(RunCMake_QUIET_ERROR 1)
    run_cmake_command(${test}-warmup
      ${CMAKE_COMMAND} --build . --config Debug)
    file(REMOVE ${v1}/postCMakeBuild.hook)

    # Inject an interrupt (SIGINT == 2) via the undocumented test seam and build
    # again; cmake exits cleanly but writes the interrupted cmakeBuild snippet
    # and skips the hook.
    set(ENV{__CMAKE_INSTRUMENTATION_TEST_INTERRUPT} 2)
    set(RunCMake_TEST_EXPECT_RESULT 0)
    run_cmake_command(${test}-seam
      ${CMAKE_COMMAND} --build . --config Debug)
    unset(RunCMake_TEST_EXPECT_RESULT)
    unset(ENV{__CMAKE_INSTRUMENTATION_TEST_INTERRUPT})
    unset(RunCMake_QUIET_ERROR)
  endif()
  if (ARGS_BUILD_MAKE_PROGRAM)
    set(RunCMake_TEST_OUTPUT_MERGE 1)
    set(RunCMake_QUIET_ERROR 1)
    # Force reconfigure to test for double preBuild & postBuild hooks
    file(TOUCH ${RunCMake_TEST_BINARY_DIR}/CMakeCache.txt)
    if (ARGS_BUILD_MAKE_PROGRAM_CHANGE_DIR)
      set(RunCMake_TEST_COMMAND_WORKING_DIRECTORY ${RunCMake_BINARY_DIR})
      run_cmake_command(${test}-make-program ${RunCMake_MAKE_PROGRAM} -C ${RunCMake_TEST_BINARY_DIR})
      unset(RunCMake_TEST_COMMAND_WORKING_DIRECTORY)
    else()
      run_cmake_command(${test}-make-program ${RunCMake_MAKE_PROGRAM})
    endif()
    unset(RunCMake_TEST_OUTPUT_MERGE)
    unset(RunCMake_QUIET_ERROR)
  endif()
  if (ARGS_INSTALL)
    run_cmake_command(${test}-install ${CMAKE_COMMAND} --install . --prefix install --config Debug)
  endif()
  if (ARGS_INSTALL_SEAM)
    # Drive the cmakeInstall interrupt path deterministically via the test-only
    # injection seam, with no OS signal, so it runs on every generator.  First
    # install normally so the postCMakeInstall hook runs and creates its marker
    # file; remove it (and the manifest) so their absence after the injected
    # install proves that install's hook was skipped and left nothing complete.
    set(RunCMake_QUIET_ERROR 1)
    run_cmake_command(${test}-warmup
      ${CMAKE_COMMAND} --install . --prefix install --config Debug)
    file(REMOVE ${v1}/postCMakeInstall.hook)
    file(REMOVE ${RunCMake_TEST_BINARY_DIR}/install_manifest.txt)

    # Inject an interrupt (SIGINT == 2) via the undocumented test seam and
    # install again; cmake exits with a non-zero status but writes the
    # interrupted cmakeInstall snippet and skips the hook.
    set(ENV{__CMAKE_INSTRUMENTATION_TEST_INTERRUPT} 2)
    set(RunCMake_TEST_EXPECT_RESULT 1)
    run_cmake_command(${test}-seam
      ${CMAKE_COMMAND} --install . --prefix install --config Debug)
    unset(RunCMake_TEST_EXPECT_RESULT)
    unset(ENV{__CMAKE_INSTRUMENTATION_TEST_INTERRUPT})
    unset(RunCMake_QUIET_ERROR)
  endif()
  if (ARGS_INSTALL_INTERRUPT)
    # Build just the interrupt helper and main so the parallel install has
    # something to do.  This build runs no postCMakeInstall hook (the query only
    # requests that hook), so remove any stale marker; its absence after the
    # interrupted install then proves that install's hook was skipped.
    run_cmake_command(${test}-helper
      ${CMAKE_COMMAND} --build . --config Debug --target InterruptBuild main)
    file(REMOVE ${v1}/postCMakeBuild.hook)
    file(REMOVE ${v1}/postCMakeInstall.hook)
    file(REMOVE ${RunCMake_TEST_BINARY_DIR}/install_manifest.txt)

    # Pin InstallScripts.json newest so the parallel install path is chosen
    # deterministically (the staleness heuristic can otherwise fall back to
    # serial on coarse-mtime filesystems).
    file(TOUCH_NOCREATE ${RunCMake_TEST_BINARY_DIR}/CMakeFiles/InstallScripts.json)

    # Run an instrumented parallel install and interrupt it after a few seconds,
    # while a slow install script is still running and others are pending.
    set(helper_dir ${RunCMake_TEST_BINARY_DIR})
    if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
      set(helper_dir ${helper_dir}/Debug)
    endif()
    set(helper ${helper_dir}/InterruptBuild${CMAKE_EXECUTABLE_SUFFIX})
    # Merge stdout/stderr: the parallel aggregation prints one "User interrupt"
    # diagnostic per in-flight script to stderr, whose presence and count are
    # cosmetic and race with the re-raise that terminates the process.
    set(RunCMake_TEST_OUTPUT_MERGE 1)
    set(RunCMake_QUIET_ERROR 1)
    run_cmake_command(${test}-signal
      ${helper} 3
      ${CMAKE_COMMAND} --install . --prefix install --config Debug -j 1)
    unset(RunCMake_QUIET_ERROR)
    unset(RunCMake_TEST_OUTPUT_MERGE)
  endif()
  if (ARGS_TEST)
    run_cmake_command(${test}-test ${CMAKE_CTEST_COMMAND} . -C Debug)
  endif()
  if (ARGS_CTEST_SEAM)
    # Drive the ctest interrupt path deterministically via the test-only
    # injection seam, with no OS signal, so it runs on every generator.  First
    # run ctest normally so the postCTest hook runs and creates its marker file;
    # remove it so its absence after the injected run proves that run's hook was
    # skipped.
    set(RunCMake_QUIET_ERROR 1)
    run_cmake_command(${test}-warmup
      ${CMAKE_CTEST_COMMAND} . -C Debug)
    file(REMOVE ${v1}/postCTest.hook)

    # Inject an interrupt (SIGINT == 2) via the undocumented test seam and run
    # ctest again; ctest exits with an error status (cmCTest::TEST_ERRORS == 8)
    # but writes the interrupted ctest snippet and skips the hook.
    set(ENV{__CMAKE_INSTRUMENTATION_TEST_INTERRUPT} 2)
    set(RunCMake_TEST_EXPECT_RESULT 8)
    run_cmake_command(${test}-seam
      ${CMAKE_CTEST_COMMAND} . -C Debug)
    unset(RunCMake_TEST_EXPECT_RESULT)
    unset(ENV{__CMAKE_INSTRUMENTATION_TEST_INTERRUPT})
    unset(RunCMake_QUIET_ERROR)
  endif()
  if (ARGS_CTEST_INTERRUPT)
    # Build just the interrupt helper so it exists for the interrupted run.
    run_cmake_command(${test}-helper
      ${CMAKE_COMMAND} --build . --config Debug --target InterruptBuild)
    file(REMOVE ${v1}/postCTest.hook)
    file(REMOVE_RECURSE ${RunCMake_TEST_BINARY_DIR}/ran)

    set(helper_dir ${RunCMake_TEST_BINARY_DIR})
    if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
      set(helper_dir ${helper_dir}/Debug)
    endif()
    set(helper ${helper_dir}/InterruptBuild${CMAKE_EXECUTABLE_SUFFIX})

    # Interrupt a serial `ctest -j 1` a few seconds in, while the fast test has
    # finished and a slow test is in-flight with others pending.  The
    # instrumented ctest re-raises the signal, so the helper reports exit 42.
    # Restrict to the ctest* fixture tests (the project's `test` needs `main`).
    set(RunCMake_TEST_OUTPUT_MERGE 1)
    set(RunCMake_QUIET_ERROR 1)
    run_cmake_command(${test}-signal
      ${helper} 4
      ${CMAKE_CTEST_COMMAND} . -C Debug -j 1 -R ctest)
    unset(RunCMake_QUIET_ERROR)
    unset(RunCMake_TEST_OUTPUT_MERGE)

    # Record which tests ran during the interrupted run so the check script can
    # assert the pending tests were canceled.
    if (EXISTS ${RunCMake_TEST_BINARY_DIR}/ran)
      file(RENAME ${RunCMake_TEST_BINARY_DIR}/ran
        ${RunCMake_TEST_BINARY_DIR}/ran-interrupt)
    endif()
  endif()
  if (ARGS_CTEST_FAILOVER)
    # Build just the interrupt helper so it exists for the interrupted run.
    run_cmake_command(${test}-helper
      ${CMAKE_COMMAND} --build . --config Debug --target InterruptBuild)
    file(REMOVE ${v1}/postCTest.hook)
    file(REMOVE_RECURSE ${RunCMake_TEST_BINARY_DIR}/ran)

    set(helper_dir ${RunCMake_TEST_BINARY_DIR})
    if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
      set(helper_dir ${helper_dir}/Debug)
    endif()
    set(helper ${helper_dir}/InterruptBuild${CMAKE_EXECUTABLE_SUFFIX})

    # Phase 1: interrupt a serial `ctest -j 1` while a slow test is in-flight.
    # The fast test finishes (checkpointed); the in-flight slow test is killed
    # (deliberately not checkpointed) and the rest stay pending.  Restrict to
    # the ctest* fixture tests (the project's `test` needs `main`).
    set(RunCMake_TEST_OUTPUT_MERGE 1)
    set(RunCMake_QUIET_ERROR 1)
    run_cmake_command(${test}-signal
      ${helper} 4
      ${CMAKE_CTEST_COMMAND} . -C Debug -j 1 -R ctest)
    if (EXISTS ${RunCMake_TEST_BINARY_DIR}/ran)
      file(RENAME ${RunCMake_TEST_BINARY_DIR}/ran
        ${RunCMake_TEST_BINARY_DIR}/ran-interrupt)
    endif()

    # The interrupted run left an un-indexed ctest snippet marked with
    # interruptSignal.  The resuming run below re-indexes instrumentation, and
    # the generic snippet validator rejects that field, so clear the snippet
    # data first.  The `ctest -F` checkpoint lives under Testing/Temporary and
    # is untouched.
    file(REMOVE_RECURSE ${v1}/data)

    # Phase 2: resume the interrupted test set with `ctest -F`.  It must skip
    # the already-finished fast test and run the interrupted and pending slow
    # tests (run in parallel so resume finishes promptly).
    run_cmake_command(${test}-resume
      ${CMAKE_CTEST_COMMAND} . -C Debug -F -j 4 -R ctest)
    unset(RunCMake_QUIET_ERROR)
    unset(RunCMake_TEST_OUTPUT_MERGE)
    if (EXISTS ${RunCMake_TEST_BINARY_DIR}/ran)
      file(RENAME ${RunCMake_TEST_BINARY_DIR}/ran
        ${RunCMake_TEST_BINARY_DIR}/ran-resume)
    endif()
  endif()
  if (ARGS_MANUAL_HOOK)
    run_cmake_command(${test}-index ${CMAKE_CTEST_COMMAND} --collect-instrumentation .)
  endif()

  # Run Post-Test Checks
  # Check scripts need to run after ALL run_cmake_command have finished
  if (ARGS_CHECK_SCRIPT)
    set(RunCMake-check-file ${ARGS_CHECK_SCRIPT})
    set(RunCMake_CHECK_ONLY 1)
    run_cmake(${test}-verify)
    unset(RunCMake-check-file)
    unset(RunCMake_CHECK_ONLY)
  endif()
endfunction()

if (INSTRUMENTATION_INTERRUPT_REAL)
  # RunCMake.InstrumentationInterrupt runs ONLY the real-signal/
  # console-event interrupt case, as it must be excluded from MemCheck.
  #
  # POSIX delivers a real SIGINT to a contained process group.  On Windows, only
  # the Ninja generator is exercised: its native tool reliably stops on the
  # console event and does not re-broadcast it to the runner; the other Windows
  # make-family generators are covered by the injection seam instead.
  if (NOT WIN32 OR RunCMake_GENERATOR MATCHES "Ninja")
    instrument(interrupt-build INTERRUPT
      CHECK_SCRIPT check-interrupted.cmake
    )
    # Interrupt a parallel `cmake --install` with a real OS signal, proving the
    # cooperative cancellation stops pending install scripts and skips the hook.
    instrument(interrupt-install INSTALL_INTERRUPT
      CHECK_SCRIPT check-installation-interrupted.cmake
    )
    # Interrupt a `ctest` run with a real OS signal, proving the scheduler stops
    # launching pending tests, skips the hook, and preserves the `ctest -F`
    # checkpoint so the interrupted test set can be resumed.
    instrument(interrupt-test CTEST_INTERRUPT
      CHECK_SCRIPT check-test-interrupted.cmake
    )
    # Interrupt a `ctest` run and then resume it with `ctest -F`, proving the
    # checkpoint keeps the finished test (skipped on resume) but not the
    # in-flight test killed by the interrupt (re-run on resume).
    instrument(interrupt-test-failover CTEST_FAILOVER
      CHECK_SCRIPT check-test-failover.cmake
    )
  endif()
  return()
endif()

# Bad Queries
instrument(bad-option BAD_QUERY
  CHECK_SCRIPT check-query-dir.cmake
)
instrument(bad-hook BAD_QUERY
  CHECK_SCRIPT check-query-dir.cmake
)
instrument(empty BAD_QUERY
  CHECK_SCRIPT check-query-dir.cmake
)
instrument(bad-version BAD_QUERY
  CHECK_SCRIPT check-query-dir.cmake
)
instrument(bad-version-major BAD_QUERY
  CHECK_SCRIPT check-query-dir.cmake
)
instrument(bad-version-minor BAD_QUERY
  CHECK_SCRIPT check-query-dir.cmake
)
instrument(bad-version-object BAD_QUERY
  CHECK_SCRIPT check-query-dir.cmake
)
instrument(hooks-invalid-version-ignored BUILD
  CHECK_SCRIPT check-hooks-invalid-version-ignored.cmake
)

# Verify Hooks Run and Index File
instrument(hooks-1 BUILD INSTALL TEST STATIC_QUERY
  CHECK_SCRIPT check-query-dir.cmake
)
instrument(hooks-2 BUILD INSTALL TEST
  CHECK_SCRIPT check-query-dir.cmake
)
instrument(hooks-no-callbacks MANUAL_HOOK
  CHECK_SCRIPT check-query-dir.cmake
)

# Check data file contents for optional query data
instrument(no-query
  BUILD INSTALL TEST
  CHECK_SCRIPT check-data-dir.cmake
)
if(RunCMake_GENERATOR STREQUAL "Ninja")
  set(CHECK_NINJA_INSTRUMENT_ORDER 1)
endif()
instrument(disabled-test
  BUILD TEST DISABLE_TEST
  CHECK_SCRIPT check-data-dir.cmake
)
instrument(dynamic-query
  BUILD INSTALL TEST DYNAMIC_QUERY
  CHECK_SCRIPT check-data-dir.cmake
)
instrument(both-query
  BUILD INSTALL TEST STATIC_QUERY DYNAMIC_QUERY CAPTURE_OUTPUT_QUERY
  CHECK_SCRIPT check-data-dir.cmake
)
unset(CHECK_NINJA_INSTRUMENT_ORDER)

# Test cmake_instrumentation command
instrument(cmake-command
  COPY_QUERIES STATIC_QUERY DYNAMIC_QUERY
  CHECK_SCRIPT check-generated-queries.cmake
)
instrument(cmake-command-data
  COPY_QUERIES BUILD INSTALL TEST DYNAMIC_QUERY
  CHECK_SCRIPT check-data-dir.cmake
)
instrument(cmake-command-bad-api-version)
instrument(cmake-command-bad-data-version)
instrument(cmake-command-unsupported-data-version)
instrument(cmake-command-missing-version)
instrument(cmake-command-bad-arg)
instrument(cmake-command-parallel-install
  BUILD INSTALL TEST INSTALL_PARALLEL DYNAMIC_QUERY
  CHECK_SCRIPT check-data-dir.cmake)
instrument(cmake-command-initial-cache
  CONFIGURE_ARGS "-C ${RunCMake_BINARY_DIR}/initial.cmake"
)
instrument(cmake-command-resets-generated
  COPY_QUERIES_GENERATED
  CHECK_SCRIPT check-data-dir.cmake
)
instrument(cmake-command-cmake-build
  BUILD
  CHECK_SCRIPT check-no-make-program-hooks.cmake
)
if(RunCMake_GENERATOR STREQUAL "Borland Makefiles")
  # Borland 'make' has no '-k' flag.
  set(Skip_COMMAND_FAILURES_Case 1)
endif()
if(NOT Skip_COMMAND_FAILURES_Case)
  instrument(cmake-command-failures
    FAIL BUILD TEST INSTALL
    CHECK_SCRIPT check-data-dir.cmake
  )
endif()
instrument(cmake-command-workflow
  WORKFLOW
  CHECK_SCRIPT check-workflow-hook.cmake
)

# Test CUSTOM_CONTENT
instrument(cmake-command-custom-content
  BUILD
  CONFIGURE_ARGS "-DN=1"
)
instrument(cmake-command-custom-content
  BUILD PRESERVE_DATA
  CONFIGURE_ARGS "-DN=2"
  CHECK_SCRIPT check-custom-content.cmake
)
set(indexDir ${v1}/data/index)
set(fakeIndex ${indexDir}/index-0.json)
file(MAKE_DIRECTORY ${indexDir})
file(TOUCH ${fakeIndex})
# fakeIndex newer than all content files prevents their deletion
set(EXPECTED_CONTENT_FILES 2)
instrument(cmake-command-custom-content
  NO_CONFIGURE MANUAL_HOOK PRESERVE_DATA
  CHECK_SCRIPT check-custom-content-removed.cmake
)
file(REMOVE ${fakeIndex})
# old content files will be removed if no index file exists
set(EXPECTED_CONTENT_FILES 1)
instrument(cmake-command-custom-content
  NO_CONFIGURE MANUAL_HOOK PRESERVE_DATA
  CHECK_SCRIPT check-custom-content-removed.cmake
)
instrument(cmake-command-custom-content-bad-type)
instrument(cmake-command-custom-content-bad-content)
instrument(cmake-command-custom-content-empty
  CHECK_SCRIPT check-custom-content-empty.cmake
)

# Test Google trace
instrument(trace-query
  BUILD INSTALL TEST TRACE_QUERY
  CHECK_SCRIPT check-generated-queries.cmake
)
instrument(cmake-command-trace
  BUILD INSTALL TEST TRACE_QUERY
)
instrument(cmake-command-trace
  BUILD PRESERVE_DATA
  CHECK_SCRIPT check-trace-removed.cmake
)

# FIXME(#27984): FASTBuild long output test failure
# FIXME(#279846): Watcom WMake unsupported file names
if(NOT RunCMake_GENERATOR MATCHES "FASTBuild|Watcom WMake")
  instrument(cmake-command-long-output
    BUILD
    CONFIGURE_ARGS "-DLONG_CUSTOM_COMMAND_OUTPUT=ON"
    CHECK_SCRIPT check-long-output.cmake
  )
endif()

# Test capture output
instrument(cmake-command-capture-output
  BUILD CAPTURE_OUTPUT_QUERY
  CHECK_SCRIPT check-data-dir.cmake
)

# Test compile trace collection
if (CMAKE_C_COMPILER_ID STREQUAL "AppleClang")
  if (CMAKE_C_COMPILER_VERSION VERSION_LESS 11.1)
    set(Skip_COMPILE_TRACE_QUERY_Case 1)
  elseif (CMAKE_C_COMPILER_VERSION VERSION_LESS 15)
    set(Skip_COMPILE_TRACE_QUERY_ARG_Case 1)
  endif()
elseif (CMAKE_C_COMPILER_ID STREQUAL "Clang")
  if (CMAKE_C_COMPILER_VERSION VERSION_LESS 9)
    set(Skip_COMPILE_TRACE_QUERY_Case 1)
  elseif (CMAKE_C_COMPILER_VERSION VERSION_LESS 16)
    set(Skip_COMPILE_TRACE_QUERY_ARG_Case 1)
  endif()
else()
  set(Skip_COMPILE_TRACE_QUERY_Case 1)
endif()
if("$ENV{CMAKE_OSX_ARCHITECTURES}" MATCHES "[;$]")
  # `-ftime-trace` with multiple `-arch` puts the trace file in TMPDIR.
  set(Skip_COMPILE_TRACE_QUERY_Case 1)
endif()
if (NOT Skip_COMPILE_TRACE_QUERY_Case)
  instrument(cmake-command-compile-trace
    BUILD COMPILE_TRACE_QUERY
    CONFIGURE_ARGS
      "-DINSTRUMENT_COMPILE_TRACE=DEFAULT"
      "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}"
    CHECK_SCRIPT check-data-dir.cmake
  )
  instrument(cmake-command-compile-trace-null
    BUILD COMPILE_TRACE_QUERY_NULL
    CHECK_SCRIPT check-data-dir.cmake
  )
  if (NOT Skip_COMPILE_TRACE_QUERY_ARG_Case)
    instrument(cmake-command-compile-trace-explicit
      BUILD COMPILE_TRACE_QUERY
      CONFIGURE_ARGS
        "-DINSTRUMENT_COMPILE_TRACE=EXPLICIT"
        "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}"
      CHECK_SCRIPT check-data-dir.cmake
    )
  endif()
  # FIXME(#16731): ar on macOS does not support response files.
  if (RunCMake_GENERATOR MATCHES "Ninja" AND NOT APPLE)
    instrument(cmake-command-compile-trace-rsp
      BUILD COMPILE_TRACE_QUERY
      CONFIGURE_ARGS
        "-DINSTRUMENT_COMPILE_TRACE=EXPLICIT"
        "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}"
        "-DCMAKE_NINJA_FORCE_RESPONSE_FILE=ON"
      CHECK_SCRIPT check-data-dir.cmake
    )
  endif()
endif()

# Test that interrupting `cmake --build` or `cmake --install` still writes the
# overall cmakeBuild/cmakeInstall snippet, recording the interrupting signal,
# and skips the corresponding post-command hook.  These cases use the
# deterministic test seam (no OS event); the real OS-event counterparts run in
# the separate RunCMake.InstrumentationInterrupt suite.
instrument(interrupt-build INTERRUPT_SEAM
  CHECK_SCRIPT check-interrupted.cmake
)
instrument(interrupt-install BUILD INSTALL_SEAM
  CHECK_SCRIPT check-installation-interrupted.cmake
)
instrument(interrupt-test BUILD CTEST_SEAM
  CHECK_SCRIPT check-test-interrupted.cmake
)

# Test make/ninja hooks
if(RunCMake_GENERATOR STREQUAL "FASTBuild")
  # FIXME(#27184): This does not work for FASTBuild.
  set(Skip_BUILD_MAKE_PROGRAM_Case 1)
elseif(RunCMake_GENERATOR STREQUAL "MSYS Makefiles")
  # FIXME(#27079): This does not work for MSYS Makefiles.
  set(Skip_BUILD_MAKE_PROGRAM_Case 1)
elseif(RunCMake_GENERATOR STREQUAL "NMake Makefiles")
 execute_process(
   COMMAND "${RunCMake_MAKE_PROGRAM}" -?
   OUTPUT_VARIABLE nmake_out
   ERROR_VARIABLE nmake_out
   RESULT_VARIABLE nmake_res
   OUTPUT_STRIP_TRAILING_WHITESPACE
   )
   if(nmake_res EQUAL 0 AND nmake_out MATCHES "Program Maintenance Utility[^\n]+Version ([1-9][0-9.]+)")
     set(nmake_version "${CMAKE_MATCH_1}")
   else()
     message(FATAL_ERROR "'nmake -?' reported:\n${nmake_out}")
   endif()
   if(nmake_version VERSION_LESS 9)
     set(Skip_BUILD_MAKE_PROGRAM_Case 1)
   endif()
endif()
if(NOT Skip_BUILD_MAKE_PROGRAM_Case)
  instrument(cmake-command-make-program
    BUILD_MAKE_PROGRAM
    CHECK_SCRIPT check-make-program-hooks.cmake)
  instrument(cmake-command-build-snippet
    BUILD_MAKE_PROGRAM
    CHECK_SCRIPT check-data-dir.cmake)
  if (NOT Skip_COMPILE_TRACE_QUERY_Case AND NOT RunCMake_GENERATOR STREQUAL "NMake Makefiles")
    instrument(cmake-command-compile-trace-make-program
      BUILD_MAKE_PROGRAM BUILD_MAKE_PROGRAM_CHANGE_DIR COMPILE_TRACE_QUERY
      CONFIGURE_ARGS
        "-DINSTRUMENT_COMPILE_TRACE=DEFAULT"
        "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}"
      CHECK_SCRIPT check-data-dir.cmake
    )
  endif()
endif()
