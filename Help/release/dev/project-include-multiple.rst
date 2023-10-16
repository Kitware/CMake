project-include-multiple
------------------------

* The :variable:`CMAKE_PROJECT_INCLUDE`,
  :variable:`CMAKE_PROJECT_INCLUDE_BEFORE`,
  :variable:`CMAKE_PROJECT_<PROJECT-NAME>_INCLUDE`, and
  :variable:`CMAKE_PROJECT_<PROJECT-NAME>_INCLUDE_BEFORE` variables learned
  to support a :ref:`semicolon-separated list <CMake Language Lists>` of
  CMake language files to be included sequentially. These variables can also
  reference module names to be found in :variable:`CMAKE_MODULE_PATH` or
  builtin to CMake.
