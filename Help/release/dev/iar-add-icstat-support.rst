iar-add-icstat-support
----------------------

* The :variable:`CMAKE_<LANG>_ICSTAT` variable and corresponding
  :prop_tgt:`<LANG>_ICSTAT` target property were added to tell
  the :ref:`Makefile Generators` and the :ref:`Ninja Generators`
  to run the IAR ``icstat`` tool along with the compiler for
  ``C`` and ``CXX`` languages.
