cmake_minimum_required(VERSION 3.31)

include(RunCMake)

# run_coverage(<case> [COV_DIR <dir>] [COMMAND <run_cmake helper and args>])
#
# Run COMMAND, by default run_cmake_script(<case>), with the environment
# variable CMAKE_LISTFILE_COVERAGE_DIR set to COV_DIR, by default
# ${RunCMake_BINARY_DIR}/<case>-coverage.  An absolute COV_DIR is removed
# first.  A relative one is never removed.  The variable cov_dir is set for
# <case>-check.cmake, which includes check-lcov.cmake.
function(run_coverage case)
  cmake_parse_arguments(PARSE_ARGV 1 arg "" "COV_DIR" "COMMAND")
  if(DEFINED arg_COV_DIR)
    set(cov_dir "${arg_COV_DIR}")
  else()
    set(cov_dir "${RunCMake_BINARY_DIR}/${case}-coverage")
  endif()
  if(NOT arg_COMMAND)
    set(arg_COMMAND run_cmake_script ${case})
  endif()
  if(IS_ABSOLUTE "${cov_dir}")
    file(REMOVE_RECURSE "${cov_dir}")
  endif()

  if(DEFINED ENV{CMAKE_LISTFILE_COVERAGE_DIR})
    set(had_old TRUE)
    set(old "$ENV{CMAKE_LISTFILE_COVERAGE_DIR}")
  else()
    set(had_old FALSE)
  endif()
  set(ENV{CMAKE_LISTFILE_COVERAGE_DIR} "${cov_dir}")
  list(POP_FRONT arg_COMMAND command)
  cmake_language(CALL ${command} ${arg_COMMAND})
  if(had_old)
    set(ENV{CMAKE_LISTFILE_COVERAGE_DIR} "${old}")
  else()
    unset(ENV{CMAKE_LISTFILE_COVERAGE_DIR})
  endif()
endfunction()

run_coverage(Lines)
run_coverage(Functions)
run_coverage(FatalErrorNested)
run_coverage(ControlFlow)
run_coverage(Callbacks)
run_coverage(CrossFile)
run_coverage(IncludeTwice)
run_coverage(Eval)
run_coverage(Exit)
run_coverage(ParseError)
run_coverage(Child COV_DIR "${RunCMake_BINARY_DIR}/Child cov dir")
run_coverage(CommentOnly)
run_coverage(CommentOnlyScript COMMAND run_cmake_command CommentOnlyScript
  ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/CommentOnlyLib.cmake)

# Startup errors.  The script must not run.
run_coverage(RelativeDir COV_DIR "relative/dir")
file(WRITE "${RunCMake_BINARY_DIR}/BadDir-file" "")
block()
  set(RunCMake_TEST_NOT_EXPECT_stderr "must not run")
  run_coverage(BadDir COV_DIR "${RunCMake_BINARY_DIR}/BadDir-file/sub")
endblock()

# An empty value turns coverage off.  set(ENV{...} "") would unset it.
run_cmake_command(EmptyDir ${CMAKE_COMMAND} -E env CMAKE_LISTFILE_COVERAGE_DIR=
  ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/EmptyDir.cmake)

# Configure and install a project.
run_coverage(Configure COMMAND run_cmake Configure)
block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/Configure-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_coverage(Configure-install COMMAND run_cmake_command Configure-install
    ${CMAKE_COMMAND} --install . --prefix ${RunCMake_TEST_BINARY_DIR}/prefix)
endblock()

# Package a project.  Only cpack records.
block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CPack-build)
  run_cmake(CPack)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_coverage(CPack-package COMMAND run_cmake_command CPack-package
    ${CMAKE_CPACK_COMMAND} -G TGZ -C Debug)
endblock()

# Modes that do not record.  The variable is set only in the child, so no
# driver process creates the directory.
function(run_no_record case dir)
  set(RunCMake-check-file NoRecord-check.cmake)
  if(IS_ABSOLUTE "${dir}")
    set(no_cov_dir "${dir}")
    file(REMOVE_RECURSE "${no_cov_dir}")
  else()
    set(no_cov_dir "${RunCMake_BINARY_DIR}/${case}-build/${dir}")
  endif()
  run_cmake_command(${case} ${CMAKE_COMMAND} -E env
    CMAKE_LISTFILE_COVERAGE_DIR=${dir} ${ARGN})
endfunction()

run_no_record(NoRecord-E "${RunCMake_BINARY_DIR}/NoRecord-E-coverage"
  ${CMAKE_COMMAND} -E true)
block()
  set(tree ${RunCMake_BINARY_DIR}/NoRecord-launch-build)
  file(REMOVE_RECURSE "${tree}")
  file(WRITE "${tree}/logs/CTestLaunchConfig.cmake" "set(launch_config 1)\n")
  set(RunCMake_TEST_BINARY_DIR "${tree}")
  set(RunCMake_TEST_NO_CLEAN 1)
  run_no_record(NoRecord-launch "${RunCMake_BINARY_DIR}/NoRecord-launch-coverage"
    CTEST_LAUNCH_LOGS=${tree}/logs ${CMAKE_CTEST_COMMAND} --launch
    --build-dir ${tree} --output ${tree}/o -- ${CMAKE_COMMAND} -E false)
endblock()
run_no_record(NoRecord-version "relative/dir" ${CMAKE_COMMAND} --version)
run_no_record(NoRecord-ctest-version "relative/dir"
  ${CMAKE_CTEST_COMMAND} --version)

# The --check-build-system step of a build records only when it re-runs.
block()
  set(src ${RunCMake_SOURCE_DIR}/CheckBuildSystem)
  set(tree ${RunCMake_BINARY_DIR}/CheckBuildSystem-build)
  set(RunCMake_TEST_SOURCE_DIR ${src})
  set(RunCMake_TEST_BINARY_DIR ${tree})
  run_cmake(CheckBuildSystem)
  set(RunCMake_TEST_NO_CLEAN 1)

  # CMake reads PreLoad.cmake from the binary dir too.
  file(WRITE "${tree}/PreLoad.cmake" "set(preload 1)\n")
  set(check_command ${CMAKE_COMMAND} -S ${src} -B ${tree}
    --check-build-system ${tree}/check.cmake 0)

  # The outputs are written last, so they are not older than the depends.
  file(WRITE "${tree}/check.cmake" "
set(CMAKE_MAKEFILE_DEPENDS \"${tree}/depends.txt\")
set(CMAKE_MAKEFILE_OUTPUTS \"${tree}/outputs.txt\")
")
  file(WRITE "${tree}/depends.txt" "")
  file(WRITE "${tree}/outputs.txt" "")
  run_coverage(CheckBuildSystem-uptodate
    COMMAND run_cmake_command CheckBuildSystem-uptodate ${check_command})

  # A missing product forces a re-configure.
  file(APPEND "${tree}/check.cmake"
    "set(CMAKE_MAKEFILE_PRODUCTS \"${tree}/missing.txt\")\n")
  run_coverage(CheckBuildSystem-rerun
    COMMAND run_cmake_command CheckBuildSystem-rerun ${check_command})
endblock()
