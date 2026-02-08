import-std
----------
* The experimental gate for C++23's ``import std`` has been dropped.

* Variable :variable:`CMAKE_CXX_MODULE_STD` and property target
  :prop_tgt:`CXX_MODULE_STD` control the availability of ``import std`` for
  C++ targets.

* Read-only variable :variable:`CMAKE_CXX_COMPILER_IMPORT_STD` lists the C++
  standard levels for which ``import std`` is available on the current
  toolchain.

* Variable :variable:`CMAKE_CXX_STDLIB_MODULES_JSON` sets a specific module
  metadata file which CMake will use for constructing ``import std`` instead
  of relying on automatic detection via the current C++ compiler.
