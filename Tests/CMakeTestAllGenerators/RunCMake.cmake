if(NOT DEFINED CMake_SOURCE_DIR)
  message(FATAL_ERROR "CMake_SOURCE_DIR not defined")
endif()

if(NOT DEFINED dir)
  message(FATAL_ERROR "dir not defined")
endif()

# Analyze 'cmake --help' output for list of available generators:
#
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${dir})
execute_process(COMMAND ${CMAKE_COMMAND} --help
  RESULT_VARIABLE result
  OUTPUT_VARIABLE stdout
  ERROR_VARIABLE stderr
  WORKING_DIRECTORY ${dir})

string(REPLACE ";" "\\;" stdout "${stdout}")
string(REPLACE "\n" "E;" stdout "${stdout}")

set(collecting 0)
set(generators)
foreach(eline ${stdout})
  string(REGEX REPLACE "^(.*)E$" "\\1" line "${eline}")
  if(collecting AND NOT line STREQUAL "")
    if(line MATCHES "=")
      string(REGEX REPLACE "^  (.+)= (.*)$" "\\1" gen "${line}")
      if(gen MATCHES "[A-Za-z]")
        string(REGEX REPLACE "^(.*[^ ]) +$" "\\1" gen "${gen}")
        if(gen)
          set(generators ${generators} ${gen})
        endif()
      endif()
    else()
      if(line MATCHES "^  [A-Za-z0-9]")
        string(REGEX REPLACE "^  (.+)$" "\\1" gen "${line}")
        string(REGEX REPLACE "^(.*[^ ]) +$" "\\1" gen "${gen}")
        if(gen)
          set(generators ${generators} ${gen})
        endif()
      endif()
    endif()
  endif()
  if(line STREQUAL "The following generators are available on this platform:")
    set(collecting 1)
  endif()
endforeach()

# Also call with one non-existent generator:
#
set(generators ${generators} "BOGUS_CMAKE_GENERATOR")

# Call cmake with -G on each available generator. We do not care if this
# succeeds or not. We expect it *not* to succeed if the underlying packaging
# tools are not installed on the system... This test is here simply to add
# coverage for the various cmake generators, even/especially to test ones
# where the tools are not installed.
#
message(STATUS "CTEST_FULL_OUTPUT (Avoid ctest truncation of output)")

message(STATUS "CMake generators='${generators}'")

# First setup a source tree to run CMake on.
#
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMake_SOURCE_DIR}/Tests/CTestTest/SmallAndFast
  ${dir}/Source
)

foreach(g ${generators})
  message(STATUS "cmake -G \"${g}\" ..")

  # Create a binary directory for each generator:
  #
  execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory
    ${dir}/Source/${g}
    )

  # Run cmake:
  #
  execute_process(COMMAND ${CMAKE_COMMAND} -G ${g} ..
    RESULT_VARIABLE result
    OUTPUT_VARIABLE stdout
    ERROR_VARIABLE stderr
    WORKING_DIRECTORY ${dir}/Source/${g}
    )

  message(STATUS "result='${result}'")
  message(STATUS "stdout='${stdout}'")
  message(STATUS "stderr='${stderr}'")
  message(STATUS "")
endforeach()

message(STATUS "CMake generators='${generators}'")
message(STATUS "CMAKE_COMMAND='${CMAKE_COMMAND}'")
