include(RunCMake)

run_cmake("CMP0171-WARN")

run_cmake_with_options(CMP0171-OLD "-DCMAKE_POLICY_DEFAULT_CMP0171=OLD")

run_cmake_with_options(CMP0171-NEW "-DCMAKE_POLICY_DEFAULT_CMP0171=NEW")

# The entire point of this test is to ensure the codegen target is not created
# unintentionally. It can only be created if CMP0171 is NEW.
block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CMP0171-codegen-build)
  run_cmake(CMP0171-codegen)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  # This command will fail with either 1 or 2 depending.
  run_cmake_command(CMP0171-codegen-build ${CMAKE_COMMAND} --build . --config Debug --target codegen)
endblock()
