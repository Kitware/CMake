include_guard
-------------

* The :command:`include_guard` command was introduced to allow guarding
  CMake scripts from being included more than once. The command supports
  ``DIRECTORY`` and ``GLOBAL`` options to adjust the corresponding include guard
  scope. If no options given, include guard is similar to basic variable-based
  check.
