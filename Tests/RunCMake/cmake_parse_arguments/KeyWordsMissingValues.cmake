include(${CMAKE_CURRENT_LIST_DIR}/test_utils.cmake)

# No keywords that miss any values, _KEYWORDS_MISSING_VALUES should not be defined
cmake_parse_arguments(PREF "" "P1" "P2" P1 p1 P2 p2_a p2_b)

TEST(PREF_KEYWORDS_MISSING_VALUES "UNDEFINED")

# Keyword should even be deleted from the actual scope
set(PREF_KEYWORDS_MISSING_VALUES "What ever")
cmake_parse_arguments(PREF "" "" "")

TEST(PREF_KEYWORDS_MISSING_VALUES "UNDEFINED")

# Given missing keywords as only option
cmake_parse_arguments(PREF "" "P1" "P2" P1)

TEST(PREF_KEYWORDS_MISSING_VALUES "P1")
TEST(PREF_P1 "UNDEFINED")
TEST(PREF_UNPARSED_ARGUMENTS "UNDEFINED")

# Mixed with unparsed arguments
cmake_parse_arguments(UPREF "" "P1" "P2" A B P2 C P1)
TEST(UPREF_KEYWORDS_MISSING_VALUES "P1")
TEST(UPREF_UNPARSED_ARGUMENTS A B)

# one_value_keyword followed by option
cmake_parse_arguments(REF "OP" "P1" "" P1 OP)
TEST(REF_KEYWORDS_MISSING_VALUES "P1")
TEST(REF_UNPARSED_ARGUMENTS "UNDEFINED")
TEST(REF_OP "TRUE")

# Counter Test
cmake_parse_arguments(REF "OP" "P1" "" P1 p1 OP)
TEST(REF_KEYWORDS_MISSING_VALUES "UNDEFINED")
TEST(REF_P1 "p1")
TEST(REF_UNPARSED_ARGUMENTS "UNDEFINED")
TEST(REF_OP "TRUE")

# one_value_keyword followed by a one_value_keyword
cmake_parse_arguments(REF "" "P1;P2" "" P1 P2 p2)
TEST(REF_KEYWORDS_MISSING_VALUES "P1")
TEST(REF_P1 "UNDEFINED")
TEST(REF_UNPARSED_ARGUMENTS "UNDEFINED")
TEST(REF_P2 "p2")

# Counter Test
cmake_parse_arguments(REF "" "P1;P2" "" P1 p1 P2 p2)
TEST(REF_KEYWORDS_MISSING_VALUES "UNDEFINED")
TEST(REF_P1 "p1")
TEST(REF_UNPARSED_ARGUMENTS "UNDEFINED")
TEST(REF_P2 "p2")

# one_value_keyword followed by a multi_value_keywords
cmake_parse_arguments(REF "" "P1" "P2" P1 P2 p1 p2)
TEST(REF_KEYWORDS_MISSING_VALUES "P1")
TEST(REF_P1 "UNDEFINED")
TEST(REF_UNPARSED_ARGUMENTS "UNDEFINED")
TEST(REF_P2 p1 p2)

# Counter Examples
cmake_parse_arguments(REF "" "P1" "P2" P1 p1 P2 p1 p2)
TEST(REF_KEYWORDS_MISSING_VALUES "UNDEFINED")
TEST(REF_P1 "p1")
TEST(REF_UNPARSED_ARGUMENTS "UNDEFINED")
TEST(REF_P2 p1 p2)

# multi_value_keywords as only option
cmake_parse_arguments(REF "" "P1" "P2" P2)
TEST(REF_KEYWORDS_MISSING_VALUES "P2")
TEST(REF_P1 "UNDEFINED")
TEST(REF_UNPARSED_ARGUMENTS "UNDEFINED")
TEST(REF_P2 "UNDEFINED")

# multi_value_keywords followed by option
cmake_parse_arguments(REF "O1" "" "P1" P1 O1)
TEST(REF_KEYWORDS_MISSING_VALUES "P1")
TEST(REF_P1 "UNDEFINED")
TEST(REF_UNPARSED_ARGUMENTS "UNDEFINED")
TEST(REF_O1 "TRUE")

# counter test
cmake_parse_arguments(REF "O1" "" "P1" P1 p1 p2 O1)
TEST(REF_KEYWORDS_MISSING_VALUES "UNDEFINED")
TEST(REF_P1 "p1;p2")
TEST(REF_UNPARSED_ARGUMENTS "UNDEFINED")
TEST(REF_O1 "TRUE")

# multi_value_keywords followed by one_value_keyword
cmake_parse_arguments(REF "" "P1" "P2" P2 P1 p1)
TEST(REF_KEYWORDS_MISSING_VALUES "P2")
TEST(REF_P1 "p1")
TEST(REF_UNPARSED_ARGUMENTS "UNDEFINED")
TEST(REF_P2 "UNDEFINED")

# counter test
cmake_parse_arguments(REF "" "P1" "P2" P2 p2 P1 p1)
TEST(REF_KEYWORDS_MISSING_VALUES "UNDEFINED")
TEST(REF_P1 "p1")
TEST(REF_UNPARSED_ARGUMENTS "UNDEFINED")
TEST(REF_P2 "p2")

# one_value_keyword as last argument
cmake_parse_arguments(REF "" "P1" "P2" A P2 p2 P1)
TEST(REF_KEYWORDS_MISSING_VALUES "P1")
TEST(REF_P1 "UNDEFINED")
TEST(REF_UNPARSED_ARGUMENTS "A")
TEST(REF_P2 "p2")

# multi_value_keywords as last argument
cmake_parse_arguments(REF "" "P1" "P2" P1 p1 P2)
TEST(REF_KEYWORDS_MISSING_VALUES "P2")
TEST(REF_P1 "p1")
TEST(REF_P2 "UNDEFINED")

# Multiple one_value_keyword and multi_value_keywords at different places
cmake_parse_arguments(REF "O1;O2" "P1" "P2" P1 O1 P2 O2)
TEST(REF_KEYWORDS_MISSING_VALUES P1 P2)
TEST(REF_P1 "UNDEFINED")
TEST(REF_P2 "UNDEFINED")

# Duplicated missing keywords
cmake_parse_arguments(REF "O1;O2" "P1" "P2" P1 O1 P2 O2 P1 P2)
TEST(REF_KEYWORDS_MISSING_VALUES P1 P2)
TEST(REF_P1 "UNDEFINED")
TEST(REF_P2 "UNDEFINED")

# make sure keywords that are never used, don't get added to KEYWORDS_MISSING_VALUES
cmake_parse_arguments(REF "O1;O2" "P1" "P2")
TEST(REF_KEYWORDS_MISSING_VALUES "UNDEFINED")
TEST(REF_O1 FALSE)
TEST(REF_O2 FALSE)
TEST(REF_P1 UNDEFINED)
TEST(REF_P2 UNDEFINED)
