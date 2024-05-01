string(REGEX REPLACE "\r\n" "\n" expect "${expect}")
string(REGEX REPLACE "\n+$" "" expect "${expect}")

file(READ "${RunCMake_TEST_BINARY_DIR}/out.txt" actual)
string(REGEX REPLACE "\r\n" "\n" actual "${actual}")
string(REGEX REPLACE "\n+$" "" actual "${actual}")

if(NOT actual MATCHES "^${expect}$")
  string(REPLACE "\n" "\n expect> " expect " expect> ${expect}")
  string(REPLACE "\n" "\n actual> " actual " actual> ${actual}")
  message(FATAL_ERROR "Expected file(GENERATE) output:\n${expect}\ndoes not match actual output:\n${actual}")
endif()
