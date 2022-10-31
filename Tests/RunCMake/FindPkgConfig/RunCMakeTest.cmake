include(RunCMake)

# Isolate test cases from caller's environment.
set(ENV{CMAKE_PREFIX_PATH} "")
set(ENV{CMAKE_APPBUNDLE_PATH} "")
set(ENV{CMAKE_FRAMEWORK_PATH} "")

run_cmake(PkgConfigDoesNotExist)

if(NOT WIN32)
  # FIXME: The Windows implementation of these tests do not work.
  #        They are disabled until they can be updated to a working state.
  run_cmake(FindPkgConfig_CMP0126_NEW)
  run_cmake(FindPkgConfig_NO_PKGCONFIG_PATH)
  run_cmake(FindPkgConfig_PKGCONFIG_PATH)
  run_cmake(FindPkgConfig_PKGCONFIG_PATH_NO_CMAKE_PATH)
  run_cmake(FindPkgConfig_PKGCONFIG_PATH_NO_CMAKE_ENVIRONMENT_PATH)
  run_cmake(FindPkgConfig_GET_MATCHING_ARGN)
endif()

run_cmake(FindPkgConfig_extract_frameworks)

if(APPLE)
  run_cmake(FindPkgConfig_extract_frameworks_target)
  run_cmake(FindPkgConfig_CMAKE_FRAMEWORK_PATH)
  run_cmake(FindPkgConfig_CMAKE_APPBUNDLE_PATH)
endif()

# We need a real pkg-config to run the test for get_variable.
find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
  string(FIND "${CMAKE_CURRENT_BINARY_DIR}" " " IS_SPACES_IN_PATH)
  if(IS_SPACES_IN_PATH GREATER -1)
    string(REPLACE " " "\\ " ESCAPED_ROOT "${CMAKE_CURRENT_BINARY_DIR}")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/test_spaces.pc" "
libdir=${ESCAPED_ROOT}
Name: test_spaces.pc
Version: 0.0
Description: test spaces
Libs: -L\${libdir}
")
    set(PKG_CONFIG_PATH_SAVED "$ENV{PKG_CONFIG_PATH}")
    set(ENV{PKG_CONFIG_PATH} "${CMAKE_CURRENT_BINARY_DIR}")
    execute_process(COMMAND "${PKG_CONFIG_EXECUTABLE}" --libs test_spaces
                    ERROR_QUIET COMMAND_ERROR_IS_FATAL ANY
                    OUTPUT_VARIABLE test_spaces_LIBS)
    set(ENV{PKG_CONFIG_PATH} "${PKG_CONFIG_PATH_SAVED}")
    string(STRIP "${test_spaces_LIBS}" test_spaces_LIBS_STRIPPED)
    if(NOT "${test_spaces_LIBS_STRIPPED}" STREQUAL "-L${ESCAPED_ROOT}")
      set(PKG_CONFIG_DONT_SUPPORT_SPACES_IN_PATH TRUE)
    endif()
  endif()
  run_cmake(FindPkgConfig_GET_VARIABLE)
  run_cmake(FindPkgConfig_GET_VARIABLE_PREFIX_PATH)
  run_cmake(FindPkgConfig_GET_VARIABLE_PKGCONFIG_PATH)
  run_cmake(FindPkgConfig_cache_variables)
  run_cmake(FindPkgConfig_IMPORTED_TARGET)
  run_cmake(FindPkgConfig_VERSION_OPERATORS)
  run_cmake(FindPkgConfig_GET_MATCHING_MODULE_NAME)
  run_cmake(FindPkgConfig_empty_target)
  if(NOT PKG_CONFIG_DONT_SUPPORT_SPACES_IN_PATH)
    run_cmake(FindPkgConfig_LIBRARY_PATH)
  endif()
endif ()
