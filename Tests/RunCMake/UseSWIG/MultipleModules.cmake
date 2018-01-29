
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

find_package(PythonLibs REQUIRED)
find_package(PerlLibs REQUIRED)

unset(CMAKE_SWIG_FLAGS)

set (CMAKE_INCLUDE_CURRENT_DIR ON)

set_property(SOURCE example.i PROPERTY CPLUSPLUS ON)
set_property(SOURCE example.i PROPERTY COMPILE_OPTIONS -includeall)

set_property(SOURCE example.i PROPERTY GENERATED_INCLUDE_DIRECTORIES ${PYTHON_INCLUDE_PATH})

swig_add_library(example1
                 LANGUAGE python
                 SOURCES example.i example.cxx)
target_link_libraries(example1 PRIVATE ${PYTHON_LIBRARIES})

# re-use sample interface file for another plugin
set_property(SOURCE example.i PROPERTY GENERATED_INCLUDE_DIRECTORIES ${PERL_INCLUDE_PATH})
separate_arguments(c_flags UNIX_COMMAND "${PERL_EXTRA_C_FLAGS}")
set_property(SOURCE example.i PROPERTY GENERATED_COMPILE_OPTIONS ${c_flags})

swig_add_library(example2
                 LANGUAGE perl
                 SOURCES example.i example.cxx)
target_link_libraries(example2 PRIVATE ${PERL_LIBRARY})
