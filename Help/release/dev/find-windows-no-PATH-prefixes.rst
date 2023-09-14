find-windows-no-PATH-prefixes
-----------------------------

* The :command:`find_library`, :command:`find_path`, and :command:`find_file`
  commands no longer search in installation prefixes derived from the ``PATH``
  environment variable.  This behavior was added in CMake 3.3 to support
  MSYS and MinGW (``MSYSTEM``) development environments on Windows, but
  it can search undesired prefixes that happen to be in the ``PATH`` for
  unrelated reasons.  Users that keep some ``<prefix>/bin`` directories in
  the ``PATH`` just for their tools do not necessarily want any corresponding
  ``<prefix>/lib`` or ``<prefix>/include`` directories searched.
  The behavior was reverted for non-Windows platforms by CMake 3.6.
  Now it has been reverted on Windows platforms too.

  One may set the ``CMAKE_PREFIX_PATH`` environment variable with a
  :ref:`semicolon-separated list <CMake Language Lists>` of prefixes
  that are to be searched.
