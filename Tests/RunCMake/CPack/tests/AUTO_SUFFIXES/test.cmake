install(FILES CMakeLists.txt DESTINATION foo COMPONENT test)

# if the filename doesn't have the expected deb suffix, test that it is appended automatically
set(CPACK_DEBIAN_FILE_NAME "autosuffixpackage")
