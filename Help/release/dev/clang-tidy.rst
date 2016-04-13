clang-tidy
----------

* A :prop_tgt:`<LANG>_CLANG_TIDY` target property and supporting
  :variable:`CMAKE_<LANG>_CLANG_TIDY` variable were introduced to tell the
  :ref:`Makefile Generators` and the :generator:`Ninja` generator to run
  ``clang-tidy`` along with the compiler for ``C`` and ``CXX`` languages.
