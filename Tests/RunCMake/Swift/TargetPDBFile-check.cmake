file(READ "${RunCMake_TEST_BINARY_DIR}/cmake_install.cmake" install_script)
if(NOT install_script MATCHES "SwiftPDBCustom\\.pdb")
  string(APPEND RunCMake_TEST_FAILED
    "Generated install script does not reference SwiftPDBCustom.pdb\n")
endif()
if(NOT install_script MATCHES "OPTIONAL")
  string(APPEND RunCMake_TEST_FAILED
    "Generated install script does not preserve OPTIONAL install\n")
endif()
