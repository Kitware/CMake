include(CMakeDependentOption)

set(A1 1)
set(bb 1)
cmake_dependent_option(USE_FOO "Use Foo" ON "A1;bb" OFF)
message(STATUS "USE_FOO='${USE_FOO}'")
