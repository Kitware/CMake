include(Setup.cmake)

# Verify that CMake will select a configuration matching the current build
# type, regardless of the package's preferred configurations

find_package(ConfigMatchBuildType REQUIRED)
add_library(config-test STATIC config_test.cxx)
target_compile_definitions(config-test PRIVATE EXPECTED_MARKER=4)
target_link_libraries(config-test ConfigMatchBuildType::target)
