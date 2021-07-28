add_library(app_extension MODULE Empty.txt)
set_target_properties(app_extension PROPERTIES
  LINKER_LANGUAGE CXX
  BUNDLE YES
  XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
  XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
  XCODE_ATTRIBUTE_ENABLE_BITCODE "NO"
  MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
  MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.app.app_extension"
  XCODE_PRODUCT_TYPE "com.apple.product-type.app-extension"
  XCODE_EXPLICIT_FILE_TYPE "wrapper.app-extension"
)

add_executable(app MACOSX_BUNDLE main.m)
add_dependencies(app app_extension)
set_target_properties(app PROPERTIES
  XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
  XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
  XCODE_EMBED_APP_EXTENSIONS app_extension
  MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.app"
)
