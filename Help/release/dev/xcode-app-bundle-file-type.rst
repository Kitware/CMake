xcode-app-bundle-file-type
--------------------------

* The :generator:`Xcode` generator now sets the file type of application
  bundles to ``wrapper.application``, so that Xcode launches them as
  applications instead of as plain executables.  Executables with a product
  type other than an application, set by :prop_tgt:`XCODE_PRODUCT_TYPE`, or
  with a custom :prop_tgt:`BUNDLE_EXTENSION`, are not affected.  Use
  :prop_tgt:`XCODE_EXPLICIT_FILE_TYPE` to override the file type.
