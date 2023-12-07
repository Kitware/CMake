set(dir "${CMAKE_CURRENT_BINARY_DIR}")

per_config(properties
  # property                      expected  alias
  # Compilation properties
  "INTERPROCEDURAL_OPTIMIZATION_" "OFF"     "<UNSET>"

  # Output location properties
  "ARCHIVE_OUTPUT_DIRECTORY_"     "${dir}"  "<UNSET>"
  "COMPILE_PDB_OUTPUT_DIRECTORY_" "${dir}"  "<UNSET>"
  "LIBRARY_OUTPUT_DIRECTORY_"     "${dir}"  "<UNSET>"
  "PDB_OUTPUT_DIRECTORY_"         "${dir}"  "<UNSET>"
  "RUNTIME_OUTPUT_DIRECTORY_"     "${dir}"  "<UNSET>"
  )

prepare_target_types(with_artifact
           EXECUTABLE          MODULE          SHARED          STATIC
  IMPORTED_EXECUTABLE IMPORTED_MODULE IMPORTED_SHARED IMPORTED_STATIC)
run_property_tests(with_artifact properties)
