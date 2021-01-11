xcode-embed-frameworks
----------------------

* When using the Xcode generator, it is now possible to embed frameworks
  using the new :prop_tgt:`XCODE_EMBED_FRAMEWORKS <XCODE_EMBED_<type>>`
  target property.  Aspects of the embedding can be customized with the
  :prop_tgt:`XCODE_EMBED_FRAMEWORKS_PATH <XCODE_EMBED_<type>>`,
  :prop_tgt:`XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY` and
  :prop_tgt:`XCODE_EMBED_FRAMEWORKS_REMOVE_HEADERS_ON_COPY` target properties.
