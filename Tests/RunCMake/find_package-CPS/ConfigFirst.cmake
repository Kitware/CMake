include(Setup.cmake)

# Verify that the first observed configuration of a component is selected when
# nothing else influences configuration selection

find_package(ConfigFirst REQUIRED)
add_library(config-test config_test.cxx)
target_compile_definitions(config-test PRIVATE EXPECTED_MARKER=1)
target_link_libraries(config-test ConfigFirst::target)
