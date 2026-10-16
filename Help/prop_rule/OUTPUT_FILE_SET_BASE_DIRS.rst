OUTPUT_FILE_SET_BASE_DIRS
-------------------------

.. versionadded:: 4.5

Specify the base directories for the output file set.

The ``OUTPUT_FILE_SET_BASE_DIRS`` may be set to a
:ref:`semicolon-separated list <CMake Language Lists>` of directories.

The directories can use the following patterns:

* ``<RULE>``: name of the rule
* ``<TARGET>``: name of the target
* ``<FILE_SET>``: name of the file set
* ``<CURRENT_SOURCE_DIR>``: path to the source directory of the file set
  creation
* ``<CURRENT_BINARY_DIR>``: path to the binary directory of the file set
  creation
* ``<OUTPUT_CURRENT_SOURCE_DIR>``: path to the source directory of the output
  file set creation
* ``<OUTPUT_CURRENT_BINARY_DIR>``: path to the binary directory of the output
  file set creation

This property gets the following default value: ``<CURRENT_BINARY_DIR>``.

Contents of ``OUTPUT_FILE_SET_BASE_DIRS`` may use generator expressions with
the syntax ``$<...>``. See the cmake-buildsystem(7) manual for more on defining
buildsystem properties.
