set(dir "${CMAKE_CURRENT_BINARY_DIR}")

set(properties
  # property                      expected  alias
  # Compilation properties
  ## Platforms
  ### Windows
  "VS_DEBUGGER_COMMAND"           "vsdbg"   "<SAME>"
  "VS_DEBUGGER_COMMAND_ARGUMENTS" "/?"      "<SAME>"
  "VS_DEBUGGER_ENVIRONMENT"       "env=val" "<SAME>"
  "VS_DEBUGGER_WORKING_DIRECTORY" "${dir}"  "<SAME>"

  # Linking properties
  ## Platforms
  ### Android
  "ANDROID_GUI"                   "OFF"     "<SAME>"

  # Metadata
  "CROSSCOMPILING_EMULATOR"       "emu"     "<SAME>"
  "TEST_LAUNCHER"                 "test"    "<SAME>"
  )

prepare_target_types(executable
           EXECUTABLE
  IMPORTED_EXECUTABLE)
run_property_tests(executable properties)
