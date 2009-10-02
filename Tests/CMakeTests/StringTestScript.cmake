message(STATUS "testname='${testname}'")


if(testname STREQUAL empty)
  string()

elseif(testname STREQUAL bogus)
  string(BOGUS)

elseif(testname STREQUAL random)
  string(RANDOM r)
  message(STATUS "r='${r}'")

elseif(testname STREQUAL toupper_no_variable)
  string(TOUPPER)

elseif(testname STREQUAL ascii_no_variable)
  string(ASCII)

elseif(testname STREQUAL ascii_bad_code)
  string(ASCII 288 bummer)

elseif(testname STREQUAL configure_no_input)
  string(CONFIGURE)

elseif(testname STREQUAL configure_no_variable)
  string(CONFIGURE "this is @testname@")

elseif(testname STREQUAL configure_escape_quotes)
  string(CONFIGURE "this is @testname@" v ESCAPE_QUOTES)
  message(STATUS "v='${v}'")

elseif(testname STREQUAL configure_bogus)
  string(CONFIGURE "this is @testname@" v ESCAPE_QUOTES BOGUS)
  message(STATUS "v='${v}'")

elseif(testname STREQUAL regex_no_mode)
  string(REGEX)

elseif(testname STREQUAL regex_match_not_enough_args)
  string(REGEX MATCH)

elseif(testname STREQUAL regex_matchall_not_enough_args)
  string(REGEX MATCHALL)

elseif(testname STREQUAL regex_replace_not_enough_args)
  string(REGEX REPLACE)

elseif(testname STREQUAL regex_bogus_mode)
  string(REGEX BOGUS)

elseif(testname STREQUAL regex_match_multiple_inputs)
  string(REGEX MATCH ".*" v input1 input2 input3 input4)
  message(STATUS "v='${v}'")

elseif(testname STREQUAL regex_match_bad_regex)
  string(REGEX MATCH "(.*" v input)

elseif(testname STREQUAL regex_match_empty_string)
  string(REGEX MATCH "x*" v "")

elseif(testname STREQUAL regex_matchall_multiple_inputs)
  string(REGEX MATCHALL "input" v input1 input2 input3 input4)
  message(STATUS "v='${v}'")

elseif(testname STREQUAL regex_matchall_bad_regex)
  string(REGEX MATCHALL "(.*" v input)

elseif(testname STREQUAL regex_matchall_empty_string)
  string(REGEX MATCHALL "x*" v "")

elseif(testname STREQUAL regex_replace_ends_with_backslash)
  string(REGEX REPLACE "input" "output\\" v input1 input2 input3 input4)

elseif(testname STREQUAL regex_replace_ends_with_escaped_backslash)
  string(REGEX REPLACE "input" "output\\\\" v input1 input2 input3 input4)
  message(STATUS "v='${v}'")

elseif(testname STREQUAL regex_replace_has_linefeed)
  string(REGEX REPLACE "input" "output\\n" v input1 input2 input3 input4)
  message(STATUS "v='${v}'")

elseif(testname STREQUAL regex_replace_has_bogus_escape)
  string(REGEX REPLACE "input" "output\\a" v input1 input2 input3 input4)

elseif(testname STREQUAL regex_replace_bad_regex)
  string(REGEX REPLACE "this (.*" "with that" v input)

elseif(testname STREQUAL regex_replace_empty_string)
  string(REGEX REPLACE "x*" "that" v "")

elseif(testname STREQUAL regex_replace_out_of_range)
  string(REGEX REPLACE "^this (.*)$" "with \\1 \\2" v "this input")

elseif(testname STREQUAL compare_no_mode)
  string(COMPARE)

elseif(testname STREQUAL compare_bogus_mode)
  string(COMPARE BOGUS)

elseif(testname STREQUAL compare_not_enough_args)
  string(COMPARE EQUAL)

elseif(testname STREQUAL replace_not_enough_args)
  string(REPLACE)

elseif(testname STREQUAL replace_multiple_inputs)
  string(REPLACE "input" "output" v input1 input2 input3 input4)
  message(STATUS "v='${v}'")

elseif(testname STREQUAL substring_not_enough_args)
  string(SUBSTRING)

elseif(testname STREQUAL substring_bad_begin)
  string(SUBSTRING "abcdefg" 25 100 v)
  message(STATUS "v='${v}'")

elseif(testname STREQUAL substring_bad_end)
  string(SUBSTRING "abcdefg" 1 100 v)
  message(STATUS "v='${v}'")

elseif(testname STREQUAL length_not_enough_args)
  string(LENGTH)

elseif(testname STREQUAL strip_not_enough_args)
  string(STRIP)

else()
  message(FATAL_ERROR "testname='${testname}' - error: no such test in '${CMAKE_CURRENT_LIST_FILE}'")

endif()
