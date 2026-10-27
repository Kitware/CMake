per_config(properties
  # property              expected  alias
  # Linking properties
  "_POSTFIX"              "test"    "<SAME>"
  )

prepare_target_types(library_with_artifact
           MODULE          OBJECT          SHARED          STATIC
  IMPORTED_MODULE IMPORTED_OBJECT IMPORTED_SHARED IMPORTED_STATIC)
run_property_tests(library_with_artifact properties)
