set(CMAKE_EXPERIMENTAL_EXPORT_PACKAGE_DEPENDENCIES "1942b4fa-b2c5-4546-9385-83f254070067")
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

enable_language(CXX)

find_package(HasDeps)

# replicates FetchContent where a dependency is brought
# in via source. In these cases we need to extend the `install`
# `export` commands to allow markup on what `Find<Project>` will
# map to the export set
add_library(my_private_lib STATIC empty.cpp)
target_link_libraries(my_private_lib PUBLIC HasDeps::A)
set_target_properties(my_private_lib PROPERTIES EXPORT_FIND_PACKAGE_NAME "MyPrivate")

install(TARGETS my_private_lib EXPORT my_private_targets)
install(EXPORT my_private_targets
        FILE my_private.cmake
        DESTINATION lib)
export(EXPORT my_private_targets EXPORT_PACKAGE_DEPENDENCIES FILE my_private_targets.cmake)

add_library(my_static_lib STATIC empty.cpp)
target_link_libraries(my_static_lib PRIVATE my_private_lib)

install(TARGETS my_static_lib EXPORT my_static_targets)
install(EXPORT my_static_targets
        FILE my_static.cmake
        DESTINATION lib)
export(EXPORT my_static_targets EXPORT_PACKAGE_DEPENDENCIES FILE my_static_targets.cmake)

add_library(my_shared_lib SHARED empty.cpp)
target_link_libraries(my_shared_lib PUBLIC my_private_lib)

install(TARGETS my_shared_lib EXPORT my_shared_targets)
install(EXPORT my_shared_targets
        FILE my_shared.cmake
        DESTINATION lib)
export(EXPORT my_shared_targets EXPORT_PACKAGE_DEPENDENCIES FILE my_shared_targets.cmake)
