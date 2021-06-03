include("${RunCMake_SOURCE_DIR}/TestVariable.cmake")

test_variable("CMAKE_TOOLCHAIN_FILE" "FILEPATH" "${CMAKE_SOURCE_DIR}/override_toolchain.cmake")
