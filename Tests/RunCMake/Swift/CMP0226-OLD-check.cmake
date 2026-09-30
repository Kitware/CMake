if(RunCMake_GENERATOR MATCHES "Xcode")
  file(READ
    "${RunCMake_TEST_BINARY_DIR}/CMP0226-OLD.xcodeproj/project.pbxproj"
    pbxproj)
  if(pbxproj MATCHES "PRODUCT_MODULE_NAME")
    string(APPEND RunCMake_TEST_FAILED
      "Did not expect PRODUCT_MODULE_NAME in Xcode project with CMP0226 OLD\n")
  endif()
endif()
