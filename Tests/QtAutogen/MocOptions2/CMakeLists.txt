cmake_minimum_required(VERSION 3.16)
project(MocOptions2)
include("../AutogenCoreTest.cmake")

# Test extra options passed to moc via AUTOMOC_MOC_OPTIONS
add_executable(mocOptions object.cpp main.cpp)
set_property(TARGET mocOptions PROPERTY AUTOMOC ON)
target_compile_options(mocOptions PRIVATE "-D_EXTRA_DEFINE")
set_property(TARGET mocOptions PROPERTY AUTOMOC_MOC_OPTIONS "-D_EXTRA_DEFINE")
target_link_libraries(mocOptions ${QT_LIBRARIES})
