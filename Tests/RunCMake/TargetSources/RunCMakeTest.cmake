include(RunCMake)

if(RunCMake_GENERATOR MATCHES "Visual Studio|Xcode")
  run_cmake(ConfigNotAllowed)
endif()

run_cmake(OriginDebug)
run_cmake(CMP0026-LOCATION)
run_cmake(RelativePathInInterface)
run_cmake(ExportBuild)
