# Older Swift compilers may not set CMAKE_Swift_MODULE_TRIPLE
# Check that we built something before verifying the output
if(EXISTS "${RunCMake_TEST_BINARY_DIR}/no-swift-module-triple")
  return()
endif()

if(NOT actual_stdout MATCHES
    "swiftc(\\.exe)?\"? [^\n]* @CMakeFiles(/|\\\\)L\\.dir(/|\\\\)(Debug(/|\\\\))?L\\.o(bj)?\\.swift\\.rsp")
  string(APPEND RunCMake_TEST_FAILED
    "No Swift compile response-file command found for target L.\n")
endif()

if(NOT actual_stdout MATCHES
    "swiftc(\\.exe)?\"? [^\n]* @[^\n]*L\\.swiftmodule(/|\\\\)[^\n]*\\.swiftmodule\\.rsp")
  string(APPEND RunCMake_TEST_FAILED
    "No Swift emit-module response-file command found for target L.\n")
endif()

# Response files are written by ninja at build time.
# Inspect the build.ninja file instead.
if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(path "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/impl-Debug.ninja")
else()
  set(path "${RunCMake_TEST_BINARY_DIR}/build.ninja")
endif()
file(READ "${path}" build_ninja)

# The emit-module edge for L: RSP_FILE set and -emit-module in its FLAGS.
string(REGEX MATCH
  "build [^\n]*L\\.swiftmodule(/|\\\\)[^\n:]*\\.swiftmodule:[^\n]*(\n [^\n]+)*"
  module_edge "${build_ninja}")
if(NOT module_edge)
  string(APPEND RunCMake_TEST_FAILED
    "Could not find emit-module edge for L.\n")
elseif(NOT module_edge MATCHES "\n  FLAGS = [^\n]* ?-emit-module ")
  string(APPEND RunCMake_TEST_FAILED
    "Emit-module edge for L is missing -emit-module in FLAGS.\nEdge:\n${module_edge}\n")
elseif(NOT module_edge MATCHES "\n  RSP_FILE = [^\n]*L\\.swiftmodule(/|\\\\)[^\n]*\\.swiftmodule\\.rsp")
  string(APPEND RunCMake_TEST_FAILED
    "Emit-module edge for L is missing a response file.\nEdge:\n${module_edge}\n")
endif()

# The object compile edge for L: RSP_FILE set and -c in its FLAGS.
string(REGEX MATCH
  "build [^\n]*L\\.swift\\.o(bj)?:[^\n]*(\n [^\n]+)*"
  object_edge "${build_ninja}")
if(NOT object_edge)
  string(APPEND RunCMake_TEST_FAILED
    "Could not find object compile edge for L.swift.\n")
elseif(NOT object_edge MATCHES "\n  FLAGS = [^\n]* ?-c ")
  string(APPEND RunCMake_TEST_FAILED
    "Object compile edge for L.swift is missing -c in FLAGS.\nEdge:\n${object_edge}\n")
elseif(NOT object_edge MATCHES "\n  RSP_FILE = [^\n]*L\\.o(bj)?\\.swift\\.rsp")
  string(APPEND RunCMake_TEST_FAILED
    "Object compile edge for L.swift is missing a response file.\nEdge:\n${object_edge}\n")
endif()
