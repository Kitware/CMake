add_library(implgather INTERFACE)

add_library(dep STATIC dep.c)

add_library(deps INTERFACE)
target_link_libraries(deps INTERFACE dep)

add_library(impl_obj OBJECT impl_obj.c)
target_link_libraries(impl_obj PUBLIC deps)

target_sources(implgather INTERFACE "$<TARGET_OBJECTS:impl_obj>")
target_link_libraries(implgather INTERFACE impl_obj)

add_executable(useimpl main.c)
target_link_libraries(useimpl PRIVATE implgather)
