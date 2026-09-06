cmake_policy(SET CMP0222 NEW)

# Under NEW the keyword is interpreted as an operator and evaluated.  The
# operator's own behavior is covered by the RunCMake.if PathIsPrefix case,
# which outlives this directory: the OLD and WARN cases here are removed
# once support for the OLD behavior is dropped.
if(NOT "/a/b" PATH_IS_PREFIX "/a/b/c")
  message(SEND_ERROR "if(PATH_IS_PREFIX): '/a/b' not a prefix of '/a/b/c'")
endif()

if("/a/b" PATH_IS_PREFIX "/a/bc")
  message(SEND_ERROR "if(PATH_IS_PREFIX): '/a/b' wrongly a prefix of '/a/bc'")
endif()
