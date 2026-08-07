# `typed_cache` exercises the `CACHE{name}:TYPE` annotation.
set(typed_cache "/filepath/in/cache" CACHE FILEPATH "")

# `uninit_cache` is injected via -D on the cmake command line in the
# harness (see RunCMakeTest.cmake), which produces an UNINITIALIZED cache
# entry.  We assert that the `:TYPE` suffix is suppressed for those.

# `shadowed` exists both as a regular variable AND a cache entry; we
# expect two distinct lines
set(shadowed "cache_val" CACHE STRING "" FORCE)
set(shadowed "regular_val")

cmake_language(PRINT_VARIABLES ALL
  NAME_REGEX "^(typed_cache|uninit_cache|shadowed)$"
)
