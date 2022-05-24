include(CTest)
add_test(NAME cmake_version COMMAND "${CMAKE_COMMAND}" --version)

set_property(TEST cmake_version
  PROPERTY ENVIRONMENT_MODIFICATION
    INVALID_OP=unknown:)
