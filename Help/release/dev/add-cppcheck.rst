add-cppcheck
------------

* A :prop_tgt:`<LANG>_CPPCHECK` target property and supporting
  :variable:`CMAKE_<LANG>_CPPCHECK` variable were introduced to tell
  the :ref:`Makefile Generators` and the :generator:`Ninja` generator to
  run ``cppcheck`` with the compiler for ``C`` and ``CXX`` languages.
