add_executable(app_extensionkit_extension main.m)
set_target_properties(app_extensionkit_extension PROPERTIES
  LINKER_LANGUAGE CXX
  BUNDLE YES
  XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
  XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
  XCODE_ATTRIBUTE_ENABLE_BITCODE "NO"
  XCODE_ATTRIBUTE_GENERATE_INFOPLIST_FILE "YES"
  MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/ExtensionKit.Info.plist.in"
  MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.app.app_extensionkit_extension"
  XCODE_PRODUCT_TYPE "com.apple.product-type.extensionkit-extension"
  XCODE_EXPLICIT_FILE_TYPE "wrapper.extensionkit-extension"
)

add_executable(app MACOSX_BUNDLE main.m)
add_dependencies(app app_extension)
set_target_properties(app PROPERTIES
  XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
  XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
  XCODE_EMBED_EXTENSIONKIT_EXTENSIONS app_extension
  MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.app"
)
