# Assertion macro
macro(check desc actual expect)
  if(NOT "x${actual}" STREQUAL "x${expect}")
    message(SEND_ERROR "${desc}: got \"${actual}\", not \"${expect}\"")
  endif()
endmacro()

# General test of all component types given an absolute path.
set(filename "/path/to/filename.ext.in")
set(expect_DIRECTORY "/path/to")
set(expect_NAME "filename.ext.in")
set(expect_EXT ".ext.in")
set(expect_NAME_WE "filename")
set(expect_PATH "/path/to")
foreach(c DIRECTORY NAME EXT NAME_WE PATH)
  get_filename_component(actual_${c} "${filename}" ${c})
  check("${c}" "${actual_${c}}" "${expect_${c}}")
  list(APPEND non_cache_vars actual_${c})
endforeach()

# Test Windows paths with DIRECTORY component and an absolute Windows path.
get_filename_component(test_slashes "c:\\path\\to\\filename.ext.in" DIRECTORY)
check("DIRECTORY from backslashes" "${test_slashes}" "c:/path/to")
list(APPEND non_cache_vars test_slashes)

get_filename_component(test_winroot "c:\\filename.ext.in" DIRECTORY)
check("DIRECTORY in windows root" "${test_winroot}" "c:/")
list(APPEND non_cache_vars test_winroot)

# Test finding absolute paths.
get_filename_component(test_absolute "/path/to/a/../filename.ext.in" ABSOLUTE)
check("ABSOLUTE" "${test_absolute}" "/path/to/filename.ext.in")

get_filename_component(test_absolute "/../path/to/filename.ext.in" ABSOLUTE)
check("ABSOLUTE .. in root" "${test_absolute}" "/path/to/filename.ext.in")
get_filename_component(test_absolute "c:/../path/to/filename.ext.in" ABSOLUTE)
check("ABSOLUTE .. in windows root" "${test_absolute}" "c:/path/to/filename.ext.in")

list(APPEND non_cache_vars test_absolute)

# Test the PROGRAM component type.
get_filename_component(test_program_name "/ arg1 arg2" PROGRAM)
check("PROGRAM with no args output" "${test_program_name}" "/")

get_filename_component(test_program_name "/ arg1 arg2" PROGRAM
  PROGRAM_ARGS test_program_args)
check("PROGRAM with args output: name" "${test_program_name}" "/")
check("PROGRAM with args output: args" "${test_program_args}" " arg1 arg2")

list(APPEND non_cache_vars test_program_name)
list(APPEND non_cache_vars test_program_args)

# Test CACHE parameter for most component types.
get_filename_component(test_cache "/path/to/filename.ext.in" DIRECTORY CACHE)
check("CACHE 1" "${test_cache}" "/path/to")
# Make sure that the existing CACHE entry from previous is honored:
get_filename_component(test_cache "/path/to/other/filename.ext.in" DIRECTORY CACHE)
check("CACHE 2" "${test_cache}" "/path/to")
unset(test_cache CACHE)
get_filename_component(test_cache "/path/to/other/filename.ext.in" DIRECTORY CACHE)
check("CACHE 3" "${test_cache}" "/path/to/other")

list(APPEND cache_vars test_cache)

# Test that ONLY the expected cache variables were created.
get_cmake_property(current_cache_vars CACHE_VARIABLES)
get_cmake_property(current_vars VARIABLES)

foreach(thisVar ${cache_vars})
  if(NOT thisVar IN_LIST current_cache_vars)
    message(SEND_ERROR "${thisVar} expected in cache but was not found.")
  endif()
endforeach()

foreach(thisVar ${non_cache_vars})
  if(thisVar IN_LIST current_cache_vars)
    message(SEND_ERROR "${thisVar} unexpectedly found in cache.")
  endif()
  if(NOT thisVar IN_LIST current_vars)
    # Catch likely typo when appending to non_cache_vars:
    message(SEND_ERROR "${thisVar} not found in regular variable list.")
  endif()
endforeach()
