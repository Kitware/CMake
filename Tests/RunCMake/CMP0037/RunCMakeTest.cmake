include(RunCMake)

run_cmake(CMP0037-NEW-space)
run_cmake(CMP0037-NEW-colon)

run_cmake(CMP0037-NEW-reserved)

run_cmake(NEW-cond)
run_cmake(NEW-cond-test)
run_cmake(NEW-cond-package)

run_cmake(alias-test-NEW)

if(RunCMake_GENERATOR MATCHES "Make|Ninja")
  run_cmake(NEW-cond-package_source)
endif()
