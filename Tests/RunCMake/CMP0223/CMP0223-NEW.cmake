cmake_policy(SET CMP0223 NEW)

set(prefix "")
cmake_path(IS_PREFIX prefix "/a/b" output)
if(output)
  message(SEND_ERROR "empty prefix is a prefix of '/a/b' under NEW")
endif()

cmake_path(IS_PREFIX prefix "" output)
if(output)
  message(SEND_ERROR "empty prefix is a prefix of the empty path under NEW")
endif()

# NORMALIZE takes the same path, because normalizing an empty path leaves
# it empty.
cmake_path(IS_PREFIX prefix "/a/b" NORMALIZE output)
if(output)
  message(SEND_ERROR "empty prefix is a prefix of '/a/b' under NEW, NORMALIZE")
endif()

# A non-empty prefix is unaffected.
set(prefix "/a")
cmake_path(IS_PREFIX prefix "/a/b" output)
if(NOT output)
  message(SEND_ERROR "'/a' is not a prefix of '/a/b' under NEW")
endif()
