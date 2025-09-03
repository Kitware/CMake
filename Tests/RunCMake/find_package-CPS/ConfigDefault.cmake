include(Setup.cmake)

# Verify that CMake respects the preferred configurations as specified by a
# CPS package

find_package(ConfigDefault REQUIRED)
add_library(config-test config_test.cxx)
target_compile_definitions(config-test PRIVATE EXPECTED_MARKER=2)
target_link_libraries(config-test ConfigDefault::target)
