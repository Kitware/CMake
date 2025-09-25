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

# Parameter testing
find_package(ParameterCheckConfig HINTS "${CMAKE_CURRENT_LIST_DIR}")
unset(ParameterCheckConfig_DIR CACHE)
find_package(ParameterCheckConfig PATH_SUFFIXES suffix1 suffix2)
unset(ParameterCheckConfig_DIR CACHE)
find_package(ParameterCheckConfig NO_DEFAULT_PATH NO_PACKAGE_ROOT_PATH NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_PACKAGE_REGISTRY NO_CMAKE_SYSTEM_PATH NO_CMAKE_INSTALL_PREFIX NO_CMAKE_SYSTEM_PACKAGE_REGISTRY)
unset(ParameterCheckConfig_DIR CACHE)
find_package(ParameterCheckConfig CMAKE_FIND_ROOT_PATH_BOTH)
unset(ParameterCheckConfig_DIR CACHE)
find_package(ParameterCheckConfig ONLY_CMAKE_FIND_ROOT_PATH)
unset(ParameterCheckConfig_DIR CACHE)
find_package(ParameterCheckConfig NO_CMAKE_FIND_ROOT_PATH)

find_package(OtherParameterCheck NAMES BogusParameterCheck ParameterCheckConfig)
