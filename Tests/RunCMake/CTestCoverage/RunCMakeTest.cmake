include(RunCMake)

if(CMake_TEST_CLANG_COVERAGE)
  block()
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/ClangCoverage-build)
    if(NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
      list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
    endif()
    run_cmake(ClangCoverage)
    set(RunCMake_TEST_NO_CLEAN 1)
    run_cmake_command(ClangCoverage-build ${CMAKE_COMMAND} --build . --config Debug)
    run_cmake_command(ClangCoverage-ctest ${CMAKE_CTEST_COMMAND} -C Debug -VV)
    run_cmake_command(ClangCoverage-coverage ${CMAKE_CTEST_COMMAND} -D ExperimentalCoverage -C Debug -VV)
  endblock()
endif()

# Isolate the coverage step from any surrounding coverage tool.  With
# COVFILE set, the coverage handler uses Bullseye instead of CMAKE.
set(ctest_coverage ${CMAKE_COMMAND} -E env COVNOSAVE=1
  --unset=COVAPPDATADIR --unset=COVFILE ${CMAKE_CTEST_COMMAND})

function(run_CMakeListFileCoverage case)
  run_cmake(${case})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${case}-ctest ${CMAKE_CTEST_COMMAND} -C Debug -VV)
  run_cmake_command(${case}-coverage ${ctest_coverage} -D ExperimentalCoverage -C Debug -VV)
endfunction()

block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CMakeListFileCoverage-build)
  run_CMakeListFileCoverage(CMakeListFileCoverage)
endblock()

# A build tree inside the source tree.
block()
  set(src ${RunCMake_BINARY_DIR}/nested-src)
  file(REMOVE_RECURSE "${src}")
  file(MAKE_DIRECTORY "${src}")
  file(COPY_FILE ${RunCMake_SOURCE_DIR}/CMakeLists.txt ${src}/CMakeLists.txt)
  file(COPY_FILE ${RunCMake_SOURCE_DIR}/Shared.cmake ${src}/Shared.cmake)
  file(COPY_FILE ${RunCMake_SOURCE_DIR}/CMakeListFileCoverage.cmake
    ${src}/CMakeListFileCoverage-nested.cmake)
  set(RunCMake_TEST_SOURCE_DIR ${src})
  set(RunCMake_TEST_BINARY_DIR ${src}/build)
  run_CMakeListFileCoverage(CMakeListFileCoverage-nested)
endblock()

# A line past the end of the file, and a truncated file.
block()
  set(tree ${RunCMake_BINARY_DIR}/CMakeListFileCoverage-range)
  set(src ${RunCMake_BINARY_DIR}/CMakeListFileCoverage-range-src)
  file(REMOVE_RECURSE "${tree}" "${src}")
  file(WRITE "${src}/three.cmake" "set(a 1)\nset(b 2)\nset(c 3)\n")
  file(WRITE "${tree}/DartConfiguration.tcl" "
SourceDirectory: ${src}
BuildDirectory: ${tree}
")
  file(WRITE "${tree}/bad.cmake-coverage.info"
    "SF:${src}/three.cmake\nDA:10,1\nend_of_record\n")
  file(WRITE "${tree}/trunc.cmake-coverage.info"
    "SF:${src}/three.cmake\nDA:1,1\n")
  set(RunCMake_TEST_BINARY_DIR ${tree})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(CMakeListFileCoverage-range ${ctest_coverage} -D ExperimentalCoverage -VV)
endblock()

# Only a record with no lines and no functions: no coverage data.
block()
  set(tree ${RunCMake_BINARY_DIR}/CMakeListFileCoverage-empty)
  set(src ${RunCMake_BINARY_DIR}/CMakeListFileCoverage-empty-src)
  file(REMOVE_RECURSE "${tree}" "${src}")
  file(WRITE "${src}/comments.cmake" "# Only a comment.\n")
  file(WRITE "${tree}/DartConfiguration.tcl" "
SourceDirectory: ${src}
BuildDirectory: ${tree}
")
  file(WRITE "${tree}/empty.cmake-coverage.info"
    "SF:${src}/comments.cmake\nend_of_record\n")
  set(RunCMake_TEST_BINARY_DIR ${tree})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(CMakeListFileCoverage-empty ${ctest_coverage} -D ExperimentalCoverage)
endblock()

# ctest records its own CMake code when the variable is set at startup.
block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/SelfCoverage)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CTestTestfile.cmake" "
add_test(t1 \"${CMAKE_COMMAND}\" -E true)
")
  set(cov_dir "${RunCMake_TEST_BINARY_DIR}/coverage")
  run_cmake_command(SelfCoverage ${CMAKE_COMMAND} -E env
    CMAKE_LISTFILE_COVERAGE_DIR=${cov_dir} ${CMAKE_CTEST_COMMAND})
endblock()

# The CMAKE coverage tool.  Make a clean test tree that uses it, with the
# given CTestTestfile.cmake content, and set tree in the caller.
function(prepare_CoverageToolCMake case testfile)
  set(tree ${RunCMake_BINARY_DIR}/${case})
  file(REMOVE_RECURSE "${tree}")
  file(MAKE_DIRECTORY "${tree}")
  file(WRITE "${tree}/DartConfiguration.tcl" "
BuildDirectory: ${tree}
CTestTestCoverageTool: CMAKE
")
  file(WRITE "${tree}/CTestTestfile.cmake" "${testfile}")
  set(tree "${tree}" PARENT_SCOPE)
endfunction()

block()
  prepare_CoverageToolCMake(CoverageToolCMake "
add_test(\"test one\" \"${CMAKE_COMMAND}\" -P \"${RunCMake_SOURCE_DIR}/CoverageScript.cmake\")
add_test(nocode \"${CMAKE_COMMAND}\" -E true)
add_test(generated \"${CMAKE_COMMAND}\" -P \"${RunCMake_BINARY_DIR}/CoverageToolCMake/generated.cmake\")
add_test(buildonly \"${CMAKE_COMMAND}\" -P \"${RunCMake_BINARY_DIR}/CoverageToolCMake/buildonly.cmake\")
add_test(commentonly \"${CMAKE_COMMAND}\" -P \"${RunCMake_SOURCE_DIR}/CoverageCommentOnly.cmake\")
add_test(emptyrecord \"${CMAKE_COMMAND}\" -P \"${RunCMake_BINARY_DIR}/CoverageToolCMake/emptyrecord.cmake\")
")
  set(RunCMake_TEST_BINARY_DIR ${tree})
  set(RunCMake_TEST_NO_CLEAN 1)
  # Stale data of an earlier run.  The stale file is valid and names a file
  # outside the build tree, so a missing cleanup shows up as merged data.
  file(WRITE "${tree}/nocode_cmake-coverage/stale.info"
    "SF:${RunCMake_SOURCE_DIR}/stale.cmake\nDA:1,1\nend_of_record\n")
  # Scripts in the build tree.  The merge leaves out their records.
  file(WRITE "${tree}/generated.cmake"
    "include(\"${RunCMake_SOURCE_DIR}/CoverageChild.cmake\")\n")
  file(WRITE "${tree}/buildonly.cmake" "set(x 1)\n")
  # This script writes a record with no lines for a file in the source tree.
  # After the merge, only that record is left.
  file(WRITE "${tree}/emptyrecord.cmake" "
if(DEFINED ENV{CMAKE_LISTFILE_COVERAGE_DIR})
  file(WRITE \"\$ENV{CMAKE_LISTFILE_COVERAGE_DIR}/empty.info\"
    \"SF:${RunCMake_SOURCE_DIR}/CoverageChild.cmake\\nend_of_record\\n\")
endif()
")
  file(WRITE "${tree}/nocode.cmake-coverage.info" "")
  run_cmake_command(CoverageToolCMake ${CMAKE_COMMAND} -E env
    CMAKE_LISTFILE_COVERAGE_DIR=${tree}/ambient ${CMAKE_CTEST_COMMAND} -V)
endblock()

block()
  prepare_CoverageToolCMake(CoverageToolCMake-insource "
add_test(insource \"${CMAKE_COMMAND}\" -P \"${RunCMake_BINARY_DIR}/CoverageToolCMake-insource/script.cmake\")
")
  set(RunCMake_TEST_BINARY_DIR ${tree})
  set(RunCMake_TEST_NO_CLEAN 1)
  # In an in-source build, the merge keeps the files of the build tree.
  file(APPEND "${tree}/DartConfiguration.tcl" "SourceDirectory: ${tree}\n")
  file(WRITE "${tree}/script.cmake" "set(x 1)\n")
  run_cmake_command(CoverageToolCMake-insource ${CMAKE_CTEST_COMMAND})
endblock()

block()
  prepare_CoverageToolCMake(CoverageToolCMake-bad "
add_test(bad \"${CMAKE_COMMAND}\" -P \"${RunCMake_SOURCE_DIR}/CoverageBad.cmake\")
")
  set(RunCMake_TEST_BINARY_DIR ${tree})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(CoverageToolCMake-bad ${CMAKE_CTEST_COMMAND})
endblock()

block()
  prepare_CoverageToolCMake(CoverageToolCMake-writefail "
add_test(wf \"${CMAKE_COMMAND}\" -P \"${RunCMake_SOURCE_DIR}/CoverageChild.cmake\")
")
  set(RunCMake_TEST_BINARY_DIR ${tree})
  set(RunCMake_TEST_NO_CLEAN 1)
  # The cleanup does not remove a directory, so the final write fails.
  file(MAKE_DIRECTORY "${tree}/wf.cmake-coverage.info")
  run_cmake_command(CoverageToolCMake-writefail ${CMAKE_CTEST_COMMAND})
endblock()

block()
  prepare_CoverageToolCMake(CoverageToolCMake-timeout "
add_test(sleeper \"${CMAKE_COMMAND}\" -E sleep 10)
set_tests_properties(sleeper PROPERTIES TIMEOUT 1)
")
  set(RunCMake_TEST_BINARY_DIR ${tree})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(CoverageToolCMake-timeout ${CMAKE_CTEST_COMMAND})
endblock()

block()
  prepare_CoverageToolCMake(CoverageToolCMake-noscratch "
add_test(blocked \"${CMAKE_COMMAND}\" -P \"${RunCMake_SOURCE_DIR}/CoverageBlocked.cmake\")
")
  set(RunCMake_TEST_BINARY_DIR ${tree})
  set(RunCMake_TEST_NO_CLEAN 1)
  # A regular file blocks the scratch folder.
  file(WRITE "${tree}/blocked_cmake-coverage" "")
  run_cmake_command(CoverageToolCMake-noscratch ${CMAKE_CTEST_COMMAND})
endblock()

if(NOT CMAKE_HOST_WIN32)
  block()
    prepare_CoverageToolCMake(CoverageToolCMake-symlink "
add_test(linked \"${CMAKE_COMMAND}\" -P \"${RunCMake_SOURCE_DIR}/CoverageBlocked.cmake\")
")
    set(RunCMake_TEST_BINARY_DIR ${tree})
    set(RunCMake_TEST_NO_CLEAN 1)
    # A symbolic link at the scratch folder path points outside the tree.
    # ctest must not record into the link target or delete its content.
    set(target "${RunCMake_BINARY_DIR}/CoverageToolCMake-symlink-target")
    file(REMOVE_RECURSE "${target}")
    file(WRITE "${target}/keep.txt" "")
    file(CREATE_LINK "${target}" "${tree}/linked_cmake-coverage" SYMBOLIC)
    run_cmake_command(CoverageToolCMake-symlink ${CMAKE_CTEST_COMMAND})
  endblock()
endif()
