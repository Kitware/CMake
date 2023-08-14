enable_language(C)

add_library(foo SHARED foo.c)
set_property(TARGET foo PROPERTY ENABLE_EXPORTS TRUE)
set_property (TARGET foo PROPERTY VERSION 2.5.0)
set_property (TARGET foo PROPERTY SOVERSION 2.0.0)

add_executable(main main.c)
target_link_libraries(main PRIVATE foo)


install(TARGETS foo DESTINATION "${CMAKE_BINARY_DIR}/INSTALL" COMPONENT default)

install(TARGETS foo ARCHIVE DESTINATION "${CMAKE_BINARY_DIR}/INSTALL2/dev1" NAMELINK_SKIP COMPONENT default)
install(TARGETS foo ARCHIVE DESTINATION "${CMAKE_BINARY_DIR}/INSTALL2/dev2" NAMELINK_ONLY COMPONENT default)

install(TARGETS foo ARCHIVE DESTINATION "${CMAKE_BINARY_DIR}/INSTALL3"
                            COMPONENT lib3 NAMELINK_COMPONENT dev3)
install(TARGETS foo ARCHIVE DESTINATION "${CMAKE_BINARY_DIR}/INSTALL4"
                            COMPONENT lib4 NAMELINK_COMPONENT dev4)


set (GENERATE_CONTENT "if (\"${CMAKE_TAPI}\")
  set (APPLE_TEXT_STUBS_SUPPORTED TRUE)
endif()\n\n")

string (APPEND GENERATE_CONTENT [[
cmake_policy (SET CMP0011 NEW)
cmake_policy (SET CMP0057 NEW)

macro (CHECK_FILE test_msg path)
  if (NOT EXISTS "${path}")
    string (APPEND RunCMake_TEST_FAILED "${test_msg}: \"${path}\" not found\n")
  endif()
endmacro()

macro (CHECK_SYMLINK test_msg path)
  if (NOT IS_SYMLINK "${path}")
    string (APPEND RunCMake_TEST_FAILED "${test_msg}: \"${path}\" is not a symbolic link\n")
  elseif (NOT EXISTS "${path}")
    string (APPEND RunCMake_TEST_FAILED "${test_msg}: \"${path}\" not a valid symlink\n")
  endif()
endmacro()

macro (CHECK_NOFILE test_msg path)
  if (EXISTS "${path}")
    string (APPEND RunCMake_TEST_FAILED "${test_msg}: \"${path}\" was found\n")
  endif()
endmacro()

macro (CHECK_INSTALLED test_msg dir file)
  file(GLOB installed_files LIST_DIRECTORIES FALSE RELATIVE "${dir}" "${dir}/*")
  if (NOT "${file}" IN_LIST installed_files)
    string (APPEND RunCMake_TEST_FAILED "${test_msg}: \"${dir}/${file}\" not found\n")
  endif()
endmacro()


check_file("DYLIB file" "$<TARGET_FILE:foo>")
check_symlink("Linkable DYLIB file" "$<TARGET_LINKER_LIBRARY_FILE:foo>")
check_symlink("SONAME DYLIB file" "$<TARGET_SONAME_FILE:foo>")
check_file("executable file" "$<TARGET_FILE:main>")

check_file("Installed DYLIB file" "${RunCMake_TEST_BINARY_DIR}/INSTALL/$<TARGET_FILE_NAME:foo>")
check_symlink("Installed Linkable DYLIB file" "${RunCMake_TEST_BINARY_DIR}/INSTALL/$<TARGET_LINKER_LIBRARY_FILE_NAME:foo>")
check_symlink("Installed SONAME DYLIB file" "${RunCMake_TEST_BINARY_DIR}/INSTALL/$<TARGET_SONAME_FILE_NAME:foo>")

if (APPLE_TEXT_STUBS_SUPPORTED)
  check_file("TBD file" "$<TARGET_IMPORT_FILE:foo>")
  check_symlink("Linkable TBD file" "$<TARGET_LINKER_IMPORT_FILE:foo>")
  check_symlink("SONAME TBD file" "$<TARGET_SONAME_IMPORT_FILE:foo>")

  check_file("Installed TBD file" "${RunCMake_TEST_BINARY_DIR}/INSTALL/$<TARGET_IMPORT_FILE_NAME:foo>")
  check_symlink("Installed Linkable TBD file" "${RunCMake_TEST_BINARY_DIR}/INSTALL/$<TARGET_LINKER_IMPORT_FILE_NAME:foo>")
  check_symlink("Installed SONAME TBD file" "${RunCMake_TEST_BINARY_DIR}/INSTALL/$<TARGET_SONAME_IMPORT_FILE_NAME:foo>")

  check_file("Installed TBD file" "${RunCMake_TEST_BINARY_DIR}/INSTALL2/dev1/$<TARGET_IMPORT_FILE_NAME:foo>")
  check_symlink("Installed SONAME TBD file" "${RunCMake_TEST_BINARY_DIR}/INSTALL2/dev1/$<TARGET_SONAME_IMPORT_FILE_NAME:foo>")
  check_nofile("Installed Linkable TBD file" "${RunCMake_TEST_BINARY_DIR}/INSTALL2/dev1/$<TARGET_LINKER_IMPORT_FILE_NAME:foo>")

  check_installed("Installed Linkable TBD file" "${RunCMake_TEST_BINARY_DIR}/INSTALL2/dev2" "$<TARGET_LINKER_IMPORT_FILE_NAME:foo>")
  check_nofile("Installed TBD file" "${RunCMake_TEST_BINARY_DIR}/INSTALL2/dev2/$<TARGET_IMPORT_FILE_NAME:foo>")
  check_nofile("Installed SONAME TBD file" "${RunCMake_TEST_BINARY_DIR}/INSTALL2/dev2/$<TARGET_SONAME_IMPORT_FILE_NAME:foo>")

  check_file("Installed TBD file" "${RunCMake_TEST_BINARY_DIR}/INSTALL3/$<TARGET_IMPORT_FILE_NAME:foo>")
  check_symlink("Installed SONAME TBD file" "${RunCMake_TEST_BINARY_DIR}/INSTALL3/$<TARGET_SONAME_IMPORT_FILE_NAME:foo>")
  check_nofile("Installed Linkable TBD file" "${RunCMake_TEST_BINARY_DIR}/INSTALL3/$<TARGET_LINKER_IMPORT_FILE_NAME:foo>")

  check_file("Installed TBD file" "${RunCMake_TEST_BINARY_DIR}/INSTALL4/$<TARGET_IMPORT_FILE_NAME:foo>")
  check_symlink("Installed SONAME TBD file" "${RunCMake_TEST_BINARY_DIR}/INSTALL4/$<TARGET_SONAME_IMPORT_FILE_NAME:foo>")
  check_symlink("Installed Linkable TBD file" "${RunCMake_TEST_BINARY_DIR}/INSTALL4/$<TARGET_LINKER_IMPORT_FILE_NAME:foo>")
endif()
]])

file (GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/LibraryWithVersions-$<CONFIG>-generated.cmake"
  CONTENT "${GENERATE_CONTENT}")
