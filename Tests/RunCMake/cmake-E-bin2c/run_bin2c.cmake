set(expected_output_file "${CMAKE_CURRENT_LIST_DIR}/${NAME}.c.txt")

function(run_bin2c output_file)
  execute_process(
    ${ARGN}
    COMMAND_ERROR_IS_FATAL ANY
    )

  execute_process(
    COMMAND ${CMAKE_COMMAND} -E compare_files --ignore-eol "${expected_output_file}" "${output_file}"
    RESULT_VARIABLE result
    )
  if(result)
    file(READ "${expected_output_file}" expected_output)
    file(READ "${output_file}" actual_output)
    string(REPLACE "\n" "\n  " formatted_expected_output "${expected_output}")
    string(REPLACE "\n" "\n  " formatted_actual_output "${actual_output}")
    set(formatted_binary_input)
    file(READ "${INPUT_FILE}" binary_contents HEX)
    string(LENGTH "${binary_contents}" binary_length)
    if(binary_length LESS 256)
      set(formatted_binary_input "\nHexadecimal contents of ${INPUT_FILE}:\n  ${binary_contents}")
    endif()
    message(FATAL_ERROR "${output_file} does not match ${expected_output_file}.\nExpected output:\n  ${formatted_expected_output}\nActual output:\n  ${formatted_actual_output}${formatted_binary_input}")
  endif()
endfunction()

set(output_file "${CMAKE_CURRENT_BINARY_DIR}/${NAME}-no-input-arg-no-output-arg.c.txt")
run_bin2c("${output_file}"
  COMMAND ${CMAKE_COMMAND} -E bin2c ${ARGS}
  INPUT_FILE "${INPUT_FILE}"
  OUTPUT_FILE "${output_file}"
  )
set(output_file "${CMAKE_CURRENT_BINARY_DIR}/${NAME}-stdin-no-output-arg.c.txt")
run_bin2c("${output_file}"
  COMMAND ${CMAKE_COMMAND} -E bin2c ${ARGS} -
  INPUT_FILE "${INPUT_FILE}"
  OUTPUT_FILE "${output_file}"
  )
set(output_file "${CMAKE_CURRENT_BINARY_DIR}/${NAME}-input-file-no-output-arg.c.txt")
run_bin2c("${output_file}"
  COMMAND ${CMAKE_COMMAND} -E bin2c ${ARGS} "${INPUT_FILE}"
  OUTPUT_FILE "${output_file}"
  )
set(output_file "${CMAKE_CURRENT_BINARY_DIR}/${NAME}-stdin-stdout.c.txt")
run_bin2c("${output_file}"
  COMMAND ${CMAKE_COMMAND} -E bin2c ${ARGS} - -
  INPUT_FILE "${INPUT_FILE}"
  OUTPUT_FILE "${output_file}"
  )
set(output_file "${CMAKE_CURRENT_BINARY_DIR}/${NAME}-input-file-stdout.c.txt")
run_bin2c("${output_file}"
  COMMAND ${CMAKE_COMMAND} -E bin2c ${ARGS} "${INPUT_FILE}" -
  OUTPUT_FILE "${output_file}"
  )
set(output_file "${CMAKE_CURRENT_BINARY_DIR}/${NAME}-stdin-output-file.c.txt")
run_bin2c("${output_file}"
  COMMAND ${CMAKE_COMMAND} -E bin2c ${ARGS} - "${output_file}"
  INPUT_FILE "${INPUT_FILE}"
  )
set(output_file "${CMAKE_CURRENT_BINARY_DIR}/${NAME}-input-file-output-file.c.txt")
run_bin2c("${output_file}"
  COMMAND ${CMAKE_COMMAND} -E bin2c ${ARGS} "${INPUT_FILE}" "${output_file}"
  )
