# Policy deliberately not set, so the warning fires and OLD behavior applies.
set(prefix "")
cmake_path(IS_PREFIX prefix "/a/b" output)
if(NOT output)
  message(SEND_ERROR "empty prefix is not a prefix of '/a/b' under WARN")
endif()
