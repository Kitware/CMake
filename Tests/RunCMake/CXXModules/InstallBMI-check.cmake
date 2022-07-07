file(READ "${RunCMake_TEST_BINARY_DIR}/cmake_install.cmake" install_script)

if (NOT install_script MATCHES [[\(CMAKE_INSTALL_COMPONENT STREQUAL "bmi" OR NOT CMAKE_INSTALL_COMPONENT\)]])
  list(APPEND RunCMake_TEST_FAILED
    "Could not find BMI install script component for `bmi`")
endif ()

if (NOT install_script MATCHES [[include\("[^)]*/CMakeFiles/install-bmi\.dir/install-cxx-module-bmi-[^.]*\.cmake" OPTIONAL\)]])
  list(APPEND RunCMake_TEST_FAILED
    "Could not find BMI install script inclusion")
endif ()

if (NOT install_script MATCHES [[\(CMAKE_INSTALL_COMPONENT STREQUAL "bmi-optional"\)]])
  list(APPEND RunCMake_TEST_FAILED
    "Could not find BMI install script component for `bmi-optional`")
endif ()

if (NOT install_script MATCHES [[\(CMAKE_INSTALL_COMPONENT STREQUAL "bmi-only-debug" OR NOT CMAKE_INSTALL_COMPONENT\)
  if\(CMAKE_INSTALL_CONFIG_NAME MATCHES "\^\(\[Dd\]\[Ee\]\[Bb\]\[Uu\]\[Gg\]\)\$"\)]])
  list(APPEND RunCMake_TEST_FAILED
    "Could not find BMI install script component for `bmi-only-debug`")
endif ()

string(REPLACE ";" "; " RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}")
