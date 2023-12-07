set(properties
  # property                      expected  alias
  # Linking properties
  ## Platforms
  ### AIX
  "AIX_EXPORT_ALL_SYMBOLS"        "OFF"     "<SAME>"
  ### Windows
  "WINDOWS_EXPORT_ALL_SYMBOLS"    "OFF"     "<SAME>"
  )

prepare_target_types(symbol_export_target
           EXECUTABLE          SHARED
  IMPORTED_EXECUTABLE IMPORTED_SHARED)
run_property_tests(symbol_export_target properties)

# `ENABLE_EXPORTS` has a more complicated initialization.
set(properties
  # property                      expected  alias
  # Linking properties
  "ENABLE_EXPORTS"                "OFF"     "<SAME>"
  )

prepare_target_types(executable
           EXECUTABLE
  IMPORTED_EXECUTABLE)
set(iteration "-ENABLE_EXPORTS")
run_property_tests(executable_target properties)

set(with_defaults 1)

set(CMAKE_SHARED_LIBRARY_ENABLE_EXPORTS OFF)
set(properties
  # property                      expected  alias
  # Linking properties
  "ENABLE_EXPORTS"                "OFF"     "<SAME>"
  )

set(iteration "-SHARED_LIBRARY_ENABLE_EXPORTS")
run_property_tests(shared_library_target properties)
unset(CMAKE_SHARED_LIBRARY_ENABLE_EXPORTS)

set(CMAKE_EXECUTABLE_ENABLE_EXPORTS OFF)
set(properties
  # property                      expected  alias
  # Linking properties
  "ENABLE_EXPORTS"                "OFF"     "<SAME>"
  )

set(iteration "-EXECUTABLE_ENABLE_EXPORTS")
run_property_tests(executable_target properties)
unset(CMAKE_EXECUTABLE_ENABLE_EXPORTS)
