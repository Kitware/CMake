set(properties
  # property                        expected        alias
  # Compilation properties
  ## Language
  ### CSharp
  "DOTNET_TARGET_FRAMEWORK"         "netcoreapp2.1" "<SAME>"
  "DOTNET_TARGET_FRAMEWORK_VERSION" "v4.5"          "<SAME>"
  )

prepare_target_types(with_commands
           EXECUTABLE          MODULE          OBJECT          SHARED          STATIC CUSTOM
  IMPORTED_EXECUTABLE IMPORTED_MODULE IMPORTED_OBJECT IMPORTED_SHARED IMPORTED_STATIC)
run_property_tests(with_commands properties)
