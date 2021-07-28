<PROJECT-NAME>_IS_TOP_LEVEL
---------------------------

.. versionadded:: 3.21

A boolean variable indicating whether the named project was called in a top
level ``CMakeLists.txt`` file.

To obtain the value from the most recent call to :command:`project` in
the current directory scope or above, see the
:variable:`PROJECT_IS_TOP_LEVEL` variable.

The variable value will be true in:

* the top-level directory of the project
* the top-level directory of an external project added by :module:`ExternalProject`

The variable value will be false in:

* a directory added by :command:`add_subdirectory`
* a directory added by :module:`FetchContent`
