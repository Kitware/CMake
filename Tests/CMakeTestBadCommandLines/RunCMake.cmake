if(NOT DEFINED CMake_SOURCE_DIR)
  message(FATAL_ERROR "CMake_SOURCE_DIR not defined")
endif()

if(NOT DEFINED dir)
  message(FATAL_ERROR "dir not defined")
endif()

if(NOT DEFINED gen)
  message(FATAL_ERROR "gen not defined")
endif()

message(STATUS "CTEST_FULL_OUTPUT (Avoid ctest truncation of output)")

# First setup a source tree to run CMake on.
#
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMake_SOURCE_DIR}/Tests/CTestTest/SmallAndFast
  ${dir}/Source
)

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory
  ${dir}/Build
  )

function(RunCMakeWithArgs)
  message(STATUS "info: running cmake with ARGN='${ARGN}'")

  execute_process(COMMAND ${CMAKE_COMMAND} ${ARGN}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE stdout
    ERROR_VARIABLE stderr
    WORKING_DIRECTORY ${dir}/Build
    )

  message(STATUS "result='${result}'")
  message(STATUS "stdout='${stdout}'")
  message(STATUS "stderr='${stderr}'")
  message(STATUS "")
endfunction()

# Run cmake once with no errors to get a good build tree:
#
RunCMakeWithArgs(-G ${gen} ../Source)

# Run cmake with args that produce some sort of problem to cover the error
# cases in cmake.cxx...
#
# (These are not good examples of cmake command lines. Do not copy and
# paste them elsewhere and expect them to work... See the cmake
# documentation or other real examples of usage instead.)
#
RunCMakeWithArgs()
RunCMakeWithArgs(-C)
RunCMakeWithArgs(-C nosuchcachefile.txt)
RunCMakeWithArgs(--check-stamp-file nostampfile)
RunCMakeWithArgs(--check-stamp-list nostamplist)
RunCMakeWithArgs(nosuchsubdir/CMakeCache.txt)
RunCMakeWithArgs(nosuchsubdir/CMakeLists.txt)
RunCMakeWithArgs(-D)
RunCMakeWithArgs(--debug-output .)
RunCMakeWithArgs(--debug-trycompile .)
RunCMakeWithArgs(-E)
RunCMakeWithArgs(-E create_symlink)
RunCMakeWithArgs(-E echo_append)
RunCMakeWithArgs(-E rename)
RunCMakeWithArgs(-E touch_nocreate)
RunCMakeWithArgs(-G)
RunCMakeWithArgs(--graphviz= ../Source)
RunCMakeWithArgs(--graphviz=g.dot .)
RunCMakeWithArgs(-P)
RunCMakeWithArgs(-P nosuchscriptfile.cmake)
RunCMakeWithArgs(--trace .)
RunCMakeWithArgs(-U)
RunCMakeWithArgs(-U nosuchvariable .)
RunCMakeWithArgs(-V)
RunCMakeWithArgs(-V .)
RunCMakeWithArgs(-Wno-dev .)
RunCMakeWithArgs(-Wdev .)
