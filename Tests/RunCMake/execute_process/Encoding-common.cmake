if(ENCODING)
  set(maybe_ENCODING ENCODING ${ENCODING})
else()
  set(maybe_ENCODING "")
  set(ENCODING AUTO) # execute_process's default ENCODING
endif()
execute_process(
  COMMAND ${TEST_ENCODING_EXE} ${ENCODING} ${CMAKE_CURRENT_LIST_DIR}/Encoding${ENCODING}-stderr.txt
  OUTPUT_VARIABLE out
  ${maybe_ENCODING}
  )
