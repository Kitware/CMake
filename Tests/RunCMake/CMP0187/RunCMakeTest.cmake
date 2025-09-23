include(RunCMake)

block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CMP0187-NEW-build)
  run_cmake_with_options(CMP0187-NEW "-DCMAKE_POLICY_DEFAULT_CMP0187=NEW")

  if(RunCMake_GENERATOR MATCHES "Ninja.*")
    set(RunCMake_TEST_NO_CLEAN 1)
    # -n: dry-run to avoid actually compiling, -v: verbose to capture executed command
    run_cmake_command(CMP0187-NEW-build ${CMAKE_COMMAND} --build .)
  endif()
endblock()

block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CMP0187-OLD-build)
  run_cmake_with_options(CMP0187-OLD "-DCMAKE_POLICY_DEFAULT_CMP0187=OLD")

  if(RunCMake_GENERATOR MATCHES "Ninja.*")
    set(RunCMake_TEST_NO_CLEAN 1)
    # -n: dry-run to avoid actually compiling, -v: verbose to capture executed command
    run_cmake_command(CMP0187-OLD-build ${CMAKE_COMMAND} --build .)
  endif()
endblock()

block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CMP0187-NEW-CMP0115-OLD-build)
  run_cmake(CMP0187-NEW-CMP0115-OLD)

  if(RunCMake_GENERATOR MATCHES "Ninja.*")
    set(RunCMake_TEST_NO_CLEAN 1)
    # -n: dry-run to avoid actually compiling, -v: verbose to capture executed command
    run_cmake_command(CMP0187-NEW-CMP0115-OLD-build ${CMAKE_COMMAND} --build .)
  endif()
endblock()
