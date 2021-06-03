include("${RunCMake_SOURCE_DIR}/TestVariable.cmake")

test_variable("CMAKE_INSTALL_PREFIX" "PATH" "${CMAKE_SOURCE_DIR}/build/install_dir2")
