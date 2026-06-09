set(parent_regular "p_val")
set(parent_cache "p_cache_val" CACHE STRING "")

add_subdirectory(PrintVariablesScope)

# Back in the parent scope: the child set `child_local` as a regular
# variable; it must NOT be visible here.
cmake_language(PRINT_VARIABLES ALL
  NAME_REGEX "^(parent_|child_)")
