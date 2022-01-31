
enable_language(C)

# Create imported target NS::lib
add_library(NS::lib STATIC IMPORTED)
set_target_properties(NS::lib PROPERTIES
  IMPORTED_LOCATION "/path/to/lib"
  IMPORTED_IMPLIB "/path/to/import.lib"
)

# Create imported target NS::lib2
add_library(NS::lib2 SHARED IMPORTED)

set_target_properties(NS::lib2 PROPERTIES
  IMPORTED_LOCATION "/path/to/lib"
  IMPORTED_IMPLIB "/path/to/import.lib"
  INTERFACE_LINK_LIBRARIES "$<LINK_GROUP:feat,NS::lib>"
)


add_library(lib SHARED lib.c)
target_link_libraries(lib PRIVATE NS::lib2)
