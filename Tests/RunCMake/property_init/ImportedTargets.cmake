set(properties
  # property        expected  alias
  "SYSTEM"          "ON"      "<SAME>"
  )

prepare_target_types(imported
  IMPORTED_EXECUTABLE IMPORTED_INTERFACE IMPORTED_MODULE IMPORTED_OBJECT IMPORTED_SHARED IMPORTED_STATIC)
set(with_defaults 1)
run_property_tests(imported properties)
