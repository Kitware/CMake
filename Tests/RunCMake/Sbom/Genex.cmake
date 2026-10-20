project(Genex C)
add_library(foo INTERFACE)

find_package(
  bar 1.3.4 REQUIRED
  NO_DEFAULT_PATH
  PATHS ${CMAKE_CURRENT_LIST_DIR}
)

find_package(
  baz 1.3.4 REQUIRED
  NO_DEFAULT_PATH
  PATHS ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(foo INTERFACE $<$<CONFIG:BarConfig>:bar::bar>)
target_link_libraries(foo INTERFACE $<$<CONFIG:BazConfig>:baz>)

add_library(resolved_dependency INTERFACE)
add_library(resolved_alias ALIAS resolved_dependency)
add_library(build_dependency INTERFACE)
add_library(install_dependency INTERFACE)
add_library(compile_dependency INTERFACE)
add_library(compile_leaf INTERFACE)
target_link_libraries(compile_dependency INTERFACE compile_leaf)
set_property(TARGET compile_dependency PROPERTY SPDX_LICENSE MIT)
set_property(TARGET compile_leaf PROPERTY SPDX_LICENSE Zlib)
target_link_libraries(foo INTERFACE
  $<IF:$<BOOL:1>,$<TARGET_NAME_IF_EXISTS:resolved_alias>,discarded_library>
  $<TARGET_NAME_IF_EXISTS:missing_target>
  $<LINK_ONLY:$<BUILD_INTERFACE:build_dependency>>
  $<LINK_ONLY:$<INSTALL_INTERFACE:install_dependency>>
  $<COMPILE_ONLY:compile_dependency>
  $<IF:$<BOOL:1>,bare_library,discarded_library>
)

set(CMAKE_C_LINK_LIBRARY_USING_WHOLE_ARCHIVE <LINK_ITEM>)
set(CMAKE_C_LINK_LIBRARY_USING_WHOLE_ARCHIVE_SUPPORTED TRUE)
set(CMAKE_C_LINK_GROUP_USING_RESCAN "" "")
set(CMAKE_C_LINK_GROUP_USING_RESCAN_SUPPORTED TRUE)
add_library(dep1 STATIC ${CMAKE_CURRENT_LIST_DIR}/test.c)
add_library(dep2 STATIC ${CMAKE_CURRENT_LIST_DIR}/test.c)
add_library(runtime_dep SHARED ${CMAKE_CURRENT_LIST_DIR}/test.c)
install(TARGETS foo dep1 dep2 runtime_dep EXPORT foo)
foreach(type IN ITEMS STATIC SHARED)
  foreach(feature IN ITEMS plain whole group)
    add_library(${feature}_${type} ${type} ${CMAKE_CURRENT_LIST_DIR}/test.c)
    if(feature STREQUAL "plain")
      target_link_libraries(${feature}_${type} PRIVATE dep1 dep2)
    elseif(feature STREQUAL "whole")
      target_link_libraries(${feature}_${type} PRIVATE
        $<LINK_LIBRARY:WHOLE_ARCHIVE,dep1,dep2>)
    else()
      target_link_libraries(${feature}_${type} PRIVATE
        $<LINK_GROUP:RESCAN,dep1,dep2>)
    endif()
    target_link_libraries(${feature}_${type} PRIVATE runtime_dep bare_library)
    install(TARGETS ${feature}_${type} EXPORT foo)
  endforeach()
endforeach()
