include(RunCMake)

if(RunCMake_GENERATOR MATCHES "^Ninja")
  # Detect ninja version so we know what tests can be supported.
  execute_process(
    COMMAND "${RunCMake_MAKE_PROGRAM}" --version
    OUTPUT_VARIABLE ninja_out
    ERROR_VARIABLE ninja_out
    RESULT_VARIABLE ninja_res
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  if(ninja_res EQUAL 0 AND "x${ninja_out}" MATCHES "^x[0-9]+\\.[0-9]+")
    set(ninja_version "${ninja_out}")
    message(STATUS "ninja version: ${ninja_version}")
  else()
    message(FATAL_ERROR "'ninja --version' reported:\n${ninja_out}")
  endif()
else()
  set(ninja_version "")
endif()

run_cmake(CMP0037-OLD-space)
run_cmake(CMP0037-NEW-space)
run_cmake(CMP0037-WARN-space)
run_cmake(CMP0037-NEW-colon)

if(NOT (WIN32 AND "${RunCMake_GENERATOR}" MATCHES "Make"))
  run_cmake(CMP0037-WARN-colon)
endif()

if(NOT ninja_version VERSION_GREATER_EQUAL 1.10)
  run_cmake(CMP0037-WARN-reserved)
  run_cmake(CMP0037-OLD-reserved)
endif()
run_cmake(CMP0037-NEW-reserved)

run_cmake(NEW-cond)
run_cmake(NEW-cond-test)
run_cmake(NEW-cond-package)
run_cmake(OLD-cond)
run_cmake(OLD-cond-test)
run_cmake(OLD-cond-package)
run_cmake(WARN-cond)
run_cmake(WARN-cond-test)
run_cmake(WARN-cond-package)

if(RunCMake_GENERATOR MATCHES "Make|Ninja")
  run_cmake(NEW-cond-package_source)
  run_cmake(OLD-cond-package_source)
  run_cmake(WARN-cond-package_source)
endif()
