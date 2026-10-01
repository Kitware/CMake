set(apk "${RunCMake_TEST_BINARY_DIR}/APKGeneratorTest-1.2.3.apk")
if(NOT EXISTS "${apk}")
  set(RunCMake_TEST_FAILED "APK package not generated")
  return()
endif()

# The application's library and those of the Android archive are packaged
# for the ABIs of both build trees, and for no other.
set(contents "${RunCMake_TEST_BINARY_DIR}/apk-contents")
file(ARCHIVE_EXTRACT INPUT "${apk}" DESTINATION "${contents}")
foreach(library IN ITEMS libapp.so libaar.so)
  file(GLOB abis RELATIVE "${contents}/lib" "${contents}/lib/*/${library}")
  list(TRANSFORM abis REPLACE "/${library}$" "")
  list(SORT abis)
  if(NOT abis STREQUAL "arm64-v8a;x86_64")
    string(APPEND RunCMake_TEST_FAILED
      "APK holds ${library} for '${abis}', expected 'arm64-v8a;x86_64'\n")
  endif()
endforeach()
