compiler-launcher
-----------------

* The :ref:`Makefile Generators` and the :generator:`Ninja` generator
  learned to add compiler launcher tools like distcc and ccache along with the
  compiler for ``C`` and ``CXX`` languages.  See the
  :variable:`CMAKE_<LANG>_COMPILER_LAUNCHER` variable and
  :prop_tgt:`<LANG>_COMPILER_LAUNCHER` target property for details.
