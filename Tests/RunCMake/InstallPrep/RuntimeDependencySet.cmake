enable_language(C)

add_library(runtime_dependency_shared SHARED EXCLUDE_FROM_ALL lib.c)
add_library(runtime_dependency_module MODULE EXCLUDE_FROM_ALL lib.c)

add_library(runtime_dependency_support SHARED EXCLUDE_FROM_ALL lib.c)
set_target_properties(runtime_dependency_support PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}")
add_executable(runtime_dependency_producer EXCLUDE_FROM_ALL runtime.c)
target_link_libraries(runtime_dependency_producer PRIVATE runtime_dependency_support)
set_target_properties(runtime_dependency_producer PROPERTIES
  OUTPUT_NAME runtime-dependency-imported
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
)

set(imported_location
  "${CMAKE_BINARY_DIR}/runtime-dependency-imported${CMAKE_EXECUTABLE_SUFFIX}")
add_executable(runtime_dependency_imported IMPORTED)
set_target_properties(runtime_dependency_imported PROPERTIES
  IMPORTED_LOCATION "${imported_location}"
)
add_dependencies(runtime_dependency_imported runtime_dependency_producer)

install(TARGETS runtime_dependency_shared runtime_dependency_module
  RUNTIME_DEPENDENCY_SET runtime_dependency_set
  RUNTIME DESTINATION bin
    COMPONENT RuntimeDependencySeeds
  LIBRARY DESTINATION lib
    COMPONENT RuntimeDependencySeeds
)
install(IMPORTED_RUNTIME_ARTIFACTS runtime_dependency_imported
  RUNTIME_DEPENDENCY_SET runtime_dependency_set
  RUNTIME DESTINATION bin
    COMPONENT RuntimeDependencySeeds
)
install(RUNTIME_DEPENDENCY_SET runtime_dependency_set
  PRE_INCLUDE_REGEXES "runtime_dependency_support"
  PRE_EXCLUDE_REGEXES ".*"
  RUNTIME DESTINATION bin
    COMPONENT RuntimeDependencySet
  LIBRARY DESTINATION lib
    COMPONENT RuntimeDependencySet
  FRAMEWORK DESTINATION Frameworks
    COMPONENT RuntimeDependencySet
)

add_library(unit_tests STATIC neverbuild.c)
