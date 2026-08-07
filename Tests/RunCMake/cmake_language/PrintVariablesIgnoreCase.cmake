set(my_lower "abc")
set(MY_UPPER "ABC")

# Case-sensitive NAME_REGEX: only my_lower matches.
cmake_language(PRINT_VARIABLES ALL NAME_REGEX "^my_")

# IGNORE_CASE NAME_REGEX: both names lower-case to start with "my_".
cmake_language(PRINT_VARIABLES ALL NAME_REGEX "^my_" IGNORE_CASE)

# Case-sensitive VALUE_REGEX: only my_lower's value matches.
cmake_language(PRINT_VARIABLES ALL VALUE_REGEX "^abc")

# IGNORE_CASE VALUE_REGEX: both values lower-case to "abc".
cmake_language(PRINT_VARIABLES ALL VALUE_REGEX "^abc" IGNORE_CASE)
