install(FILES CMakeLists.txt DESTINATION foo COMPONENT test)

# if the filename doesn't have the expected deb/rpm suffix, test that it is appended automatically
set(CPACK_DEBIAN_FILE_NAME "autosuffixpackage")
set(CPACK_RPM_FILE_NAME "autosuffixpackage")
