cxxmodules-custom-import-std-metadata-file
------------------------------------------

* The ``import std`` support learned to use the
  :variable:`CMAKE_CXX_STDLIB_MODULES_JSON` variable to set the path to the
  metadata file for the standard library rather than using the compiler to
  discover its location.
