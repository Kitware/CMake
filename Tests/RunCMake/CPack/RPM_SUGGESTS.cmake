install(FILES CMakeLists.txt DESTINATION foo)

set(CPACK_RPM_PACKAGE_SUGGESTS "libsuggested")
set(CPACK_PACKAGE_NAME "rpm_suggests")
