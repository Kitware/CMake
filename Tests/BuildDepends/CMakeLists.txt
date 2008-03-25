# this test creates a static library and an executable
# the source to the library is then changed 
# and the build is done on the executable and if things
# are working the executable should relink with the new
# value.  The subdir Project contains the CMakelists.txt
# and source files for the test project. 
cmake_minimum_required (VERSION 2.6)
project(BuildDepends)

# This entire test takes place during the initial
# configure step.  It should not run again when the
# project is built.
set(CMAKE_SUPPRESS_REGENERATION 1)

file(MAKE_DIRECTORY ${BuildDepends_BINARY_DIR}/Project)
message("Creating Project/foo.cxx")
write_file(${BuildDepends_BINARY_DIR}/Project/foo.cxx 
  "const char* foo() { return \"foo\";}" )

file(WRITE ${BuildDepends_BINARY_DIR}/Project/zot.hxx.in
  "static const char* zot = \"zot\";\n")
file(WRITE ${BuildDepends_BINARY_DIR}/Project/zot_custom.hxx.in
  "static const char* zot_custom = \"zot_custom\";\n")

message("Building project first time")
try_compile(RESULT 
  ${BuildDepends_BINARY_DIR}/Project
  ${BuildDepends_SOURCE_DIR}/Project
  testRebuild
  OUTPUT_VARIABLE OUTPUT)
if("${CMAKE_GENERATOR}" MATCHES "Xcode")
  try_compile(RESULT 
    ${BuildDepends_BINARY_DIR}/Project
    ${BuildDepends_SOURCE_DIR}/Project
    testRebuild
    OUTPUT_VARIABLE OUTPUT)
  try_compile(RESULT 
    ${BuildDepends_BINARY_DIR}/Project
    ${BuildDepends_SOURCE_DIR}/Project
    testRebuild
    OUTPUT_VARIABLE OUTPUT)
endif("${CMAKE_GENERATOR}" MATCHES "Xcode")

if(NOT RESULT)
  message(SEND_ERROR "Could not build test project: ${OUTPUT}")
endif(NOT RESULT)

set(bar ${BuildDepends_BINARY_DIR}/Project/bar${CMAKE_EXECUTABLE_SUFFIX})
if(EXISTS 
    "${BuildDepends_BINARY_DIR}/Project/Debug/bar${CMAKE_EXECUTABLE_SUFFIX}" )
  message("found debug")
  set(bar 
    "${BuildDepends_BINARY_DIR}/Project/Debug/bar${CMAKE_EXECUTABLE_SUFFIX}")
endif(EXISTS 
  "${BuildDepends_BINARY_DIR}/Project/Debug/bar${CMAKE_EXECUTABLE_SUFFIX}")
set(zot ${BuildDepends_BINARY_DIR}/Project/zot${CMAKE_EXECUTABLE_SUFFIX})
if(EXISTS 
    "${BuildDepends_BINARY_DIR}/Project/Debug/zot${CMAKE_EXECUTABLE_SUFFIX}" )
  message("found debug")
  set(zot 
    "${BuildDepends_BINARY_DIR}/Project/Debug/zot${CMAKE_EXECUTABLE_SUFFIX}")
endif(EXISTS 
  "${BuildDepends_BINARY_DIR}/Project/Debug/zot${CMAKE_EXECUTABLE_SUFFIX}")

message("Running ${bar}  ")
execute_process(COMMAND ${bar} OUTPUT_VARIABLE out RESULT_VARIABLE runResult)
string(REGEX REPLACE "[\r\n]" " " out "${out}")
message("Run result: ${runResult} Output: \"${out}\"")

if("${out}" STREQUAL "foo ")
  message("Worked!")
else("${out}" STREQUAL "foo ")
  message(SEND_ERROR "Project did not initially build properly: ${out}")
endif("${out}" STREQUAL "foo ")

message("Running ${zot}  ")
execute_process(COMMAND ${zot} OUTPUT_VARIABLE out RESULT_VARIABLE runResult)
string(REGEX REPLACE "[\r\n]" " " out "${out}")
message("Run result: ${runResult} Output: \"${out}\"")

if("${out}" STREQUAL "[zot] [zot_custom] ")
  message("Worked!")
else("${out}" STREQUAL "[zot] [zot_custom] ")
  message(SEND_ERROR "Project did not initially build properly: ${out}")
endif("${out}" STREQUAL "[zot] [zot_custom] ")

message("Waiting 3 seconds...")
# any additional argument will cause ${bar} to wait forever
execute_process(COMMAND ${bar} -infinite TIMEOUT 3 OUTPUT_VARIABLE out)

message("Modifying Project/foo.cxx")
write_file(${BuildDepends_BINARY_DIR}/Project/foo.cxx
  "const char* foo() { return \"foo changed\";}" )
file(WRITE ${BuildDepends_BINARY_DIR}/Project/zot.hxx.in
  "static const char* zot = \"zot changed\";\n")
file(WRITE ${BuildDepends_BINARY_DIR}/Project/zot_custom.hxx.in
  "static const char* zot_custom = \"zot_custom changed\";\n")

message("Building project second time")
try_compile(RESULT 
  ${BuildDepends_BINARY_DIR}/Project
  ${BuildDepends_SOURCE_DIR}/Project
  testRebuild
  OUTPUT_VARIABLE OUTPUT)

# Xcode is in serious need of help here
if("${CMAKE_GENERATOR}" MATCHES "Xcode")
  try_compile(RESULT 
    ${BuildDepends_BINARY_DIR}/Project
    ${BuildDepends_SOURCE_DIR}/Project
    testRebuild
    OUTPUT_VARIABLE OUTPUT)
  try_compile(RESULT 
    ${BuildDepends_BINARY_DIR}/Project
    ${BuildDepends_SOURCE_DIR}/Project
    testRebuild
    OUTPUT_VARIABLE OUTPUT)
endif("${CMAKE_GENERATOR}" MATCHES "Xcode")

if(NOT RESULT)
  message(SEND_ERROR "Could not build test project: ${OUTPUT}")
endif(NOT RESULT)
if(EXISTS 
    "${BuildDepends_BINARY_DIR}/Project/Debug/bar${CMAKE_EXECUTABLE_SUFFIX}" )
  message("found debug")
endif(EXISTS 
  "${BuildDepends_BINARY_DIR}/Project/Debug/bar${CMAKE_EXECUTABLE_SUFFIX}")
if(EXISTS 
    "${BuildDepends_BINARY_DIR}/Project/Debug/zot${CMAKE_EXECUTABLE_SUFFIX}" )
  message("found debug")
endif(EXISTS 
  "${BuildDepends_BINARY_DIR}/Project/Debug/zot${CMAKE_EXECUTABLE_SUFFIX}")

message("Running ${bar}  ")
execute_process(COMMAND ${bar} OUTPUT_VARIABLE out RESULT_VARIABLE runResult)
string(REGEX REPLACE "[\r\n]" " " out "${out}")
message("Run result: ${runResult} Output: \"${out}\"")

if("${out}" STREQUAL "foo changed ")
  message("Worked!")
else("${out}" STREQUAL "foo changed ")
  message(SEND_ERROR "Project did not rebuild properly!")
endif("${out}" STREQUAL "foo changed ")

message("Running ${zot}  ")
execute_process(COMMAND ${zot} OUTPUT_VARIABLE out RESULT_VARIABLE runResult)
string(REGEX REPLACE "[\r\n]" " " out "${out}")
message("Run result: ${runResult} Output: \"${out}\"")

if("${out}" STREQUAL "[zot changed] [zot_custom changed] ")
  message("Worked!")
else("${out}" STREQUAL "[zot changed] [zot_custom changed] ")
  message(SEND_ERROR "Project did not rebuild properly!")
endif("${out}" STREQUAL "[zot changed] [zot_custom changed] ")
