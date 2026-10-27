set(dir "${CMAKE_CURRENT_BINARY_DIR}")

per_config(properties
  # property                      expected  alias
  # Compilation properties
  "INTERPROCEDURAL_OPTIMIZATION_" "OFF"     "<SAME>"

  # Output location properties
  "ARCHIVE_OUTPUT_DIRECTORY_"     "${dir}"  "<SAME>"
  "COMPILE_PDB_OUTPUT_DIRECTORY_" "${dir}"  "<SAME>"
  "LIBRARY_OUTPUT_DIRECTORY_"     "${dir}"  "<SAME>"
  "PDB_OUTPUT_DIRECTORY_"         "${dir}"  "<SAME>"
  "RUNTIME_OUTPUT_DIRECTORY_"     "${dir}"  "<SAME>"
  )

prepare_target_types(with_artifact
           EXECUTABLE          MODULE          OBJECT          SHARED          STATIC
  IMPORTED_EXECUTABLE IMPORTED_MODULE IMPORTED_OBJECT IMPORTED_SHARED IMPORTED_STATIC)
run_property_tests(with_artifact properties)
