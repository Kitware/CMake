include(RunCMake)

run_cmake(NoTarget)
run_cmake(NotObjlibTarget)

if(RunCMake_GENERATOR STREQUAL "Xcode" AND "$ENV{CMAKE_OSX_ARCHITECTURES}" MATCHES "[;$]")
  run_cmake(XcodeVariableNoGenexExpansion)
endif()

function(run_cmake_and_build case)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-build)
  run_cmake(${case})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${case}-build ${CMAKE_COMMAND} --build .)
endfunction()
run_cmake_and_build(Unity)
