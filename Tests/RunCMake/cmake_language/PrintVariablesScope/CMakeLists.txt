set(child_local "c_val")

# Inside the subdir: parent's regular variable and the cache entry are
# both visible.  `child_local` is too because we just defined it here.
cmake_language(PRINT_VARIABLES ALL
  NAME_REGEX "^(parent_|child_)")
