per_config(properties
  # property                      expected  alias
  # Usage requirement properties
  "MAP_IMPORTED_CONFIG_"          "Release" "<UNSET>"
  )

prepare_target_types(normal
           EXECUTABLE          INTERFACE          MODULE          OBJECT          SHARED          STATIC
  IMPORTED_EXECUTABLE IMPORTED_INTERFACE IMPORTED_MODULE IMPORTED_OBJECT IMPORTED_SHARED IMPORTED_STATIC)
run_property_tests(normal properties)
