include(RunCMake)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "AIX")
  # Tests with ENABLE_EXPORTS ON.  For AIX we do not use the flags at all.
  run_cmake(NEWBad)
endif()
run_cmake(NEWGood)
