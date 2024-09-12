execute_process(
  COMMAND ${TEST_ENCODING_EXE} ${ENCODING} ${CMAKE_CURRENT_LIST_DIR}/Encoding${ENCODING}-stderr.txt
  OUTPUT_VARIABLE out
  ENCODING ${ENCODING}
  )
message("${out}")
