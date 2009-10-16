message(STATUS "testname='${testname}'")

if(testname STREQUAL empty) # fail
  file()

elseif(testname STREQUAL bogus) # fail
  file(BOGUS ffff)

elseif(testname STREQUAL different_not_enough_args) # fail
  file(DIFFERENT ffff)

elseif(testname STREQUAL download_not_enough_args) # fail
  file(DOWNLOAD ffff)

elseif(testname STREQUAL read_not_enough_args) # fail
  file(READ ffff)

elseif(testname STREQUAL rpath_check_not_enough_args) # fail
  file(RPATH_CHECK ffff)

elseif(testname STREQUAL rpath_remove_not_enough_args) # fail
  file(RPATH_REMOVE ffff)

elseif(testname STREQUAL strings_not_enough_args) # fail
  file(STRINGS ffff)

#elseif(testname STREQUAL to_native_path_not_enough_args) # fail
#  file(TO_NATIVE_PATH ffff)

elseif(testname STREQUAL read_with_offset) # pass
  file(READ ${CMAKE_CURRENT_LIST_FILE} v OFFSET 42 LIMIT 30)
  message("v='${v}'")

elseif(testname STREQUAL strings_bad_length_minimum) # fail
  file(STRINGS ${CMAKE_CURRENT_LIST_FILE} v LENGTH_MINIMUM bogus)

elseif(testname STREQUAL strings_bad_length_maximum) # fail
  file(STRINGS ${CMAKE_CURRENT_LIST_FILE} v LENGTH_MAXIMUM bogus)

elseif(testname STREQUAL strings_bad_limit_count) # fail
  file(STRINGS ${CMAKE_CURRENT_LIST_FILE} v LIMIT_COUNT bogus)

elseif(testname STREQUAL strings_bad_limit_input) # fail
  file(STRINGS ${CMAKE_CURRENT_LIST_FILE} v LIMIT_INPUT bogus)

elseif(testname STREQUAL strings_bad_limit_output) # fail
  file(STRINGS ${CMAKE_CURRENT_LIST_FILE} v LIMIT_OUTPUT bogus)

elseif(testname STREQUAL strings_bad_regex) # fail
  file(STRINGS ${CMAKE_CURRENT_LIST_FILE} v REGEX "(")

elseif(testname STREQUAL strings_unknown_arg) # fail
  file(STRINGS ${CMAKE_CURRENT_LIST_FILE} v BOGUS)

elseif(testname STREQUAL strings_bad_filename) # fail
  file(STRINGS ffff v LIMIT_COUNT 10)

elseif(testname STREQUAL strings_use_limit_count) # pass
  file(STRINGS ${CMAKE_CURRENT_LIST_FILE} v LIMIT_COUNT 10)
  message("v='${v}'")

elseif(testname STREQUAL strings_use_no_hex_conversion) # pass
  file(STRINGS ${CMAKE_CURRENT_LIST_FILE} v NO_HEX_CONVERSION)
  message("v='${v}'")

elseif(testname STREQUAL glob_recurse_follow_symlinks_no_expression) # fail
  file(GLOB_RECURSE v FOLLOW_SYMLINKS)

elseif(testname STREQUAL glob_recurse_relative_no_directory) # fail
  file(GLOB_RECURSE v RELATIVE)

elseif(testname STREQUAL glob_recurse_relative_no_expression) # fail
  file(GLOB_RECURSE v RELATIVE dddd)

elseif(testname STREQUAL glob_non_full_path) # pass
  file(GLOB_RECURSE v ffff*.*)
  message("v='${v}'")

elseif(testname STREQUAL different_no_variable) # fail
  file(DIFFERENT FILES)

elseif(testname STREQUAL different_no_files) # fail
  file(DIFFERENT v FILES)

elseif(testname STREQUAL different_different) # pass
  file(DIFFERENT v FILES ffffLHS ffffRHS)
  message("v='${v}'")

elseif(testname STREQUAL different_same) # pass
  file(DIFFERENT v FILES
    ${CMAKE_CURRENT_LIST_FILE} ${CMAKE_CURRENT_LIST_FILE})
  message("v='${v}'")

else() # fail
  message(FATAL_ERROR "testname='${testname}' - error: no such test in '${CMAKE_CURRENT_LIST_FILE}'")

endif()
