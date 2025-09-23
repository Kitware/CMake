cmake_minimum_required(VERSION 3.10)
project (EmptyProperty)

set_property(DIRECTORY APPEND
    PROPERTY
        COMPILE_DEFINITIONS)

include(CTest)

add_executable(EmptyProperty EmptyProperty.cxx)
