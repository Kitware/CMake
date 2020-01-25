project(subproject_with_object_lib)

add_library(object_lib_dependency OBJECT dummy.cpp)

add_library(shared_lib SHARED dummy.cpp)
target_sources(shared_lib PRIVATE $<TARGET_OBJECTS:object_lib_dependency>)
set_target_properties(shared_lib PROPERTIES MACOSX_RPATH ON)
