enable_language(C)

add_executable(tool main.c)

add_executable(app MACOSX_BUNDLE main.c)

add_executable(app_ext_app MACOSX_BUNDLE main.c)
set_property(TARGET app_ext_app PROPERTY BUNDLE_EXTENSION "app")

# Variants of the application product type are also applications.
add_executable(app_clip MACOSX_BUNDLE main.c)
set_property(TARGET app_clip PROPERTY
  XCODE_PRODUCT_TYPE "com.apple.product-type.application.on-demand-install-capable")

# Bundles that are not applications.
add_executable(xpc_by_extension MACOSX_BUNDLE main.c)
set_property(TARGET xpc_by_extension PROPERTY BUNDLE_EXTENSION "xpc")

add_executable(xpc_by_product_type MACOSX_BUNDLE main.c)
set_property(TARGET xpc_by_product_type PROPERTY
  XCODE_PRODUCT_TYPE "com.apple.product-type.xpc-service")

add_executable(app_extension MACOSX_BUNDLE main.c)
set_property(TARGET app_extension PROPERTY
  XCODE_PRODUCT_TYPE "com.apple.product-type.app-extension")

# An explicit file type takes precedence.
add_executable(explicit MACOSX_BUNDLE main.c)
set_property(TARGET explicit PROPERTY
  XCODE_EXPLICIT_FILE_TYPE "wrapper.app-extension")
