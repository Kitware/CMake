enable_language(C)

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME DefaultComponent)

add_library(runtime_helper STATIC lib.c)
add_executable(runtime EXCLUDE_FROM_ALL runtime.c)
target_link_libraries(runtime PRIVATE runtime_helper)
install(TARGETS runtime
  RUNTIME DESTINATION bin
  COMPONENT Runtime
)

add_library(default_component STATIC lib.c)
install(TARGETS default_component
  ARCHIVE DESTINATION lib
)

add_library(hidden STATIC neverbuild.c)
install(TARGETS hidden
  ARCHIVE DESTINATION lib
  COMPONENT Hidden
  EXCLUDE_FROM_ALL
)

add_executable(imported_producer EXCLUDE_FROM_ALL runtime_main.c)
set_target_properties(imported_producer PROPERTIES
  OUTPUT_NAME imported-tool
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
)

set(imported_location "${CMAKE_BINARY_DIR}/imported-tool${CMAKE_EXECUTABLE_SUFFIX}")
add_executable(imported_tool IMPORTED)
set_target_properties(imported_tool PROPERTIES
  IMPORTED_LOCATION "${imported_location}"
)
add_dependencies(imported_tool imported_producer)

install(IMPORTED_RUNTIME_ARTIFACTS imported_tool
  RUNTIME DESTINATION bin
  COMPONENT Imported
)

install(FILES lib.c
  DESTINATION share/install-prep
  COMPONENT Documentation
)

install(FILES lib.c
  DESTINATION share/install-prep
  COMPONENT "Unsafe/../Runtime"
  EXCLUDE_FROM_ALL
)

string(ASCII 9 component_tab)
install(FILES lib.c
  DESTINATION share/install-prep
  COMPONENT "Unsafe${component_tab}Component"
  EXCLUDE_FROM_ALL
)

add_library(unit_tests STATIC neverbuild.c)

add_subdirectory(subdir)
add_subdirectory(excluded EXCLUDE_FROM_ALL)
