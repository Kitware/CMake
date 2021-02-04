object-lib-multiarch
--------------------

* The :command:`add_library` command previously prohibited imported object
  libraries when using potentially multi-architecture configurations.
  This mostly affected the :generator:`Xcode` generator, e.g. when targeting
  iOS or one of the other device platforms.  This restriction has now been
  removed.
