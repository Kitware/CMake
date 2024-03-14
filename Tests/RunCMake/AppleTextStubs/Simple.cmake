enable_language(C)

add_library(foo SHARED foo.c)
set_property(TARGET foo PROPERTY ENABLE_EXPORTS TRUE)

add_executable(main main.c)
target_link_libraries(main PRIVATE foo)

add_subdirectory(SUBDIR)

install(TARGETS foo DESTINATION "${CMAKE_BINARY_DIR}/INSTALL")

install(TARGETS foo LIBRARY DESTINATION "${CMAKE_BINARY_DIR}/INSTALL2/lib"
                    ARCHIVE DESTINATION "${CMAKE_BINARY_DIR}/INSTALL2/dev")


set (GENERATE_CONTENT "if (\"${CMAKE_TAPI}\")
  set (APPLE_TEXT_STUBS_SUPPORTED TRUE)
endif()\n\n")

string (APPEND GENERATE_CONTENT [[
macro (CHECK_FILE test_msg path)
  if (NOT EXISTS "${path}")
    string (APPEND RunCMake_TEST_FAILED "${test_msg}: \"${path}\" not found\n")
  endif()
endmacro()

check_file("foo DYLIB file" "$<TARGET_FILE:foo>")
check_file("foo2 DYLIB file" "$<TARGET_FILE:foo2>")
check_file("executable file" "$<TARGET_FILE:main>")

check_file("Installed foo DYLIB file" "${RunCMake_TEST_BINARY_DIR}/INSTALL/$<TARGET_FILE_NAME:foo>")
check_file("Installed foo2 DYLIB file" "${RunCMake_TEST_BINARY_DIR}/INSTALL/$<TARGET_FILE_NAME:foo2>")
check_file("Installed DYLIB file" "${RunCMake_TEST_BINARY_DIR}/INSTALL2/lib/$<TARGET_FILE_NAME:foo>")

if (APPLE_TEXT_STUBS_SUPPORTED)
  check_file("foo TBD file" "$<TARGET_IMPORT_FILE:foo>")
  check_file("foo2 TBD file" "$<TARGET_IMPORT_FILE:foo2>")

  check_file("Installed foo TBD file" "${RunCMake_TEST_BINARY_DIR}/INSTALL/$<TARGET_IMPORT_FILE_NAME:foo>")
  check_file("Installed foo2 TBD file" "${RunCMake_TEST_BINARY_DIR}/INSTALL/$<TARGET_IMPORT_FILE_NAME:foo2>")
  check_file("Installed TBD file" "${RunCMake_TEST_BINARY_DIR}/INSTALL2/dev/$<TARGET_IMPORT_FILE_NAME:foo>")
endif()
]])

file (GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/Simple-$<CONFIG>-generated.cmake"
  CONTENT "${GENERATE_CONTENT}")
