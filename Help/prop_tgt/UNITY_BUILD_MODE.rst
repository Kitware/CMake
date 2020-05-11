UNITY_BUILD_MODE
----------------

CMake provides different algorithms for selecting which sources are grouped
together into a *bucket*. Selection is decided by this property,
which has the following acceptable values:

* ``BATCH``
  When in this mode CMake determines which files are grouped together.
  The :prop_tgt:`UNITY_BUILD_BATCH_SIZE` property controls the upper limit on
  how many sources can be combined per unity source file.

* ``GROUP``
  When in this mode each target explicitly specifies how to group
  source files. Each source file that has the same
  :prop_sf:`UNITY_GROUP` value will be grouped together. Any sources
  that don't have this property will be compiled individually. The
  :prop_tgt:`UNITY_BUILD_BATCH_SIZE` property is ignored when using
  this mode.

If no explicit :prop_tgt:`UNITY_BUILD_MODE` has been specified, CMake will
default to ``BATCH``.
