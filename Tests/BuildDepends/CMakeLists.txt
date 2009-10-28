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

# Xcode needs some help with the fancy dependencies in this test.
if("${CMAKE_GENERATOR}" MATCHES "Xcode")
  set(HELP_XCODE 1)
endif("${CMAKE_GENERATOR}" MATCHES "Xcode")
function(help_xcode_depends)
  if(HELP_XCODE)
    file(GLOB_RECURSE MACRO_OBJS
      ${BuildDepends_BINARY_DIR}/Project/zot_macro_*.o*
      )
    if(MACRO_OBJS)
      message("Helping Xcode by removing objects [${MACRO_OBJS}]")
      file(REMOVE ${MACRO_OBJS})
    endif(MACRO_OBJS)
  endif(HELP_XCODE)
endfunction(help_xcode_depends)

# The Intel compiler causes the MSVC linker to crash during
# incremental linking, so avoid the /INCREMENTAL:YES flag.
if(WIN32 AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
  set(_cmake_options "-DCMAKE_EXE_LINKER_FLAGS=")
endif()

file(MAKE_DIRECTORY ${BuildDepends_BINARY_DIR}/Project)
message("Creating Project/foo.cxx")
write_file(${BuildDepends_BINARY_DIR}/Project/foo.cxx 
  "const char* foo() { return \"foo\";}" )

file(WRITE ${BuildDepends_BINARY_DIR}/Project/zot.hxx.in
  "static const char* zot = \"zot\";\n")
file(WRITE ${BuildDepends_BINARY_DIR}/Project/zot_custom.hxx.in
  "static const char* zot_custom = \"zot_custom\";\n")
file(WRITE ${BuildDepends_BINARY_DIR}/Project/zot_macro_dir.hxx
  "static const char* zot_macro_dir = \"zot_macro_dir\";\n")
file(WRITE ${BuildDepends_BINARY_DIR}/Project/zot_macro_tgt.hxx
  "static const char* zot_macro_tgt = \"zot_macro_tgt\";\n")

help_xcode_depends()

message("Building project first time")
try_compile(RESULT 
  ${BuildDepends_BINARY_DIR}/Project
  ${BuildDepends_SOURCE_DIR}/Project
  testRebuild
  CMAKE_FLAGS ${_cmake_options}
  OUTPUT_VARIABLE OUTPUT)
if(HELP_XCODE)
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
endif(HELP_XCODE)

message("Output from first build:\n${OUTPUT}")
if(NOT RESULT)
  message(SEND_ERROR "Could not build test project (1)!")
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

set(VALUE_UNCHANGED "[zot] [zot_custom] [zot_macro_dir] [zot_macro_tgt] ")
if("${out}" STREQUAL "${VALUE_UNCHANGED}")
  message("Worked!")
else("${out}" STREQUAL "${VALUE_UNCHANGED}")
  message(SEND_ERROR "Project did not initially build properly: ${out}")
endif("${out}" STREQUAL "${VALUE_UNCHANGED}")

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
file(WRITE ${BuildDepends_BINARY_DIR}/Project/zot_macro_dir.hxx
  "static const char* zot_macro_dir = \"zot_macro_dir changed\";\n")
file(WRITE ${BuildDepends_BINARY_DIR}/Project/zot_macro_tgt.hxx
  "static const char* zot_macro_tgt = \"zot_macro_tgt changed\";\n")

help_xcode_depends()

message("Building project second time")
try_compile(RESULT 
  ${BuildDepends_BINARY_DIR}/Project
  ${BuildDepends_SOURCE_DIR}/Project
  testRebuild
  CMAKE_FLAGS ${_cmake_options}
  OUTPUT_VARIABLE OUTPUT)

# Xcode is in serious need of help here
if(HELP_XCODE)
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
endif(HELP_XCODE)

message("Output from second build:\n${OUTPUT}")
if(NOT RESULT)
  message(SEND_ERROR "Could not build test project (2)!")
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

set(VALUE_CHANGED
  "[zot changed] [zot_custom changed] [zot_macro_dir changed] [zot_macro_tgt changed] "
  )
if("${out}" STREQUAL "${VALUE_CHANGED}")
  message("Worked!")
else("${out}" STREQUAL "${VALUE_CHANGED}")
  message(SEND_ERROR "Project did not rebuild properly!")
endif("${out}" STREQUAL "${VALUE_CHANGED}")
