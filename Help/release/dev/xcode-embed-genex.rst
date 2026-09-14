xcode-embed-genex
-----------------

* The :prop_tgt:`XCODE_EMBED_<type>` target property now supports
  :manual:`generator expressions <cmake-generator-expressions(7)>`.
  Because Xcode shares one copy-files build phase across all configurations,
  an expression whose result depends on the configuration is rejected with
  an error.
