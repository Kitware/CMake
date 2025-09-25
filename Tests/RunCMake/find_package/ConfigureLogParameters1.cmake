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
find_package(ParameterCheck 1.0 EXACT)
find_package(ParameterCheck 1.0...1.5)
find_package(ParameterCheck 1.2)
find_package(ParameterCheck QUIET)
find_package(ParameterCheck REQUIRED)
find_package(ParameterCheck COMPONENTS component OPTIONAL_COMPONENTS opt_component)
find_package(ParameterCheck REGISTRY_VIEW HOST)
find_package(ParameterCheck GLOBAL)
find_package(ParameterCheck NO_POLICY_SCOPE)
find_package(ParameterCheck BYPASS_PROVIDER)
