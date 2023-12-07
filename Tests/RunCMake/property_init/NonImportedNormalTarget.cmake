set(properties
  # property                      expected  alias
  # Linking properties
  "LINK_LIBRARIES_ONLY_TARGETS"   "OFF"     "<SAME>"
  )

prepare_target_types(normal_non_imported
  EXECUTABLE SHARED STATIC MODULE OBJECT INTERFACE)
run_property_tests(normal_non_imported properties)
