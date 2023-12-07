per_config(properties
  # property                        expected  alias
  # Linking properties
  ## Platforms
  ### macOS
  "FRAMEWORK_MULTI_CONFIG_POSTFIX_" ".fw"     "<UNSET>"
  )

prepare_target_types(linkable
           EXECUTABLE          SHARED          STATIC
  IMPORTED_EXECUTABLE IMPORTED_SHARED IMPORTED_STATIC)
run_property_tests(linkable properties)
