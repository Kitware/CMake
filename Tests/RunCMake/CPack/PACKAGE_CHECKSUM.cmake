install(FILES CMakeLists.txt DESTINATION foo)

set(CPACK_PACKAGE_NAME "package_checksum")
set(CPACK_PACKAGE_CHECKSUM ${RunCMake_SUBTEST_SUFFIX})
