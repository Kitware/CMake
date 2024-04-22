set(expect [[
# file\(GENERATE\) produced:
main INCLUDE_DIRECTORIES: '[^';]*/Tests/RunCMake/GenEx-TARGET_PROPERTY/dirM;[^';]*/Tests/RunCMake/GenEx-TARGET_PROPERTY/dir1'
main SYSTEM_INCLUDE_DIRECTORIES: '[^';]*/Tests/RunCMake/GenEx-TARGET_PROPERTY/sys1'
main COMPILE_DEFINITIONS: 'DEFM;DEF1'
main COMPILE_FEATURES: 'cxx_std_20;cxx_std_11'
main COMPILE_OPTIONS: '-optM;-opt1'
main PRECOMPILE_HEADERS: '[^';]*/Tests/RunCMake/GenEx-TARGET_PROPERTY/empty.h;[^';]*/Tests/RunCMake/GenEx-TARGET_PROPERTY/empty1.h'
main SOURCES: 'main.c;[^';]*/Tests/RunCMake/GenEx-TARGET_PROPERTY/empty1.c'
main AUTOMOC_MACRO_NAMES: 'MOCM;MOC1'
main AUTOUIC_OPTIONS: '-uicM;-uic1'
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
