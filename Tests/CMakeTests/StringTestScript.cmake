message(STATUS "testname='${testname}'")

if(testname STREQUAL empty) # fail
  string()

elseif(testname STREQUAL bogus) # fail
  string(BOGUS)

elseif(testname STREQUAL random) # pass
  string(RANDOM r)
  message(STATUS "r='${r}'")

elseif(testname STREQUAL toupper_no_variable) # fail
  string(TOUPPER)

elseif(testname STREQUAL ascii_no_variable) # fail
  string(ASCII)

elseif(testname STREQUAL ascii_code_too_small) # fail
  string(ASCII -1 bummer)

elseif(testname STREQUAL ascii_code_too_large) # fail
  string(ASCII 288 bummer)

elseif(testname STREQUAL configure_no_input) # fail
  string(CONFIGURE)

elseif(testname STREQUAL configure_no_variable) # fail
  string(CONFIGURE "this is @testname@")

elseif(testname STREQUAL configure_escape_quotes) # pass
  string(CONFIGURE "this is @testname@" v ESCAPE_QUOTES)
  message(STATUS "v='${v}'")

elseif(testname STREQUAL configure_bogus) # fail
  string(CONFIGURE "this is @testname@" v ESCAPE_QUOTES BOGUS)

elseif(testname STREQUAL regex_no_mode) # fail
  string(REGEX)

elseif(testname STREQUAL regex_match_not_enough_args) # fail
  string(REGEX MATCH)

elseif(testname STREQUAL regex_matchall_not_enough_args) # fail
  string(REGEX MATCHALL)

elseif(testname STREQUAL regex_replace_not_enough_args) # fail
  string(REGEX REPLACE)

elseif(testname STREQUAL regex_bogus_mode) # fail
  string(REGEX BOGUS)

elseif(testname STREQUAL regex_match_multiple_inputs) # pass
  string(REGEX MATCH ".*" v input1 input2 input3 input4)
  message(STATUS "v='${v}'")

elseif(testname STREQUAL regex_match_bad_regex) # fail
  string(REGEX MATCH "(.*" v input)

elseif(testname STREQUAL regex_match_empty_string) # fail
  string(REGEX MATCH "x*" v "")

elseif(testname STREQUAL regex_match_no_match) # pass
  string(REGEX MATCH "xyz" v "abc")
  message(STATUS "v='${v}'")

elseif(testname STREQUAL regex_matchall_multiple_inputs) # pass
  string(REGEX MATCHALL "input" v input1 input2 input3 input4)
  message(STATUS "v='${v}'")

elseif(testname STREQUAL regex_matchall_bad_regex) # fail
  string(REGEX MATCHALL "(.*" v input)

elseif(testname STREQUAL regex_matchall_empty_string) # fail
  string(REGEX MATCHALL "x*" v "")

elseif(testname STREQUAL regex_replace_ends_with_backslash) # fail
  string(REGEX REPLACE "input" "output\\" v input1 input2 input3 input4)

elseif(testname STREQUAL regex_replace_ends_with_escaped_backslash) # pass
  string(REGEX REPLACE "input" "output\\\\" v input1 input2 input3 input4)
  message(STATUS "v='${v}'")

elseif(testname STREQUAL regex_replace_has_linefeed) # pass
  string(REGEX REPLACE "input" "output\\n" v input1 input2 input3 input4)
  message(STATUS "v='${v}'")

elseif(testname STREQUAL regex_replace_has_bogus_escape) # fail
  string(REGEX REPLACE "input" "output\\a" v input1 input2 input3 input4)

elseif(testname STREQUAL regex_replace_bad_regex) # fail
  string(REGEX REPLACE "this (.*" "with that" v input)

elseif(testname STREQUAL regex_replace_empty_string) # fail
  string(REGEX REPLACE "x*" "that" v "")

elseif(testname STREQUAL regex_replace_index_too_small) # fail
  string(REGEX REPLACE "^this (.*)$" "with \\1 \\-1" v "this input")

elseif(testname STREQUAL regex_replace_index_too_large) # fail
  string(REGEX REPLACE "^this (.*)$" "with \\1 \\2" v "this input")

elseif(testname STREQUAL compare_no_mode) # fail
  string(COMPARE)

elseif(testname STREQUAL compare_bogus_mode) # fail
  string(COMPARE BOGUS)

elseif(testname STREQUAL compare_not_enough_args) # fail
  string(COMPARE EQUAL)

elseif(testname STREQUAL replace_not_enough_args) # fail
  string(REPLACE)

elseif(testname STREQUAL replace_multiple_inputs) # pass
  string(REPLACE "input" "output" v input1 input2 input3 input4)
  message(STATUS "v='${v}'")

elseif(testname STREQUAL substring_not_enough_args) # fail
  string(SUBSTRING)

elseif(testname STREQUAL substring_begin_too_large) # fail
  string(SUBSTRING "abcdefg" 25 100 v)

elseif(testname STREQUAL substring_end_too_large) # fail
  string(SUBSTRING "abcdefg" 1 100 v)

elseif(testname STREQUAL substring_begin_less_than_zero) # fail
  string(SUBSTRING "abcdefg" -2 4 v)

elseif(testname STREQUAL substring_end_less_than_begin) # fail
  string(SUBSTRING "abcdefg" 6 3 v)

elseif(testname STREQUAL length_not_enough_args) # fail
  string(LENGTH)

elseif(testname STREQUAL strip_not_enough_args) # fail
  string(STRIP)

elseif(testname STREQUAL random_not_enough_args) # fail
  string(RANDOM)

elseif(testname STREQUAL random_3_args) # fail
  string(RANDOM LENGTH 9)

elseif(testname STREQUAL random_5_args) # fail
  string(RANDOM LENGTH 9 ALPHABET "aceimnorsuvwxz")

elseif(testname STREQUAL random_with_length) # pass
  string(RANDOM LENGTH 9 v)
  message(STATUS "v='${v}'")

elseif(testname STREQUAL random_with_alphabet) # pass
  string(RANDOM ALPHABET "aceimnorsuvwxz" v)
  message(STATUS "v='${v}'")

elseif(testname STREQUAL random_bad_length) # fail
  string(RANDOM LENGTH 0 v)

elseif(testname STREQUAL random_empty_alphabet) # pass
  string(RANDOM ALPHABET "" v)
  message(STATUS "v='${v}'")

elseif(testname STREQUAL random_with_length_and_alphabet) # pass
  string(RANDOM LENGTH 9 ALPHABET "aceimnorsuvwxz" v)
  message(STATUS "v='${v}'")

elseif(testname STREQUAL random_with_various_alphabets) # pass
  # small alphabet
  string(RANDOM LENGTH 32 ALPHABET "ACGT" v)
  message(STATUS "v='${v}'")

  # smaller alphabet
  string(RANDOM LENGTH 32 ALPHABET "AB" v)
  message(STATUS "v='${v}'")

  # smallest alphabet
  string(RANDOM LENGTH 32 ALPHABET "Z" v)
  message(STATUS "v='${v}'")

  # smallest length and alphabet
  string(RANDOM LENGTH 1 ALPHABET "Q" v)
  message(STATUS "v='${v}'")

  # seed values -- 2 same, then 1 different
  string(RANDOM LENGTH 32 ALPHABET "ACGT" RANDOM_SEED 987654 v)
  message(STATUS "v='${v}'")
  string(RANDOM LENGTH 32 ALPHABET "ACGT" RANDOM_SEED 987654 v)
  message(STATUS "v='${v}'")
  string(RANDOM LENGTH 32 ALPHABET "ACGT" RANDOM_SEED 876543 v)
  message(STATUS "v='${v}'")

  # alphabet of many colors - use all the crazy keyboard characters
  string(RANDOM LENGTH 78 ALPHABET "~`!@#$%^&*()_-+={}[]\\|:\\;'\",.<>/?" v)
  message(STATUS "v='${v}'")

else() # fail
  message(FATAL_ERROR "testname='${testname}' - error: no such test in '${CMAKE_CURRENT_LIST_FILE}'")

endif()
