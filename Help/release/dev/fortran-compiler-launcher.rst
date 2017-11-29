fortran-compiler-launcher
-------------------------

* The :ref:`Makefile Generators` and the :generator:`Ninja` generator learned
  to add compiler launcher tools along with the compiler for the ``Fortran``
  language (``C``, ``CXX``, and ``CUDA`` were supported previously).
  See the :variable:`CMAKE_<LANG>_COMPILER_LAUNCHER` variable and
  :prop_tgt:`<LANG>_COMPILER_LAUNCHER` target property for details.
