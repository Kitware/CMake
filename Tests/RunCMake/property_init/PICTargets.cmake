set(properties
  # property                                expected            alias
  # Compilation properties
  "POSITION_INDEPENDENT_CODE"               "True"              "<SAME>"
  )

prepare_target_types(pic_targets
           EXECUTABLE          MODULE          OBJECT          SHARED          STATIC
                      IMPORTED_MODULE                 IMPORTED_SHARED)
run_property_tests(pic_targets properties)

set(APPEND properties_with_defaults
  # property                      expected  alias
  "POSITION_INDEPENDENT_CODE"     "True"    "<SAME>"
  )

prepare_target_types(pic_default_targets
           MODULE          SHARED
  IMPORTED_MODULE IMPORTED_SHARED)
set(with_defaults 1)
run_property_tests(pic_default_targets properties_with_defaults)
