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
