execute_process(COMMAND ${CMAKE_NM} -D ${TEST_LIBRARY_PATH}
  RESULT_VARIABLE RESULT
  OUTPUT_VARIABLE OUTPUT
  ERROR_VARIABLE ERROR
)

if(NOT "${RESULT}" STREQUAL "0")
  message(FATAL_ERROR "nm failed [${RESULT}] [${OUTPUT}] [${ERROR}]")
endif()

if(${OUTPUT} MATCHES "Foo[^\\n]*bar")
  message(FATAL_ERROR
    "Found Foo::bar() which should have been hidden [${OUTPUT}]")
endif()
