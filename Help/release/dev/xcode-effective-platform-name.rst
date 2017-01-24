xcode-effective-platform-name
-----------------------------

* The Xcode generator can now control emission of the
  ``EFFECTIVE_PLATFORM_NAME`` variable through the
  :prop_gbl:`XCODE_EMIT_EFFECTIVE_PLATFORM_NAME` global property.
  This is useful when building with multiple SDKs like macosx and
  iphoneos in parallel.
