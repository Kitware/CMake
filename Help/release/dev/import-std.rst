import-std
----------

* The experimental gate for ``import std`` has been dropped. ``import std`` is
  available for all C++20 and later code with :prop_tgt:`CXX_SCAN_FOR_MODULES`
  enabled.

* Variable :variable:`CMAKE_CXX_STDLIB_MODULES_JSON` sets a specific module
  metadata file which CMake will use for constructing ``import std`` instead
  of relying on automatic detection via the current C++ compiler.
