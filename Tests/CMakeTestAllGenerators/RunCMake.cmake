if(NOT DEFINED CMake_SOURCE_DIR)
  message(FATAL_ERROR "CMake_SOURCE_DIR not defined")
endif()

if(NOT DEFINED dir)
  message(FATAL_ERROR "dir not defined")
endif()

# Analyze 'cmake --help' output for list of available generators:
#
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${dir})
execute_process(COMMAND ${CMAKE_COMMAND} -E capabilities
  RESULT_VARIABLE result
  OUTPUT_VARIABLE stdout
  ERROR_VARIABLE stderr
  WORKING_DIRECTORY ${dir})

set(generators)
string(REGEX MATCHALL [["name":"[^"]+","platformSupport"]] generators_json "${stdout}")
foreach(gen_json IN LISTS generators_json)
  if("${gen_json}" MATCHES [["name":"([^"]+)"]])
    set(gen "${CMAKE_MATCH_1}")
    if(NOT gen MATCHES " (Win64|IA64|ARM)$")
      list(APPEND generators "${gen}")
    endif()
  endif()
endforeach()
list(REMOVE_DUPLICATES generators)

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

# If we'll be testing any of the MinGW Makefiles generators, adjust the
# ENV{PATH} to make sure libgmp-10.dll can be loaded as needed. But only if
# the testing machine has a default MinGW install... (If you have a
# non-default install, append to the PATH before running the test...)
#
if(generators MATCHES "MinGW Makefiles")
  if(EXISTS "C:/MinGW/bin/libgmp-10.dll")
    string(TOLOWER "$ENV{PATH}" path)
    if(NOT path MATCHES "/mingw/bin")
      if(UNIX)
        set(sep ":")
        set(mingw_bin "/mingw/bin")
      else()
        set(sep ";")
        set(mingw_bin "C:/MinGW/bin")
      endif()
      set(ENV{PATH} "$ENV{PATH}${sep}${mingw_bin}")
      message(STATUS "info: appending '${sep}${mingw_bin}' to the PATH")
    endif()
  endif()
endif()

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
