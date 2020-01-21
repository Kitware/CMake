mingw-find-no-dll
-----------------

* When using MinGW tools, the :command:`find_library` command no longer
  finds ``.dll`` files by default.  Instead it expects ``.dll.a`` import
  libraries to be available.
