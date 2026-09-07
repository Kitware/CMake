cmake_policy(SET CMP0223 OLD)

set(prefix "")
cmake_path(IS_PREFIX prefix "/a/b" output)
if(NOT output)
  message(SEND_ERROR "empty prefix is not a prefix of '/a/b' under OLD")
endif()

cmake_path(IS_PREFIX prefix "" output)
if(NOT output)
  message(SEND_ERROR "empty prefix is not a prefix of the empty path under OLD")
endif()
