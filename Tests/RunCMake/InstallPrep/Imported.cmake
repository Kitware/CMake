enable_language(C)

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

add_library(unit_tests STATIC neverbuild.c)
