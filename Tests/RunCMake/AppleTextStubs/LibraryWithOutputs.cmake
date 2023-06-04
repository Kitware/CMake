enable_language(C)

add_library(foo SHARED foo.c)
set_property(TARGET foo PROPERTY ENABLE_EXPORTS TRUE)
set_property(TARGET foo PROPERTY ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/TBD/$<CONFIG>")
set_property(TARGET foo PROPERTY ARCHIVE_OUTPUT_NAME "tbd")

add_executable(main main.c)
target_link_libraries(main PRIVATE foo)
set_property(TARGET main PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")

add_custom_target(run COMMAND "$<TARGET_FILE:main>")


set (GENERATE_CONTENT "if (\"${CMAKE_TAPI}\")
  set (APPLE_TEXT_STUBS_SUPPORTED TRUE)
endif()\n\n")

string (APPEND GENERATE_CONTENT [[
macro (CHECK_FILE test_msg path)
  if (NOT EXISTS "${path}")
    string (APPEND RunCMake_TEST_FAILED "${test_msg}: \"${path}\" not found\n")
  endif()
endmacro()

check_file("DYLIB file" "$<TARGET_FILE:foo>")
check_file("executable file" "$<TARGET_FILE:main>")

if (APPLE_TEXT_STUBS_SUPPORTED)
  check_file("TBD file" "$<TARGET_IMPORT_FILE:foo>")
]])

if (CMAKE_GENERATOR STREQUAL "Xcode")
  # ARCHIVE outputs are ignored by this generator
  string (APPEND GENERATE_CONTENT
    "\n  if (NOT \"$<TARGET_IMPORT_FILE_DIR:foo>\" STREQUAL \"${CMAKE_BINARY_DIR}/$<CONFIG>\")
    string (APPEND RunCMake_TEST_FAILED \"Wrong directory for TBD file: \\\"$<TARGET_IMPORT_FILE_DIR:foo>\\\"\n\")
  endif()
  if (NOT \"$<TARGET_IMPORT_FILE_BASE_NAME:foo>\" STREQUAL \"foo\")
    string (APPEND RunCMake_TEST_FAILED \"Wrong base name for TBD file: \\\"$<TARGET_IMPORT_FILE_BASE_NAME:foo>\\\"\n\")
  endif()\n")
else()
  string (APPEND GENERATE_CONTENT
    "\n  if (NOT \"$<TARGET_IMPORT_FILE_DIR:foo>\" STREQUAL \"${CMAKE_BINARY_DIR}/TBD/$<CONFIG>\")
    string (APPEND RunCMake_TEST_FAILED \"Wrong directory for TBD file: \\\"$<TARGET_IMPORT_FILE_DIR:foo>\\\"\n\")
  endif()
  if (NOT \"$<TARGET_IMPORT_FILE_BASE_NAME:foo>\" STREQUAL \"tbd\")
    string (APPEND RunCMake_TEST_FAILED \"Wrong base name for TBD file: \\\"$<TARGET_IMPORT_FILE_BASE_NAME:foo>\\\"\n\")
  endif()\n")
endif()
string (APPEND GENERATE_CONTENT "endif()\n")


file (GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/LibraryWithOutputs-$<CONFIG>-generated.cmake"
  CONTENT "${GENERATE_CONTENT}")
