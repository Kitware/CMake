ninja-compiler-PATH-windows
---------------------------

* On Windows, the :generator:`Ninja` and :generator:`Ninja Multi-Config`
  generators, when a compiler is not explicitly specified, now select
  the first compiler (of any name) found in directories listed by the
  ``PATH`` environment variable.
