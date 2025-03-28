
enable_language(C)

add_library(lib1 STATIC lib1.c)
set_property(TARGET lib1 PROPERTY ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")

# This function simulates a find_package call for the third-party lib
# by making an imported target with non-global scope.
function(find_package_lib1)
  add_library(lib1::lib1 STATIC IMPORTED)

  set_target_properties(lib1::lib1 PROPERTIES
    IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/Release/${CMAKE_STATIC_LIBRARY_PREFIX}lib1${CMAKE_STATIC_LIBRARY_SUFFIX}"
  )

  add_dependencies(lib1::lib1 lib1)
endfunction()

# ------------------------------------------------------------------------------
add_subdirectory(subdir1)
add_subdirectory(subdir2)
