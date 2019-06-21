global-controls-over-find-locations
-----------------------------------

* The :command:`find_file`, :command:`find_library`, :command:`find_path`,
  and :command:`find_program` commands have learned to check the following
  variables to control searching
  * :variable:`CMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH` - Controls the searching the cmake-specific environment variables.
  * :variable:`CMAKE_FIND_USE_CMAKE_PATH` - Controls the searching the cmake-specific cache variables.
  * :variable:`CMAKE_FIND_USE_CMAKE_SYSTEM_PATH` - Controls the searching cmake platform specific variables.
  * :variable:`CMAKE_FIND_USE_PACAKGE_ROOT_PATH` - Controls the searching of :variable:`<PackageName>_ROOT` variables.
  * :variable:`CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH` - Controls the searching the standard system environment variables.
