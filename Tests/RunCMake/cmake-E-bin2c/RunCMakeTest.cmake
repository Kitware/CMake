include(RunCMake)

function(run_bin2c name contents_name)
  string(REPLACE ";" "\\;" ARGS "${ARGN}")
  run_cmake_command(${name}
    "${CMAKE_COMMAND}"
    "-DNAME=${name}"
    "-DINPUT_FILE=${RunCMake_TEST_BINARY_DIR}/${contents_name}.bin"
    "-DARGS=${ARGS}"
    -P "${RunCMake_SOURCE_DIR}/run_bin2c.cmake"
    )
endfunction()

set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/generate_binaries-build")
run_cmake_with_options(generate_binaries "-DCMake_TEST_BIN2C_LARGE_FILE:BOOL=${CMake_TEST_BIN2C_LARGE_FILE}")
set(RunCMake_TEST_NO_CLEAN 1)
run_cmake_command(generate_binaries-build "${CMAKE_COMMAND}" --build .)
run_bin2c(basic basic)
run_bin2c(empty empty)
run_bin2c(text_lf text_lf)
run_bin2c(text_crlf text_crlf)
run_bin2c(text_align text_align)
run_bin2c(long long)
run_bin2c(double_hyphen basic --)
run_bin2c(signed basic --signed)
run_bin2c(long_signed long --signed)
run_bin2c(decimal basic --decimal)
run_bin2c(long_decimal long --decimal)
run_bin2c(signed_decimal basic --signed --decimal)
run_bin2c(long_signed_decimal long --signed --decimal)
run_bin2c(trailing_comma basic --trailing-comma)
run_bin2c(trailing_comma_empty empty --trailing-comma)
run_bin2c(trailing_comma_text_align text_align --trailing-comma)
run_bin2c(template_file basic --template-file "${RunCMake_SOURCE_DIR}/template_file.c.in.txt")
run_bin2c(template_file_empty empty --template-file "${RunCMake_SOURCE_DIR}/template_file.c.in.txt")
run_bin2c(template_file_trailing_comma basic --template-file "${RunCMake_SOURCE_DIR}/template_file.c.in.txt" --trailing-comma)
run_bin2c(template_file_placeholders basic --template-file "${RunCMake_SOURCE_DIR}/template_file_placeholders.c.in.txt" --template-array-placeholder arr --template-length-placeholder len)

run_cmake_command(too_many_files ${CMAKE_COMMAND} -E bin2c a a a)
run_cmake_command(arg_after_double_hyphen ${CMAKE_COMMAND} -E bin2c --array-name arr -- --size-name size)
run_cmake_command(input_noexist ${CMAKE_COMMAND} -E bin2c noexist.bin)
file(WRITE "${RunCMake_TEST_BINARY_DIR}/not_a_dir" "")
run_cmake_command(output_not_a_dir ${CMAKE_COMMAND} -E bin2c "${RunCMake_TEST_BINARY_DIR}/basic.bin" "not_a_dir/output_not_a_dir.c.txt")
run_cmake_command(template_file_invalid_array_placeholder ${CMAKE_COMMAND} -E bin2c --template-file "${RunCMake_SOURCE_DIR}/template_file.c.in.txt" --template-array-placeholder "array*")
run_cmake_command(template_file_invalid_length_placeholder ${CMAKE_COMMAND} -E bin2c --template-file "${RunCMake_SOURCE_DIR}/template_file.c.in.txt" --template-length-placeholder "length*")
run_cmake_command(template_file_noexist ${CMAKE_COMMAND} -E bin2c --template-file noexist.c.in.txt a a)
run_cmake_command(template_file_double_array ${CMAKE_COMMAND} -E bin2c --template-file "${RunCMake_SOURCE_DIR}/template_file_double_array.c.in.txt" "${RunCMake_TEST_BINARY_DIR}/basic.bin")
run_cmake_command(template_file_length_before_array ${CMAKE_COMMAND} -E bin2c --template-file "${RunCMake_SOURCE_DIR}/template_file_length_before_array.c.in.txt" "${RunCMake_TEST_BINARY_DIR}/basic.bin")

if(CMake_TEST_BIN2C_LARGE_FILE)
  run_cmake_command(generate_binaries-verify_very_long_hash
    "${CMAKE_COMMAND}"
    --build .
    --target verify_very_long_hash
    )
  run_cmake_command(very_long
    "${CMAKE_COMMAND}"
    -P "${RunCMake_SOURCE_DIR}/very_long.cmake"
    )
endif()
