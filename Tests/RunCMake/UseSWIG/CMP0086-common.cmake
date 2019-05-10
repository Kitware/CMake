
cmake_policy(SET CMP0078 NEW)

find_package(Python REQUIRED COMPONENTS Development)
find_package (SWIG REQUIRED)
include(UseSWIG)

set_property (SOURCE example.i PROPERTY SWIG_MODULE_NAME "new_example")

swig_add_library(example LANGUAGE python TYPE MODULE SOURCES example.i)
target_link_libraries(example PRIVATE Python::Module)
