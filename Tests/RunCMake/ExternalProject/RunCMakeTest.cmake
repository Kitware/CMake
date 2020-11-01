cmake_minimum_required(VERSION 3.12)
include(RunCMake)

# We do not contact any remote URLs, but may use a local one.
# Remove any proxy configuration that may change behavior.
unset(ENV{http_proxy})
unset(ENV{https_proxy})

run_cmake(BadIndependentStep1)
run_cmake(BadIndependentStep2)
run_cmake(IncludeScope-Add)
run_cmake(IncludeScope-Add_Step)
run_cmake(NoOptions)
run_cmake(SourceEmpty)
run_cmake(SourceMissing)
run_cmake(CMAKE_CACHE_ARGS)
run_cmake(CMAKE_CACHE_DEFAULT_ARGS)
run_cmake(CMAKE_CACHE_mix)
if(NOT XCODE_VERSION OR XCODE_VERSION VERSION_LESS 12)
  run_cmake(NO_DEPENDS-CMP0114-WARN)
  run_cmake(NO_DEPENDS-CMP0114-OLD)
endif()
run_cmake(NO_DEPENDS-CMP0114-NEW)
run_cmake(NO_DEPENDS-CMP0114-NEW-Direct)
run_cmake(Add_StepDependencies)
run_cmake(Add_StepDependencies_iface)
run_cmake(Add_StepDependencies_iface_step)
run_cmake(Add_StepDependencies_no_target)
run_cmake(UsesTerminal)
if(XCODE_VERSION AND XCODE_VERSION VERSION_GREATER_EQUAL 12)
  run_cmake(Xcode-CMP0114)
endif()

macro(check_steps_missing proj)
  set(steps "${ARGN}")
  foreach(step ${steps})
    if(EXISTS ${RunCMake_TEST_BINARY_DIR}/${proj}-${step}-mark)
      string(APPEND RunCMake_TEST_FAILED "${proj} '${step}' step ran but should not have\n")
    endif()
  endforeach()
endmacro()

macro(check_steps_present proj)
  set(steps "${ARGN}")
  foreach(step ${steps})
    if(NOT EXISTS ${RunCMake_TEST_BINARY_DIR}/${proj}-${step}-mark)
      string(APPEND RunCMake_TEST_FAILED "${proj} '${step}' step did not run but should have\n")
    endif()
  endforeach()
endmacro()

function(run_steps_CMP0114 val)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/Steps-CMP0114-${val}-build)
  run_cmake(Steps-CMP0114-${val})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(Steps-CMP0114-${val}-build-download ${CMAKE_COMMAND} --build . --target proj1-download)
  run_cmake_command(Steps-CMP0114-${val}-build-update ${CMAKE_COMMAND} --build . --target proj1-update)
  run_cmake_command(Steps-CMP0114-${val}-build-install ${CMAKE_COMMAND} --build . --target proj1-install)
  run_cmake_command(Steps-CMP0114-${val}-build-test ${CMAKE_COMMAND} --build . --target proj1-test)
endfunction()
if(NOT XCODE_VERSION OR XCODE_VERSION VERSION_LESS 12)
  run_steps_CMP0114(OLD)
endif()
run_steps_CMP0114(NEW)

# Run both cmake and build steps. We always do a clean before the
# build to ensure that the download step re-runs each time.
function(__ep_test_with_build testName)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${testName}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake(${testName})
  run_cmake_command(${testName}-clean ${CMAKE_COMMAND} --build . --target clean)
  run_cmake_command(${testName}-build ${CMAKE_COMMAND} --build .)
endfunction()

find_package(Python3)
function(__ep_test_with_build_with_server testName)
  if(NOT Python3_EXECUTABLE)
    return()
  endif()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${testName}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_TIMEOUT 20)
  set(RunCMake_TEST_OUTPUT_MERGE TRUE)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  set(URL_FILE ${RunCMake_BINARY_DIR}/${testName}.url)
  if(EXISTS "${URL_FILE}")
    file(REMOVE "${URL_FILE}")
  endif()
  execute_process(
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/DownloadServer.py --file "${URL_FILE}" ${ARGN}
    OUTPUT_FILE ${RunCMake_BINARY_DIR}/${testName}-python.txt
    ERROR_FILE ${RunCMake_BINARY_DIR}/${testName}-python.txt
    RESULT_VARIABLE result
    TIMEOUT 30
    )
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "Failed to start download server:\n  ${result}")
  endif()

  foreach(i RANGE 1 8)
    if(EXISTS ${URL_FILE})
      break()
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} -E sleep ${i})
  endforeach()

  if(NOT EXISTS ${URL_FILE})
    message(FATAL_ERROR "Failed to load download server URL from:\n  ${URL_FILE}")
  endif()

  file(READ ${URL_FILE} SERVER_URL)
  message(STATUS "URL : ${URL_FILE} - ${SERVER_URL}")
  run_cmake_with_options(${testName} ${CMAKE_COMMAND} -DSERVER_URL=${SERVER_URL} )
  run_cmake_command(${testName}-clean ${CMAKE_COMMAND} --build . --target clean)
  run_cmake_command(${testName}-build ${CMAKE_COMMAND} --build .)
endfunction()

__ep_test_with_build(MultiCommand)

set(RunCMake_TEST_OUTPUT_MERGE 1)
__ep_test_with_build(PreserveEmptyArgs)
set(RunCMake_TEST_OUTPUT_MERGE 0)

# Output is not predictable enough to be able to verify it reliably
# when using the various different Visual Studio generators
if(NOT RunCMake_GENERATOR MATCHES "Visual Studio")
  __ep_test_with_build(LogOutputOnFailure)
  __ep_test_with_build(LogOutputOnFailureMerged)
  __ep_test_with_build(DownloadTimeout)
  __ep_test_with_build_with_server(DownloadInactivityTimeout --speed_limit --limit_duration 40)
  __ep_test_with_build_with_server(DownloadInactivityResume --speed_limit --limit_duration 1)
endif()

# We can't test the substitution when using the old MSYS due to
# make/sh mangling the paths (substitution is performed correctly,
# but the mangling means we can't reliably test the output).
# There is no such issue when using the newer MSYS though. Therefore,
# we need to bypass the substitution test if using old MSYS.
# See merge request 1537 for discussion.
set(doSubstitutionTest YES)
if(RunCMake_GENERATOR STREQUAL "MSYS Makefiles")
  execute_process(COMMAND uname OUTPUT_VARIABLE uname)
  if(uname MATCHES "^MINGW32_NT")
      set(doSubstitutionTest NO)
  endif()
endif()
if(doSubstitutionTest)
    __ep_test_with_build(Substitutions)
endif()
