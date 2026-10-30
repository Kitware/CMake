set(dir "${CMAKE_CURRENT_BINARY_DIR}")

set(properties
  # property                      expected  alias
  # Linking properties
  "DLL_NAME_WITH_SOVERSION"       "OFF"     "<SAME>"
  )

prepare_target_types(shared_library
           SHARED
  IMPORTED_SHARED)
run_property_tests(shared_library properties)

set(properties
  # property                      expected  alias
  # Linking properties
  ## Platforms
  ### AIX
  # Imported targets must set `AIX_SHARED_LIBRARY_ARCHIVE` and
  # `AIX_SHARED_LIBRARY_SYMLINKS`    explicitly.
  "AIX_SHARED_LIBRARY_ARCHIVE"    "OFF"     "<SAME>"
  "AIX_SHARED_LIBRARY_SYMLINKS"   "OFF"      "<SAME>"
  )

prepare_target_types(non_imported_shared_library
  SHARED)
set(iteration "-AIX_SHARED_LIBRARY_ARCHIVE")
run_property_tests(non_imported_shared_library properties)
