mingw-no-find_library-dll
-------------------------

* When building with GNU tools on Windows (MinGW tools), the
  :command:`find_library` command will no longer consider
  ``.dll`` files to be linkable libraries.  All dynamic link
  libraries are expected to provide separate ``.dll.a`` or
  ``.lib`` import libraries.
