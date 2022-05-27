mingw-compiler-PATH
-------------------

* The :generator:`MSYS Makefiles` and :generator:`MinGW Makefiles`
  generators, when a compiler is not explicitly specified, now select
  the first compiler (of any name) found in directories listed by the
  ``PATH`` environment variable.
