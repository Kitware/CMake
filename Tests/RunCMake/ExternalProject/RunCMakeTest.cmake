cmake_minimum_required(VERSION 3.12)
include(RunCMake)

# We do not contact any remote URLs, but may use a local one.
# Remove any proxy configuration that may change behavior.
unset(ENV{http_proxy})
unset(ENV{https_proxy})

if(RunCMake_GENERATOR STREQUAL "Borland Makefiles" OR
   RunCMake_GENERATOR STREQUAL "Watcom WMake")
  set(fs_delay 3)
else()
  set(fs_delay 1.125)
endif()

run_cmake(BadIndependentStep1)
run_cmake(BadIndependentStep2)
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

function(__ep_test_source_dir_change)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/SourceDirChange-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake(SourceDirChange)
  run_cmake_command(SourceDirChange-build1 ${CMAKE_COMMAND} --build .)
  # Because some file systems have timestamps with only one second resolution,
  # we have to ensure we don't re-run the configure stage too quickly after the
  # first build. Otherwise, the modified RepositoryInfo.txt files the next
  # configure writes might still have the same timestamp as the previous one.
  execute_process(COMMAND ${CMAKE_COMMAND} -E sleep ${fs_delay})
  run_cmake_command(SourceDirChange-change ${CMAKE_COMMAND} -DSOURCE_DIR_CHANGE=YES .)
  run_cmake_command(SourceDirChange-build2 ${CMAKE_COMMAND} --build .)
endfunction()
__ep_test_source_dir_change()

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
  if(NOT DOWNLOAD_SERVER_TIMEOUT)
    set(DOWNLOAD_SERVER_TIMEOUT 30)
  endif()
  execute_process(
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/DownloadServer.py --file "${URL_FILE}" ${ARGN}
    OUTPUT_FILE ${RunCMake_BINARY_DIR}/${testName}-python.txt
    ERROR_FILE ${RunCMake_BINARY_DIR}/${testName}-python.txt
    RESULT_VARIABLE result
    TIMEOUT "${DOWNLOAD_SERVER_TIMEOUT}"
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
  run_cmake_with_options(${testName} -DSERVER_URL=${SERVER_URL})
  run_cmake_command(${testName}-clean ${CMAKE_COMMAND} --build . --target clean)
  run_cmake_command(${testName}-build ${CMAKE_COMMAND} --build .)
endfunction()

if(RunCMake_GENERATOR MATCHES "(MSYS|MinGW|Unix) Makefiles")
  __ep_test_with_build(GNUMakeJobServerAware)
endif()

function(__ep_test_jobserver)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/DetectJobServer-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake_with_options(DetectJobServer -DDETECT_JOBSERVER=${DETECT_JOBSERVER})
  run_cmake_command(DetectJobServer-clean ${CMAKE_COMMAND} --build . --target clean)
  run_cmake_command(DetectJobServer-build ${CMAKE_COMMAND} --build . -j4)
endfunction()

if(RunCMake_GENERATOR MATCHES "(MinGW|Unix) Makefiles")
  __ep_test_jobserver()
endif()

__ep_test_with_build(MultiCommand)

set(RunCMake_TEST_OUTPUT_MERGE 1)
__ep_test_with_build(InstallBuilds)
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

function(__ep_test_BUILD_ALWAYS)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/BUILD_ALWAYS-build)
  run_cmake(BUILD_ALWAYS)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/once-configure.cmake" [[message(STATUS "once: configure")]])
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/once-build.cmake" [[message(STATUS "once: build")]])
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/once-install.cmake" [[message(STATUS "once: install")]])
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/always-configure.cmake" [[message(STATUS "always: configure")]])
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/always-build.cmake" [[message(STATUS "always: build")]])
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/always-install.cmake" [[message(STATUS "always: install")]])
  run_cmake_command(BUILD_ALWAYS-build1 ${CMAKE_COMMAND} --build . --target always)
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/once-configure.cmake" [[message(FATAL_ERROR "once: configure should not run again")]])
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/once-build.cmake" [[message(FATAL_ERROR "once: build should not run again")]])
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/once-install.cmake" [[message(FATAL_ERROR "once: install should not run again")]])
  if(NOT RunCMake_GENERATOR MATCHES "^(Xcode|Visual Studio 9 )")
    # The Xcode and VS 9 build systems decide to run this every time.
    file(WRITE "${RunCMake_TEST_BINARY_DIR}/always-configure.cmake" [[message(FATAL_ERROR "always: configure should not run again")]])
  endif()
  run_cmake_command(BUILD_ALWAYS-build2 ${CMAKE_COMMAND} --build . --target always)
endfunction()
__ep_test_BUILD_ALWAYS()

function(__ep_test_CONFIGURE_HANDLED_BY_BUILD)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CONFIGURE_HANDLED_BY_BUILD-build)
  run_cmake(CONFIGURE_HANDLED_BY_BUILD)

  if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(BUILD_CONFIG --config Debug)
    set(STAMP_DIR "${RunCMake_TEST_BINARY_DIR}/stamp/Debug")
  else()
    set(BUILD_CONFIG "")
    set(STAMP_DIR "${RunCMake_TEST_BINARY_DIR}/stamp")
  endif()

  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(CONFIGURE_HANDLED_BY_BUILD-build ${CMAKE_COMMAND} --build . ${BUILD_CONFIG})

  # Calculate timestamps before rebuilding so we can compare before and after in
  # CONFIGURE_HANDLED_BY_BUILD-rebuild-check.cmake

  file(TIMESTAMP "${STAMP_DIR}/proj1-configure" PROJ1_CONFIGURE_TIMESTAMP_BEFORE "%s")
  # When BUILD_ALWAYS is set, the build stamp is never created.
  file(TIMESTAMP "${STAMP_DIR}/proj2-configure" PROJ2_CONFIGURE_TIMESTAMP_BEFORE "%s")
  file(TIMESTAMP "${STAMP_DIR}/proj2-build" PROJ2_BUILD_TIMESTAMP_BEFORE "%s")

  run_cmake_command(CONFIGURE_HANDLED_BY_BUILD-rebuild ${CMAKE_COMMAND} --build . ${BUILD_CONFIG})
endfunction()

if(NOT RunCMake_GENERATOR MATCHES "Visual Studio 9 ")
  __ep_test_CONFIGURE_HANDLED_BY_BUILD()
endif()

find_package(Git QUIET)
if(GIT_EXECUTABLE)
  # Note that there appear to be differences in where git writes its output to
  # on some platforms. It may go to stdout or stderr, so force it to be merged.
  set(RunCMake_TEST_OUTPUT_MERGE TRUE)
  run_cmake(FetchGitRefs)
  set(RunCMake_TEST_OUTPUT_MERGE FALSE)
endif()
