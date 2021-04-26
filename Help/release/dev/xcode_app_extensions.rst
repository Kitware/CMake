xcode_app_extensions
--------------------

* The :prop_tgt:`XCODE_EMBED_APP_EXTENSIONS <XCODE_EMBED_<type>>` target property
  was added to tell the :generator:`Xcode` generator to embed app extensions
  such as iMessage sticker packs.
  Aspects of the embedding can be customized with the
  :prop_tgt:`XCODE_EMBED_APP_EXTENSIONS_PATH <XCODE_EMBED_<type>>`,
  :prop_tgt:`XCODE_EMBED_APP_EXTENSIONS_CODE_SIGN_ON_COPY <XCODE_EMBED_<type>_CODE_SIGN_ON_COPY>` and
  :prop_tgt:`XCODE_EMBED_APP_EXTENSIONS_REMOVE_HEADERS_ON_COPY <XCODE_EMBED_<type>_REMOVE_HEADERS_ON_COPY>`
  properties.
