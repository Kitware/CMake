cmake_minimum_required(VERSION 3.16)
project(RccOffMocLibrary)
include("../AutogenCoreTest.cmake")

# Add not_generated_file.qrc to the source list to get the file-level
# dependency, but don't generate a c++ file from it.  Disable the AUTORCC
# feature for this target.  This tests that qrc files in the sources don't
# have an effect on generation if AUTORCC is off.
add_library(empty STATIC empty.cpp not_generated_file.qrc)
set_target_properties(empty PROPERTIES AUTORCC OFF)
set_target_properties(empty PROPERTIES AUTOMOC TRUE)
target_link_libraries(empty PRIVATE no_link_language ${QT_LIBRARIES})
add_library(no_link_language STATIC empty.h)
set_target_properties(no_link_language PROPERTIES AUTOMOC TRUE)
target_link_libraries(no_link_language PRIVATE ${QT_LIBRARIES})
