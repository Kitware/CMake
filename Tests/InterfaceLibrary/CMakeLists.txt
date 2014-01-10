
cmake_minimum_required(VERSION 2.8)

project(InterfaceLibrary)

add_library(iface_nodepends INTERFACE)
target_compile_definitions(iface_nodepends INTERFACE IFACE_DEFINE)

add_subdirectory(headerdir)

add_executable(InterfaceLibrary definetestexe.cpp)
target_link_libraries(InterfaceLibrary iface_nodepends headeriface)

add_subdirectory(libsdir)

add_executable(sharedlibtestexe sharedlibtestexe.cpp)
target_link_libraries(sharedlibtestexe shared_iface imported::iface)

add_library(broken EXCLUDE_FROM_ALL broken.cpp)

add_library(iface_broken INTERFACE)
# This is not a dependency, so broken will not be built (and the error in
# it will not be hit)
target_link_libraries(iface_broken INTERFACE broken)

add_library(iface_whitelist INTERFACE)
# The target property CUSTOM will never be evaluated on the INTERFACE library.
target_link_libraries(iface_whitelist INTERFACE $<$<BOOL:$<TARGET_PROPERTY:CUSTOM>>:irrelevant>)

add_executable(exec_whitelist dummy.cpp)
target_link_libraries(exec_whitelist iface_whitelist)

add_library(iface_imported INTERFACE IMPORTED)
set_property(TARGET iface_imported PROPERTY
  INTERFACE_COMPILE_DEFINITIONS
    $<$<CONFIG:SPECIAL>:SPECIAL_MODE>
    $<$<CONFIG:Debug>:DEBUG_MODE>
)
set_property(TARGET iface_imported PROPERTY
  MAP_IMPORTED_CONFIG_DEBUG SPECIAL
)

add_executable(map_config map_config.cpp)
target_link_libraries(map_config iface_imported)
