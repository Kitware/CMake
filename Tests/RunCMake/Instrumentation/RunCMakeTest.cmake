cmake_minimum_required(VERSION 3.30)
include(RunCMake)

function(instrument test)
  # Set Paths Variables
  set(config "${CMAKE_CURRENT_LIST_DIR}/config")
  set(ENV{CMAKE_CONFIG_DIR} ${config})
  set(OPTIONS
    "BUILD"
    "BUILD_MAKE_PROGRAM"
    "INSTALL"
    "INSTALL_PARALLEL"
    "TEST"
    "WORKFLOW"
    "EXPERIMENTAL_WARNING"
    "COPY_QUERIES"
    "COPY_QUERIES_GENERATED"
    "STATIC_QUERY"
    "DYNAMIC_QUERY"
    "TRACE_QUERY"
    "MANUAL_HOOK"
    "PRESERVE_DATA"
    "NO_CONFIGURE"
    "FAIL"
  )
  cmake_parse_arguments(ARGS "${OPTIONS}" "CHECK_SCRIPT;CONFIGURE_ARG" "" ${ARGN})
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test})
  set(uuid "ec7aa2dc-b87f-45a3-8022-fe01c5f59984")
  set(v1 ${RunCMake_TEST_BINARY_DIR}/.cmake/instrumentation-${uuid}/v1)
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

  # Set hook command
  set(static_query_hook_arg 0)
  if (ARGS_STATIC_QUERY)
    set(static_query_hook_arg 1)
  endif()
  set(trace_query_hook_arg 0)
  if (ARGS_TRACE_QUERY)
    set(trace_query_hook_arg 1)
  endif()
  set(GET_HOOK "\\\"${CMAKE_COMMAND}\\\" -P \\\"${RunCMake_SOURCE_DIR}/hook.cmake\\\" ${static_query_hook_arg} ${trace_query_hook_arg}")

  # Load query JSON and cmake (with cmake_instrumentation(...)) files
  set(query ${query_dir}/${test}.json.in)
  set(cmake_file ${query_dir}/${test}.cmake)
  if (EXISTS ${query})
    file(MAKE_DIRECTORY ${v1}/query)
    configure_file(${query} ${v1}/query/${test}.json)
  else ()
    if (NOT EXISTS ${cmake_file} AND NOT EXISTS ${cmake_file}.in)
      set(cmake_file ${query_dir}/default.cmake)
    endif()
    list(APPEND ARGS_CONFIGURE_ARG "-DINSTRUMENT_COMMAND_FILE=${cmake_file}")
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
  if (NOT ARGS_EXPERIMENTAL_WARNING)
    list(APPEND ARGS_CONFIGURE_ARG "-Wno-dev")
  endif()
  if (ARGS_FAIL)
    list(APPEND ARGS_CONFIGURE_ARG "-DFAIL=ON")
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
    configure_file(
      "${cmake_file}.in"
      "${RunCMake_TEST_BINARY_DIR}/cmake-command-workflow.cmake"
      @ONLY
    )
    foreach(f IN ITEMS CMakeLists.txt main.cxx lib.cxx lib.h)
      configure_file(
        "${RunCMake_TEST_SOURCE_DIR}/${f}"
        "${RunCMake_TEST_BINARY_DIR}/${f}"
        COPYONLY
      )
    endforeach()
    set(v1 ${RunCMake_TEST_BINARY_DIR}/build/.cmake/instrumentation-${uuid}/v1)
    run_cmake_command(${test}-workflow ${CMAKE_COMMAND} --workflow default)
    set(ARGS_NO_CONFIGURE TRUE)
  endif()
  if (NOT ARGS_NO_CONFIGURE)
    run_cmake_with_options(${test} ${ARGS_CONFIGURE_ARG} ${maybe_CMAKE_BUILD_TYPE})
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
    run_cmake_command(${test}-build
      ${CMAKE_COMMAND} --build . ${cmake_build_args} -- ${additional_build_args}
    )
    if (ARGS_FAIL)
      unset(RunCMake_TEST_OUTPUT_MERGE)
    endif()
  endif()
  if (ARGS_BUILD_MAKE_PROGRAM)
    set(RunCMake_TEST_OUTPUT_MERGE 1)
    # Force reconfigure to test for double preBuild & postBuild hooks
    file(TOUCH ${RunCMake_TEST_BINARY_DIR}/CMakeCache.txt)
    run_cmake_command(${test}-make-program ${RunCMake_MAKE_PROGRAM})
    unset(RunCMake_TEST_OUTPUT_MERGE)
  endif()
  if (ARGS_INSTALL)
    run_cmake_command(${test}-install ${CMAKE_COMMAND} --install . --prefix install --config Debug)
  endif()
  if (ARGS_TEST)
    run_cmake_command(${test}-test ${CMAKE_CTEST_COMMAND} . -C Debug)
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

# Bad Queries
instrument(bad-option)
instrument(bad-hook)
instrument(empty)
instrument(bad-version)

# Verify Hooks Run and Index File
instrument(hooks-1 BUILD INSTALL TEST STATIC_QUERY)
instrument(hooks-2 BUILD INSTALL TEST)
instrument(hooks-no-callbacks MANUAL_HOOK)

# Check data file contents for optional query data
instrument(no-query
  BUILD INSTALL TEST
  CHECK_SCRIPT check-data-dir.cmake
)
instrument(dynamic-query
  BUILD INSTALL TEST DYNAMIC_QUERY
  CHECK_SCRIPT check-data-dir.cmake
)
instrument(both-query
  BUILD INSTALL TEST DYNAMIC_QUERY
  CHECK_SCRIPT check-data-dir.cmake
)

# Test cmake_instrumentation command
instrument(cmake-command
  COPY_QUERIES STATIC_QUERY DYNAMIC_QUERY
  CHECK_SCRIPT check-generated-queries.cmake
)
instrument(cmake-command-data
  COPY_QUERIES BUILD INSTALL TEST DYNAMIC_QUERY
  CHECK_SCRIPT check-data-dir.cmake
)
instrument(cmake-command-experimental-warning
  EXPERIMENTAL_WARNING
)
instrument(cmake-command-bad-api-version)
instrument(cmake-command-bad-data-version)
instrument(cmake-command-missing-version)
instrument(cmake-command-bad-arg)
instrument(cmake-command-parallel-install
  BUILD INSTALL TEST INSTALL_PARALLEL DYNAMIC_QUERY
  CHECK_SCRIPT check-data-dir.cmake)
instrument(cmake-command-initial-cache
  CONFIGURE_ARG "-C ${RunCMake_BINARY_DIR}/initial.cmake"
  CHECK_SCRIPT check-data-dir.cmake
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
  CONFIGURE_ARG "-DN=1"
)
instrument(cmake-command-custom-content
  BUILD PRESERVE_DATA
  CONFIGURE_ARG "-DN=2"
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
endif()
