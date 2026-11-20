rpath-head-target
-----------------

* The :prop_tgt:`BUILD_RPATH` and :prop_tgt:`INSTALL_RPATH` target properties
  now support generator expressions that refer to the current target,
  such as :genex:`$<TARGET_PROPERTY:prop>`.
