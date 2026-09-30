if(RunCMake_GENERATOR MATCHES "Xcode")
  file(READ
    "${RunCMake_TEST_BINARY_DIR}/CMP0226-NEW.xcodeproj/project.pbxproj"
    pbxproj)
  if(NOT pbxproj MATCHES "PRODUCT_MODULE_NAME = CustomModule")
    string(APPEND RunCMake_TEST_FAILED
      "Expected PRODUCT_MODULE_NAME = CustomModule in Xcode project for L\n")
  endif()
endif()
