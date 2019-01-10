AUTO*_EXECUTABLE
----------------

* The :prop_tgt:`AUTOMOC_EXECUTABLE`, :prop_tgt:`AUTORCC_EXECUTABLE` and
  :prop_tgt:`AUTOUIC_EXECUTABLE` target properties all take a path to an
  executable and force automoc/autorcc/autouic to use this executable.

  Setting these will also prevent the configure time testing for these
  executables. This is mainly useful when you build these tools yourself.
