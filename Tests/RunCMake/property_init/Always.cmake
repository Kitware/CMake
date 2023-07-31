set(properties
  # property                      expected  alias
  # Test a property which should never be initialized.
  "notset"                        "<UNSET>" "<SAME>"

  # Build graph properties
  "VERIFY_INTERFACE_HEADER_SETS"  "TRUE"    "<SAME>"

  # Metadata
  "FOLDER"                        "folder"  "<SAME>"
  )

prepare_target_types(always ${all_target_types})

run_property_tests(always properties)
