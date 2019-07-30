string(REPEAT "q" 4 q_out)

if(NOT DEFINED q_out)
  message(FATAL_ERROR "q_out is not defined")
endif()

if(NOT q_out STREQUAL "qqqq")
  message(FATAL_ERROR "unexpected result")
endif()

string(REPEAT "1234" 0 zero_out)

if(NOT DEFINED zero_out)
  message(FATAL_ERROR "zero_out is not defined")
endif()

if(NOT zero_out STREQUAL "")
  message(FATAL_ERROR "unexpected result")
endif()

unset(zero_out)

string(REPEAT "" 100 zero_out)

if(NOT DEFINED zero_out)
  message(FATAL_ERROR "zero_out is not defined")
endif()

if(NOT zero_out STREQUAL "")
  message(FATAL_ERROR "unexpected result")
endif()

string(REPEAT "1" 1 one_out)

if(NOT one_out STREQUAL "1")
  message(FATAL_ERROR "unexpected result")
endif()

unset(one_out)

string(REPEAT "one" 1 one_out)

if(NOT one_out STREQUAL "one")
  message(FATAL_ERROR "unexpected result")
endif()
