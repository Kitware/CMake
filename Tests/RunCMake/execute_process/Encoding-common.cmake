if(ENCODING)
  set(maybe_ENCODING ENCODING ${ENCODING})
else()
  set(maybe_ENCODING "")
  cmake_policy(GET CMP0176 cmp0176)
  if(cmp0176 STREQUAL "NEW")
    set(ENCODING UTF-8) # execute_process's default ENCODING
  else()
    set(ENCODING AUTO) # execute_process's default ENCODING
  endif()
endif()
execute_process(
  COMMAND ${TEST_ENCODING_EXE} ${ENCODING} ${CMAKE_CURRENT_LIST_DIR}/Encoding${ENCODING}-stderr.txt
  OUTPUT_VARIABLE out
  ${maybe_ENCODING}
  )
