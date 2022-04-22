file(READ "${RunCMake_TEST_BINARY_DIR}/cmake_install.cmake" install_script)

if (NOT install_script MATCHES [[include\("[^)]*/CMakeFiles/install-bmi-generic-args\.dir/install-cxx-module-bmi-[^.]*\.cmake" OPTIONAL\)]])
  list(APPEND RunCMake_TEST_FAILED
    "Could not find BMI install script inclusion")
endif ()

string(REPLACE ";" "; " RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}")
