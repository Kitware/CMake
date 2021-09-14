include(CMakeDependentOption)

cmake_policy(SET CMP0127 NEW)

set(FOO "lower")
cmake_dependent_option(USE_FOO "Use Foo" ON "FOO MATCHES \"(UPPER|lower)\"" OFF)
message(STATUS "USE_FOO='${USE_FOO}'")
