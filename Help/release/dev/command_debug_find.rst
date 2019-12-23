cmake-debug-find
----------------

* :manual:`cmake(1)` gained a ``--debug-find`` command line
  option that can be used to provide information on where find
  commands searched.

* Variable :variable:`CMAKE_FIND_DEBUG_MODE` was introduced to
  print extra find call information during the cmake run to standard
  error. Output is designed for human consumption and not for parsing.
