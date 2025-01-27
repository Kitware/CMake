linker-configuration
--------------------

* The :variable:`CMAKE_<LANG>_USING_LINKER_MODE` variable is no longer used to
  determine the type of the contents of the
  :variable:`CMAKE_<LANG>_USING_LINKER_<TYPE>` variable. The
  :variable:`CMAKE_<LANG>_LINK_MODE` variable, set by CMake, is used instead.
