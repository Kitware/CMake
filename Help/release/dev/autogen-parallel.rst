autogen-parallel
----------------

* When using :prop_tgt:`AUTOMOC` or :prop_tgt:`AUTOUIC`, CMake starts multiple
  parallel ``moc`` or ``uic`` processes to reduce the build time.
  The new :variable:`CMAKE_AUTOGEN_PARALLEL` variable and
  :prop_tgt:`AUTOGEN_PARALLEL` target property allow to modify the number of
  parallel ``moc`` or ``uic`` processes to start.
  By default CMake starts a single ``moc`` or ``uic`` process for each physical
  CPU on the host system.
