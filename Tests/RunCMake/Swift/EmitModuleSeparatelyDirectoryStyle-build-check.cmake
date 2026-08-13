# Older Swift compilers may not set CMAKE_Swift_MODULE_TRIPLE
# Check that we built something before verifying the output
if(EXISTS "${RunCMake_TEST_BINARY_DIR}/no-swift-module-triple")
  return()
endif()

string(
  REGEX MATCHALL
  "swiftc(\\.exe)?\"? [^\n]* -emit-module-path [^\n]*L\\.swiftmodule[^ \n]*"
  swift_module_commands "${actual_stdout}")

set(emit_module_commands "${swift_module_commands}")
list(FILTER emit_module_commands EXCLUDE REGEX " -c ")
if(NOT emit_module_commands)
  string(APPEND RunCMake_TEST_FAILED
    "Expected an emit-module command with '-emit-module-path ... L.swiftmodule'.\n")
endif()

set(compile_commands "${swift_module_commands}")
list(FILTER compile_commands INCLUDE REGEX " -c ")
if(compile_commands)
  string(APPEND RunCMake_TEST_FAILED
    "Compile command (-c) should not contain '-emit-module-path' when emitting module separately.\n")
endif()

# CMP0195=NEW: the emit-module path should be directory-style
# (L.swiftmodule/<triple>.swiftmodule), not flat.
list(FILTER emit_module_commands INCLUDE REGEX "L\\.swiftmodule(/|\\\\)[-_a-zA-Z0-9]+\\.swiftmodule")
if(NOT emit_module_commands)
  string(APPEND RunCMake_TEST_FAILED
    "Expected directory-style -emit-module-path (L.swiftmodule/<triple>.swiftmodule) from CMP0195=NEW.\n")
endif()
