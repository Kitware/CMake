install_name_policy
-------------------

* A :prop_tgt:`BUILD_WITH_INSTALL_NAME_DIR` target property and corresponding
  :variable:`CMAKE_BUILD_WITH_INSTALL_NAME_DIR` variable were added to
  control whether to use the :prop_tgt:`INSTALL_NAME_DIR` target property
  value for binaries in the build tree.  This is for macOS ``install_name``
  as :prop_tgt:`BUILD_WITH_INSTALL_RPATH` is for ``RPATH``.

* On macOS, ``RPATH`` settings such as :prop_tgt:`BUILD_WITH_INSTALL_RPATH`
  no longer affect the ``install_name`` field.  See policy :policy:`CMP0068`.
