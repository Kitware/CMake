include(RunCMake)

run_cmake(NoTarget)
run_cmake(NotObjlibTarget)

if(RunCMake_GENERATOR STREQUAL "Xcode" AND "$ENV{CMAKE_OSX_ARCHITECTURES}" MATCHES "[;$]")
  run_cmake(XcodeVariableNoGenexExpansion)
endif()
