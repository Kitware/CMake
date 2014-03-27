include(RunCMake)

if(RunCMake_GENERATOR MATCHES Xcode
    OR RunCMake_GENERATOR MATCHES "Visual Studio")
  run_cmake(ConfigNotAllowed)
  run_cmake(OriginDebugIDE)
else()
  run_cmake(OriginDebug)
endif()
