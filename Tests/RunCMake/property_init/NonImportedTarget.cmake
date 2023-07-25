set(properties
  # property                      expected            alias
  # Compilation properties
  ## Language
  ### CSharp
  "DOTNET_SDK"                    "Microsoft.NET.Sdk" "<SAME>"
  )

prepare_target_types(non_imported
  EXECUTABLE SHARED STATIC MODULE OBJECT INTERFACE CUSTOM)
run_property_tests(non_imported properties)
