
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

find_package(Python2 REQUIRED COMPONENTS Development)
find_package(Python3 REQUIRED COMPONENTS Development)

unset(CMAKE_SWIG_FLAGS)

set (CMAKE_INCLUDE_CURRENT_DIR ON)

set_property(SOURCE example.i PROPERTY CPLUSPLUS ON)
set_property(SOURCE example.i PROPERTY COMPILE_OPTIONS -includeall)

swig_add_library(example1
                 LANGUAGE python
                 OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/Python2"
                 SOURCES example.i example.cxx)
target_link_libraries(example1 PRIVATE Python2::Python)

# re-use sample interface file for another plugin
swig_add_library(example2
                 LANGUAGE python
                 OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/Python3"
                 SOURCES example.i example.cxx)
target_link_libraries(example2 PRIVATE Python3::Python)
