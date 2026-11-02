project(UnknownDependencies LANGUAGES NONE)

add_library(root INTERFACE)
add_library(Unknown::dep UNKNOWN IMPORTED)
set_target_properties(Unknown::dep PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/unknown.a"
  EXPORT_FIND_PACKAGE_NAME Unknown
)
target_link_libraries(root INTERFACE
  Unknown::dep bare_library Unknown::dep bare_library)

install(TARGETS root EXPORT unknown_dependencies DESTINATION .)

foreach(dependency_package IN ITEMS First Second)
  if(dependency_package STREQUAL "First")
    set(dependency_license MIT)
  else()
    set(dependency_license Apache-2.0)
  endif()
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/SameNamedImportedTargets
    ${dependency_package})
endforeach()
