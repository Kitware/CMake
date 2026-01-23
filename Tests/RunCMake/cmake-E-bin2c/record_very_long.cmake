file(CONFIGURE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/very_long_executables.cmake"
  CONTENT [==[set(generate_very_long [====[@GENERATE_VERY_LONG@]====])
]==]
  @ONLY
  )
