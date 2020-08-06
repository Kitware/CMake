compiler_flags
-----------------

* The :variable:`CMAKE_<LANG>_COMPILER` variable may now be used to
  store "mandatory" compiler flags like the :envvar:`CC` and other environment variables.

* The :variable:`CMAKE_<LANG>_FLAGS_INIT` variable will now be considered during
  the compiler indentification check if other sources like :variable:`CMAKE_<LANG>_FLAGS`
  or :envvar:`CFLAGS` are not set.
