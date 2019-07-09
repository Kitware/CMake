add_cmake_find_use_package_registry
-----------------------------------

* The :command:`find_package` command has learned to check the following
  variables to control searching

  * :variable:`CMAKE_FIND_USE_PACKAGE_REGISTRY` - Controls the searching the
    cmake user registry.

* The :variable:`CMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY` has been deprecated.
  Instead use :variable:`CMAKE_FIND_USE_PACKAGE_REGISTRY`
