xcode-lib-dirs
--------------

* The :generator:`Xcode` generator no longer adds the per-config suffix
  ``$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)`` to library search paths.
  See policy :policy:`CMP0142`.
