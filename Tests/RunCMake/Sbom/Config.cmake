project(Config C)
add_library(foo ${CMAKE_CURRENT_LIST_DIR}/test.c)

add_library(debug_dependency INTERFACE)
add_library(release_dependency INTERFACE)
add_library(Common::Common INTERFACE IMPORTED)
set_target_properties(Common::Common PROPERTIES
  EXPORT_FIND_PACKAGE_NAME Common
  SPDX_LICENSE MIT
)
set_target_properties(foo debug_dependency release_dependency PROPERTIES
  SPDX_LICENSE MIT
)
target_link_libraries(debug_dependency INTERFACE Common::Common)
target_link_libraries(release_dependency INTERFACE Common::Common)
target_link_libraries(foo PRIVATE
  $<IF:$<CONFIG:Debug>,debug_dependency,release_dependency>
  Common::Common
)

install(TARGETS foo EXPORT foo DESTINATION .)
