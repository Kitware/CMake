cmake_policy(SET CMP0222 NEW)

# CMP0223 deliberately not set, so the warning fires and OLD behavior applies.
if(NOT "" PATH_IS_PREFIX "/a/b")
  message(SEND_ERROR "empty prefix is not a prefix of '/a/b' under WARN")
endif()
