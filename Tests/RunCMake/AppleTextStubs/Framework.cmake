enable_language(C)

add_library(foo SHARED foo.c)
set_property(TARGET foo PROPERTY ENABLE_EXPORTS TRUE)
set_property(TARGET foo PROPERTY FRAMEWORK TRUE)

add_executable(main main.c)
target_link_libraries(main PRIVATE foo)


install(TARGETS foo FRAMEWORK DESTINATION "${CMAKE_BINARY_DIR}/INSTALL")

# LIBRARY and ARCHIVE should be ignored
install(TARGETS foo FRAMEWORK DESTINATION "${CMAKE_BINARY_DIR}/INSTALL2"
                    LIBRARY DESTINATION "${CMAKE_BINARY_DIR}/INSTALL2/lib"
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

macro (CHECK_SYMLINK test_msg path)
  if(NOT IS_SYMLINK "${path}")
    string (APPEND RunCMake_TEST_FAILED "${test_msg}: \"${path}\" is not a symbolic link\n")
  elseif (NOT EXISTS "${path}")
    string (APPEND RunCMake_TEST_FAILED "${test_msg}: \"${path}\" is not a valid symlink\n")
  endif()
endmacro()

check_file("DYLIB file" "$<TARGET_FILE:foo>")
check_symlink("Public DYLIB file" "$<TARGET_LINKER_LIBRARY_FILE:foo>")
check_file("executable file" "$<TARGET_FILE:main>")

check_file("Installed DYLIB file" "${RunCMake_TEST_BINARY_DIR}/INSTALL/foo.framework/Versions/A/$<TARGET_FILE_NAME:foo>")
check_symlink("Installed Public DULIB file" "${RunCMake_TEST_BINARY_DIR}/INSTALL/foo.framework/$<TARGET_FILE_NAME:foo>")
check_file("Installed DULIB file" "${RunCMake_TEST_BINARY_DIR}/INSTALL2/foo.framework/Versions/A/$<TARGET_FILE_NAME:foo>")
check_symlink("Installed Public DYLIB file" "${RunCMake_TEST_BINARY_DIR}/INSTALL2/foo.framework/$<TARGET_FILE_NAME:foo>")

if (APPLE_TEXT_STUBS_SUPPORTED)
  check_file("TBD file" "$<TARGET_IMPORT_FILE:foo>")
  check_symlink("Public TBD file" "$<TARGET_LINKER_IMPORT_FILE:foo>")

  check_file("Installed TBD file" "${RunCMake_TEST_BINARY_DIR}/INSTALL/foo.framework/Versions/A/$<TARGET_IMPORT_FILE_NAME:foo>")
  check_symlink("Installed Public TBD file" "${RunCMake_TEST_BINARY_DIR}/INSTALL/foo.framework/$<TARGET_IMPORT_FILE_NAME:foo>")
  check_file("Installed TBD file" "${RunCMake_TEST_BINARY_DIR}/INSTALL2/foo.framework/Versions/A/$<TARGET_IMPORT_FILE_NAME:foo>")
  check_symlink("Installed Public TBD file" "${RunCMake_TEST_BINARY_DIR}/INSTALL2/foo.framework/$<TARGET_IMPORT_FILE_NAME:foo>")
endif()
]])

file (GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/Framework-$<CONFIG>-generated.cmake"
  CONTENT "${GENERATE_CONTENT}")
