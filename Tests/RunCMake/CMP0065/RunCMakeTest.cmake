include(RunCMake)
set(RunCMake_IGNORE_POLICY_VERSION_DEPRECATION ON)

run_cmake(OLDBad1)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "AIX")
  # Tests with ENABLE_EXPORTS ON.  For AIX we do not use the flags at all.
  run_cmake(OLDBad2)
  run_cmake(NEWBad)
endif()
run_cmake(NEWGood)
run_cmake(WARN-OFF)
run_cmake(WARN-ON)
