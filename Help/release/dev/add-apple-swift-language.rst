add-apple-swift-language
------------------------

* CMake learned rudimentary support for the Apple Swift language.  When using
  the :generator:`Xcode` generator with Xcode 6.1 or higher, one may enable
  the ``Swift`` language with the :command:`enable_language` command or the
  :command:`project` command (this is an error with other generators or when
  Xcode is too old).  Then one may list ``.swift`` source files in targets
  for compilation.
