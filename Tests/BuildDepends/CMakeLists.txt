# this test creates a static library and an executable
# the source to the library is then changed 
# and the build is done on the executable and if things
# are working the executable should relink with the new
# value.  The subdir Project contains the CMakelists.txt
# and source files for the test project. 
project(BuildDepends)
make_directory(${BuildDepends_BINARY_DIR}/Project)
write_file(${BuildDepends_BINARY_DIR}/Project/foo.cxx 
  "const char* foo() { return \"foo\";}" )
try_compile(RESULT 
  ${BuildDepends_BINARY_DIR}/Project
  ${BuildDepends_SOURCE_DIR}/Project
  testRebuild
  OUTPUT_VARIABLE OUTPUT)
if(NOT RESULT)
  message(SEND_ERROR "Could not build test project: ${OUTPUT}")
endif(NOT RESULT)

set(bar ${BuildDepends_BINARY_DIR}/Project/bar${CMAKE_EXECUTABLE_SUFFIX})
message("${BuildDepends_BINARY_DIR}/Project/Debug/bar${CMAKE_EXECUTABLE_SUFFIX}")
if(EXISTS 
    "${BuildDepends_BINARY_DIR}/Project/Debug/bar${CMAKE_EXECUTABLE_SUFFIX}" )
  message("found debug")
  set(bar 
    "${BuildDepends_BINARY_DIR}/Project/Debug/bar${CMAKE_EXECUTABLE_SUFFIX}")
endif(EXISTS 
  "${BuildDepends_BINARY_DIR}/Project/Debug/bar${CMAKE_EXECUTABLE_SUFFIX}")
message("running ${bar}  ")
execute_process(COMMAND ${bar} OUTPUT_VARIABLE out TIMEOUT 3)
string(REGEX REPLACE "[\r\n]" " " out "${out}")
message("${out}")
if("${out}" STREQUAL "foo ")
  message("Worked!")
else("${out}" STREQUAL "foo ")
  message(SEND_ERROR "Program did not rebuild with changed file: ${out}")
endif("${out}" STREQUAL "foo ")

write_file(${BuildDepends_BINARY_DIR}/Project/foo.cxx
  "const char* foo() { return \"foo changed\";}" )
try_compile(RESULT 
  ${BuildDepends_BINARY_DIR}/Project
  ${BuildDepends_SOURCE_DIR}/Project
  testRebuild
  OUTPUT_VARIABLE OUTPUT)

if(NOT RESULT)
  message(SEND_ERROR "Could not build test project: ${OUTPUT}")
endif(NOT RESULT)
if(EXISTS 
    "${BuildDepends_BINARY_DIR}/Project/Debug/bar${CMAKE_EXECUTABLE_SUFFIX}" )
  message("found debug")
endif(EXISTS 
  "${BuildDepends_BINARY_DIR}/Project/Debug/bar${CMAKE_EXECUTABLE_SUFFIX}")

execute_process(COMMAND ${bar} OUTPUT_VARIABLE out TIMEOUT 3)
string(REGEX REPLACE "[\r\n]" " " out "${out}")
message("${out}")

if("${out}" STREQUAL "foo changed ")
  message("Worked!")
else("${out}" STREQUAL "foo changed ")
  message(SEND_ERROR "Program did not rebuild with changed file")
endif("${out}" STREQUAL "foo changed ")
