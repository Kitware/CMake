set(pkg1_cmake "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/Export/59965f5e1aafdb63698f8ae505daf864/pkg1.cmake")
file(STRINGS "${pkg1_cmake}" pkg1_includes REGEX INTERFACE_INCLUDE_DIRECTORIES)
set(pkg1_expect [[INTERFACE_INCLUDE_DIRECTORIES "\${_IMPORT_PREFIX}/pkg1/inc"]])
if(NOT pkg1_includes MATCHES "${pkg1_expect}")
  set(RunCMake_TEST_FAILED "pkg1 has unexpected INTERFACE_INCLUDE_DIRECTORIES line:
  ${pkg1_includes}
It does not match:
  ${pkg1_expect}")
endif()

set(pkg2_cmake "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/Export/72c00a5f9d34b6649110956cfc9f27e6/pkg2.cmake")
file(STRINGS "${pkg2_cmake}" pkg2_includes REGEX INTERFACE_INCLUDE_DIRECTORIES)
set(pkg2_expect [[INTERFACE_INCLUDE_DIRECTORIES "\${_IMPORT_PREFIX}/pkg2/inc"]])
if(NOT pkg2_includes MATCHES "${pkg2_expect}")
  set(RunCMake_TEST_FAILED "pkg2 has unexpected INTERFACE_INCLUDE_DIRECTORIES line:
  ${pkg2_includes}
It does not match:
  ${pkg2_expect}")
endif()
