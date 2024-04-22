# FIXME: TARGET_PROPERTY evaluation does not pierce LINK_ONLY
set(expect [[
# file\(GENERATE\) produced:
main LINK_LIBRARIES: 'foo1' # not transitive
main LINK_DIRECTORIES: '[^';]*/Tests/RunCMake/GenEx-TARGET_PROPERTY/dirM;[^';]*/Tests/RunCMake/GenEx-TARGET_PROPERTY/dir1'
main LINK_OPTIONS: '-optM;-opt1'
main LINK_DEPENDS: '[^';]*/Tests/RunCMake/GenEx-TARGET_PROPERTY/TransitiveLink-build/depM;[^';]*/Tests/RunCMake/GenEx-TARGET_PROPERTY/TransitiveLink-build/dep1'
]])

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
