# Shared by the *-check.cmake scripts: verify that appimagetool recorded
# ${expected_version} as X-AppImage-Version in the package's desktop file.
set(appimage "${RunCMake_TEST_BINARY_DIR}/GeneratorTest-1.2.3-Linux.AppImage")
if(NOT EXISTS "${appimage}")
  set(RunCMake_TEST_FAILED "AppImage package not generated")
  return()
endif()

set(extract_dir "${RunCMake_TEST_BINARY_DIR}/extract")
file(REMOVE_RECURSE "${extract_dir}")
file(MAKE_DIRECTORY "${extract_dir}")
execute_process(
  COMMAND "${appimage}" --appimage-extract com.example.app.desktop
  WORKING_DIRECTORY "${extract_dir}"
  RESULT_VARIABLE extract_result
  OUTPUT_QUIET
  ERROR_VARIABLE extract_error
  )
set(desktop_file "${extract_dir}/squashfs-root/com.example.app.desktop")
if(NOT extract_result EQUAL 0 OR NOT EXISTS "${desktop_file}")
  set(RunCMake_TEST_FAILED "Failed to extract desktop file from AppImage: ${extract_result}\n${extract_error}")
  return()
endif()

file(STRINGS "${desktop_file}" actual_version REGEX "^X-AppImage-Version=")
if(NOT actual_version STREQUAL "X-AppImage-Version=${expected_version}")
  set(RunCMake_TEST_FAILED "Expected 'X-AppImage-Version=${expected_version}' in desktop file, got: '${actual_version}'")
endif()
