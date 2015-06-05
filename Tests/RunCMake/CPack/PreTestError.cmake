file(READ "${bin_dir}/test_output.txt" output)
file(READ "${bin_dir}/test_error.txt" error)

message(FATAL_ERROR "Error in pre-test phase '${RunCMake_TEST_STEP}'!\n"
        "Return code: '${return_code}'\n"
        "Info output: '${output}'\n"
        "Error output: '${error}'")
