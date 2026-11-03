include(RunCPack) # Uses sample projects from `../RunCPack/*`

set(RunCPack_GENERATORS AppImage)

if(CMake_TEST_CPACK_APPIMAGE_RUNTIME_FILE)
  list(APPEND RunCMake_TEST_OPTIONS "-DCPACK_APPIMAGE_RUNTIME_FILE=${CMake_TEST_CPACK_APPIMAGE_RUNTIME_FILE}")
endif()

run_cpack(AppImageTestApp BUILD)
run_cpack(AppImageUsrPrefix BUILD)

block()
  list(APPEND RunCMake_TEST_OPTIONS -DCUSTOM_APPRUN=ON)
  run_cpack(AppImageCustomAppRun SAMPLE AppImageUsrPrefix BUILD)
endblock()
