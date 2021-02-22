include("${RunCMake_SOURCE_DIR}/TestVariable.cmake")

test_variable("CMAKE_INSTALL_PREFIX" "PATH" "${RunCMake_SOURCE_DIR}/path/passed/on/command_line")
