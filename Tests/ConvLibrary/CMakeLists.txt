cmake_minimum_required (VERSION 2.6)
project(ConvLibrary)

# create a source list
set(foo_sources foo.cxx bar.c sub1/car.cxx) 
# create a library foo from the sources
add_library(foo ${foo_sources})
# get the object files from the target
get_target_property(OBJECT_FILES foo OBJECT_FILES)
message("${OBJECT_FILES}")
# set the object files as generated
set_source_files_properties(${OBJECT_FILES} PROPERTIES GENERATED true)
# create a library bar that contains the object files from foo
add_library(bar ${OBJECT_FILES})
# set the linker language since bar only has .obj
set_target_properties(bar PROPERTIES LINKER_LANGUAGE CXX)
# make sure foo is built before bar
add_dependencies(bar foo)
add_executable(bartest bartest.cxx)
target_link_libraries(bartest bar)
