include(${CMAKE_CURRENT_LIST_DIR}/test_utils.cmake)

function(test1)
  cmake_parse_arguments(PARSE_ARGV 0
    mpref "" "" "MULTI")

  TEST(mpref_MULTI foo "foo\;bar")

  cmake_parse_arguments(PARSE_ARGV 1
    upref "" "" "MULTI")

  TEST(upref_UNPARSED_ARGUMENTS foo "foo\;bar")
endfunction()
test1(MULTI foo "foo\;bar")

function(test2)
  cmake_parse_arguments(PARSE_ARGV 0
    mpref "" "" "MULTI")

  TEST(mpref_MULTI "foo;" "bar;")

  cmake_parse_arguments(PARSE_ARGV 1
    upref "" "" "MULTI")

  TEST(upref_UNPARSED_ARGUMENTS "foo;" "bar;")
endfunction()
test2(MULTI "foo;" "bar;")

function(test3)
  cmake_parse_arguments(PARSE_ARGV 0
    mpref "" "" "MULTI")

  TEST(mpref_MULTI "[foo;]" "bar\\")

  cmake_parse_arguments(PARSE_ARGV 1
    upref "" "" "MULTI")

  TEST(upref_UNPARSED_ARGUMENTS "[foo;]" "bar\\")
endfunction()
test3(MULTI "[foo;]" "bar\\")

function(test4)
  cmake_parse_arguments(PARSE_ARGV 0
    mpref "" "" "MULTI")

  TEST(mpref_MULTI foo "bar;none")

  cmake_parse_arguments(PARSE_ARGV 1
    upref "" "" "MULTI")

  TEST(upref_UNPARSED_ARGUMENTS foo "bar;none")
endfunction()
test4(MULTI foo bar\\ none)

# Single-value keyword with empty string as value
message(NOTICE "Testing CMP0174 = NEW")
block(SCOPE_FOR POLICIES)
  cmake_policy(SET CMP0174 NEW)
  function(test_cmp0174_new_missing)
    cmake_parse_arguments(PARSE_ARGV 0 arg "" "P1;P2" "")
    TEST(arg_KEYWORDS_MISSING_VALUES "P2")
    TEST(arg_P1 "")
    TEST(arg_P2 "")
  endfunction()
  test_cmp0174_new_missing(P1 "" P2)

  function(test_cmp0174_new_no_missing)
    cmake_parse_arguments(PARSE_ARGV 0 arg "" "P1;P2" "")
    TEST(arg_KEYWORDS_MISSING_VALUES "UNDEFINED")
    TEST(arg_P1 "")
    TEST(arg_P2 "UNDEFINED")
  endfunction()
  test_cmp0174_new_no_missing(P1 "")

  # For repeated keywords, the keyword will be reported as missing a value if
  # any one of them is missing a value.
  function(test_cmp0174_new_repeated_arg)
    cmake_parse_arguments(PARSE_ARGV 0 arg "" "P1;P2" "")
    TEST(arg_KEYWORDS_MISSING_VALUES "P1")
    TEST(arg_P1 "abc")
    TEST(arg_P2 "UNDEFINED")
  endfunction()
  test_cmp0174_new_repeated_arg(P1 P1 abc)
endblock()

message(NOTICE "Testing CMP0174 = OLD")
block(SCOPE_FOR POLICIES)
  cmake_policy(SET CMP0174 OLD)
  function(test_cmp0174_old)
    cmake_parse_arguments(PARSE_ARGV 0 arg "" "P1;P2;P3" "")
    TEST(arg_KEYWORDS_MISSING_VALUES "P2")
    TEST(arg_P1 "UNDEFINED")
    TEST(arg_P2 "UNDEFINED")
    TEST(arg_P3 "UNDEFINED")
  endfunction()
  test_cmp0174_old(P1 "" P2)

  function(test_cmp0174_old_repeated_arg)
    cmake_parse_arguments(PARSE_ARGV 0 arg "" "P1;P2" "")
    TEST(arg_KEYWORDS_MISSING_VALUES "P1")
    TEST(arg_P1 "abc")
    TEST(arg_P2 "UNDEFINED")
  endfunction()
  test_cmp0174_old_repeated_arg(P1 P1 abc)
endblock()

message(NOTICE "Testing CMP0174 = WARN")
block(SCOPE_FOR POLICIES)
  cmake_policy(VERSION 3.30)  # WARN
  function(test_cmp0174_warn)
    cmake_parse_arguments(PARSE_ARGV 0 arg "" "P1;P2;P3" "")
    TEST(arg_KEYWORDS_MISSING_VALUES "P2")
    TEST(arg_P1 "UNDEFINED")
    TEST(arg_P2 "UNDEFINED")
    TEST(arg_P3 "UNDEFINED")
  endfunction()
  test_cmp0174_warn(P1 "" P2)

  function(test_cmp0174_warn_repeated_arg)
    cmake_parse_arguments(PARSE_ARGV 0 arg "" "P1;P2" "")
    TEST(arg_KEYWORDS_MISSING_VALUES "P1")
    TEST(arg_P1 "abc")
    TEST(arg_P2 "UNDEFINED")
  endfunction()
  test_cmp0174_warn_repeated_arg(P1 P1 abc)
endblock()
