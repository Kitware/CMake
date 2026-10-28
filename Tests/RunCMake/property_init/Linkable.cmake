per_config(properties
  # property                        expected  alias
  # Linking properties
  ## Platforms
  ### macOS
  "FRAMEWORK_MULTI_CONFIG_POSTFIX_" ".fw"     "<SAME>"
  )

prepare_target_types(linkable
           SHARED          STATIC
  IMPORTED_SHARED IMPORTED_STATIC)
run_property_tests(linkable properties)
