CMAKE_<LANG>_LIBRARY_PATH_FLAG
------------------------------

.. versionadded:: 3.16

The flag to be used to add a library search path to the linker
for the toolchain of language ``<LANG>``.

CMake sets this variable automatically for some toolchains during
toolchain inspection by calls to the :command:`project` or
:command:`enable_language` commands.

If the :variable:`!CMAKE_<LANG>_LIBRARY_PATH_FLAG` variable
is defined, it takes precedence over the language-agnostic
:variable:`CMAKE_LIBRARY_PATH_FLAG` variable.
