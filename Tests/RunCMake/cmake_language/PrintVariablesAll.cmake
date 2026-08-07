set(MY_FOO "alpha")
set(MY_BAR "beta")
set(MY_BAZ "alpha-too")
set(OTHER_FOO "gamma")
set(OTHER_ALPHA "alpha-other")

# NAME_REGEX alone.
cmake_language(PRINT_VARIABLES ALL NAME_REGEX "^MY_")

# VALUE_REGEX alone.
cmake_language(PRINT_VARIABLES ALL VALUE_REGEX "^alpha")

# Both together.
cmake_language(PRINT_VARIABLES ALL
  NAME_REGEX "^MY_" VALUE_REGEX "^alpha")
