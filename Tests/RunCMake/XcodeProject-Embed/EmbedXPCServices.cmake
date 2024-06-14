add_executable(xpc_service MACOSX_BUNDLE main.m)
set_target_properties(xpc_service PROPERTIES
  BUNDLE_EXTENSION "xpc"
  XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
  XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
  MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/XPCService.Info.plist.in"
  MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.app.xpc_service"
)

add_executable(app MACOSX_BUNDLE main.m)
add_dependencies(app xpc_service)
set_target_properties(app PROPERTIES
    XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
    XCODE_EMBED_XPC_SERVICES xpc_service
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.app"
)
