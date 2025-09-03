include(Setup.cmake)

# Verify that MAP_IMPORTED_CONFIG_${CMAKE_BUILD_TYPE} can be used to select a
# configuration of a target imported via CPS

find_package(ConfigMapped REQUIRED)
set_target_properties(
  ConfigMapped::target PROPERTIES
  MAP_IMPORTED_CONFIG_RELEASE test
)

add_library(config-test STATIC config_test.cxx)
target_compile_definitions(config-test PRIVATE EXPECTED_MARKER=4)
target_link_libraries(config-test PUBLIC ConfigMapped::target)
