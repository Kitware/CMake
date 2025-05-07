list(INSERT CMAKE_MODULE_PATH 0
  "${CMAKE_CURRENT_LIST_DIR}/ConfigureLog/cmake")
list(INSERT CMAKE_PREFIX_PATH 0
  "${CMAKE_CURRENT_LIST_DIR}/ConfigureLog")

set(CMAKE_FIND_DEBUG_MODE 1)
# Stable sorting for predictable behaviors.
set(CMAKE_FIND_PACKAGE_SORT_ORDER NAME)

# Unset search variables for more predictable output.
unset(CMAKE_FRAMEWORK_PATH)
unset(CMAKE_APPBUNDLE_PATH)
unset(ENV{CMAKE_PREFIX_PATH})
unset(ENV{CMAKE_FRAMEWORK_PATH})
unset(ENV{CMAKE_APPBUNDLE_PATH})

# Find a config package
find_package(ViaConfig)

# Find a module
find_package(ViaModule)
