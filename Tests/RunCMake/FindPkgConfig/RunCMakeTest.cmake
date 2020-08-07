include(RunCMake)

# Isolate test cases from caller's environment.
set(ENV{CMAKE_PREFIX_PATH} "")
set(ENV{CMAKE_APPBUNDLE_PATH} "")
set(ENV{CMAKE_FRAMEWORK_PATH} "")

run_cmake(FindPkgConfig_NO_PKGCONFIG_PATH)
run_cmake(FindPkgConfig_PKGCONFIG_PATH)
run_cmake(FindPkgConfig_PKGCONFIG_PATH_NO_CMAKE_PATH)
run_cmake(FindPkgConfig_PKGCONFIG_PATH_NO_CMAKE_ENVIRONMENT_PATH)

if(APPLE)
  run_cmake(FindPkgConfig_CMAKE_FRAMEWORK_PATH)
  run_cmake(FindPkgConfig_CMAKE_APPBUNDLE_PATH)
endif()

# We need a real pkg-config to run the test for get_variable.
find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
  run_cmake(FindPkgConfig_GET_VARIABLE)
  run_cmake(FindPkgConfig_GET_VARIABLE_PREFIX_PATH)
  run_cmake(FindPkgConfig_GET_VARIABLE_PKGCONFIG_PATH)
  run_cmake(FindPkgConfig_cache_variables)
  run_cmake(FindPkgConfig_IMPORTED_TARGET)
  run_cmake(FindPkgConfig_VERSION_OPERATORS)
  run_cmake(FindPkgConfig_GET_MATCHING_MODULE_NAME)
endif ()
