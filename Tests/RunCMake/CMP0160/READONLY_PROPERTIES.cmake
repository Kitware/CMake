
set(read_only_properties
  "HEADER_SETS"
  "INTERFACE_HEADER_SETS"
  "MANUALLY_ADDED_DEPENDENCIES"
  "NAME"
  "TYPE"
  )
set(read_only_properties_imported
  "EXPORT_NAME"
  "SOURCES"
  )
set(read_only_properties_nonimported
  "IMPORTED_GLOBAL"
  )
set(read_only_properties_160
    "ALIAS_GLOBAL"
    "BINARY_DIR"
    "CXX_MODULE_SETS"
    "IMPORTED"
    "INTERFACE_CXX_MODULE_SETS"
    "LOCATION"
    "LOCATION_CONFIG"
    "SOURCE_DIR"
    )

cmake_policy(GET CMP0160 policy160)
add_library(ReadOnlyLib )
add_library(ReadOnlyImport IMPORTED UNKNOWN)

foreach(target ReadOnlyLib ReadOnlyImport)
  get_target_property(is_imported ${target} IMPORTED)
  set(are_read_only ${read_only_properties})
  if(NOT policy160 STREQUAL "OLD")
    list(APPEND are_read_only ${read_only_properties_160})
  endif()
  if(is_imported)
    list(APPEND are_read_only ${read_only_properties_imported})
  else()
    list(APPEND are_read_only ${read_only_properties_nonimported})
  endif()

  foreach(prop IN LISTS are_read_only)
    set_target_properties(${target} PROPERTIES ${prop} "a_value")
  endforeach()

  if(policy160 STREQUAL "OLD")
    foreach(prop IN LISTS read_only_properties_160)
      set_target_properties(${target} PROPERTIES ${prop} "a_value")
    endforeach()
  endif()
endforeach()
