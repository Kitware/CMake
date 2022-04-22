file(READ "${RunCMake_TEST_BINARY_DIR}/cmake_install.cmake" install_script)

if (install_script MATCHES [[\(CMAKE_INSTALL_COMPONENT STREQUAL "bmi" OR NOT CMAKE_INSTALL_COMPONENT\)]])
  list(APPEND RunCMake_TEST_FAILED
    "Found BMI install script component for `bmi`")
endif ()

if (install_script MATCHES [[include\("[^)]*/CMakeFiles/install-bmi-ignore\.dir/install-cxx-module-bmi-[^.]*\.cmake" OPTIONAL\)]])
  list(APPEND RunCMake_TEST_FAILED
    "Found BMI install script inclusion")
endif ()

string(REPLACE ";" "; " RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}")
