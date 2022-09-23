include(CMakeDependentOption)

cmake_dependent_option(USE_FOO "Use Foo" ON "CMAKE_VERSION VERSION_GREATER_EQUAL 3.08" OFF)
message(STATUS "USE_FOO='${USE_FOO}'")
