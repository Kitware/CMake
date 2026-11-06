include(RunCMake)
include(${RunCMake_SOURCE_DIR}/CDashServer.cmake)

# Isolate the dashboard scripts from the external environment.
unset(ENV{CTEST_PARALLEL_LEVEL})
unset(ENV{CTEST_OUTPUT_ON_FAILURE})
unset(ENV{CTEST_NO_TESTS_ACTION})
unset(ENV{CTEST_USE_INSTRUMENTATION})
set(ENV{CDASH_TOKEN} "test-token")

# Build the examples with the same compilers and build tool as CMake.
set(ENV{CC} "${CMAKE_C_COMPILER}")
set(ENV{CXX} "${CMAKE_CXX_COMPILER}")
if(RunCMake_MAKE_PROGRAM)
  cmake_path(GET RunCMake_MAKE_PROGRAM PARENT_PATH make_program_dir)
  cmake_path(CONVERT "${make_program_dir}" TO_NATIVE_PATH_LIST make_program_dir)
  if(CMAKE_HOST_WIN32)
    set(ENV{PATH} "${make_program_dir};$ENV{PATH}")
  else()
    set(ENV{PATH} "${make_program_dir}:$ENV{PATH}")
  endif()
endif()

run_cmake(Inspect)
include("${RunCMake_BINARY_DIR}/Inspect-build/info.cmake")

# Copy the guide's examples so that the scripts do not write to the source tree.
set(guide_dir "${RunCMake_BINARY_DIR}/guide")
file(REMOVE_RECURSE "${guide_dir}")
file(COPY "${Guide_SOURCE_DIR}/" DESTINATION "${guide_dir}"
  PATTERN "*.rst" EXCLUDE
  REGEX "/build[^/]*$" EXCLUDE
)

function(replace_in_file file old new)
  file(READ "${guide_dir}/${file}" content)
  string(FIND "${content}" "${old}" pos)
  if(pos EQUAL -1)
    message(FATAL_ERROR "Text not found in ${file}:\n ${old}")
  endif()
  string(REPLACE "${old}" "${new}" content "${content}")
  file(WRITE "${guide_dir}/${file}" "${content}")
endfunction()

# Use the generator under test instead of the one the examples name.
set(generator_code "set(CTEST_CMAKE_GENERATOR \"${RunCMake_GENERATOR}\")")
if(RunCMake_GENERATOR_PLATFORM)
  string(APPEND generator_code
    "\nset(CTEST_CMAKE_GENERATOR_PLATFORM \"${RunCMake_GENERATOR_PLATFORM}\")"
  )
endif()
if(RunCMake_GENERATOR_TOOLSET)
  string(APPEND generator_code
    "\nset(CTEST_CMAKE_GENERATOR_TOOLSET \"${RunCMake_GENERATOR_TOOLSET}\")"
  )
endif()
foreach(script IN ITEMS
  Step1/submit.cmake
  Step2/submit.cmake
  Step3/submit.cmake
  Step4/asan.cmake
  Step4/valgrind.cmake
  Step5/submit.cmake
  Step6/submit.cmake
  Step7/submit.cmake
)
  replace_in_file(${script}
    "set(CTEST_CMAKE_GENERATOR \"Unix Makefiles\")" "${generator_code}"
  )
endforeach()
replace_in_file(Step8/CMakePresets.json
  "\"generator\": \"Unix Makefiles\""
  "\"generator\": \"${RunCMake_GENERATOR}\""
)
# N.B. ctest_test doesn't respect the "configuration" specified in a test
# preset, so pass the configuration explicitly for multi-config generators.
if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  foreach(step IN ITEMS Build Test)
    replace_in_file(Step9/submit.sh
      "ctest -T ${step} -T Submit"
      "ctest -T ${step} -T Submit -C Debug"
    )
  endforeach()
endif()

# Use the gcov that matches the compiler instead of the first one in the PATH.
# See Inspect.cmake.
if(can_code_coverage)
  foreach(script IN ITEMS
    Step3/submit.cmake
    Step5/submit.cmake
    Step6/submit.cmake
    Step7/submit.cmake
    Step8/submit.cmake
  )
    replace_in_file(${script}
      "set(CTEST_COVERAGE_COMMAND \"gcov\")"
      "set(CTEST_COVERAGE_COMMAND \"${GCOV}\")"
    )
  endforeach()
  replace_in_file(Step9/submit.sh
    "ctest -T Coverage -T Submit"
    "ctest -T Coverage -T Submit -D CTEST_COVERAGE_COMMAND=\"${GCOV}\""
  )
endif()

# Submit to a local stand-in for CDash instead of my.cdash.org.
start_cdash_server()

foreach(file IN ITEMS
  Project/CTestConfig.cmake
  Step4/Project/CTestConfig.cmake
  Step7/submit.cmake
)
  replace_in_file(${file} "https://my.cdash.org/" "${server_url}/")
endforeach()
file(GLOB_RECURSE guide_files "${guide_dir}/*")
foreach(file IN LISTS guide_files)
  file(STRINGS "${file}" remote REGEX "my\\.cdash\\.org")
  if(remote)
    message(FATAL_ERROR "Submission URL not replaced in:\n ${file}")
  endif()
endforeach()

# The update step warns if it is performed inside a source checkout with local
# changes. In many cases the build tree in which this test runs will be under
# the source tree, so ignore these warnings so we don't hinder local
# development.
set(RunCMake_TEST_EXPECT_stderr
  "^(   There are modified or conflicting files in the repository)?$")
set(RunCMake_TEST_NOT_EXPECT_stderr "CMake Error")
set(RunCMake_TEST_COMMAND_WORKING_DIRECTORY "${guide_dir}")

function(run_step name script build_dir)
  set(build_dir "${guide_dir}/${build_dir}")
  ping_cdash_server()
  run_cmake_command(${name} ${CMAKE_CTEST_COMMAND} -C Debug -S ${script})
endfunction()

run_step(Step1 Step1/submit.cmake Step1/build)

if(RunCMake_GENERATOR MATCHES "Make|Ninja")
  block()
    set(RunCMake_TEST_EXPECT_RESULT "[^0]")
    set(RunCMake_TEST_EXPECT_stderr "Error\\(s\\) when building project")
    run_step(Step2 Step2/submit.cmake Step2/build)
  endblock()
endif()

if(RunCMake_GENERATOR MATCHES "Make|Ninja" AND NOT CMAKE_HOST_WIN32 AND can_asan)
  run_step(Step4-asan Step4/asan.cmake Step4/build-asan)
endif()

if(NOT CMAKE_HOST_WIN32 AND can_valgrind)
  run_step(Step4-valgrind Step4/valgrind.cmake Step4/build-valgrind)
endif()

if(RunCMake_GENERATOR MATCHES "Make|Ninja" AND can_code_coverage)
  # Isolate the coverage steps from any surrounding coverage tool.
  set(ENV{COVNOSAVE} 1)
  unset(ENV{COVAPPDATADIR})
  unset(ENV{COVFILE})

  run_step(Step3 Step3/submit.cmake Step3/build)
  run_step(Step5 Step5/submit.cmake Step5/build)
  run_step(Step6 Step6/submit.cmake Step6/build)
  run_step(Step7 Step7/submit.cmake Step7/build)
  run_step(Step8 Step8/submit.cmake Step8/build)

  if(can_sh)
    cmake_path(GET CMAKE_CTEST_COMMAND PARENT_PATH ctest_dir)
    set(build_dir "${guide_dir}/Step9/build")
    set(RunCMake_TEST_EXPECT_stderr
      "^Cannot find file: [^\n]*/DartConfiguration\\.tcl")
    ping_cdash_server()
    run_cmake_command(Step9
      ${CMAKE_COMMAND} -E env --modify
      "PATH=path_list_prepend:${ctest_dir}"
      "${SH}" Step9/submit.sh
    )
  endif()
endif()

stop_cdash_server()
