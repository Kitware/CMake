string(TIMESTAMP output "%z")

STRING(LENGTH output output_length)

message("~${output}~")

set(expected_output_length 6)

if(NOT(${output_length} EQUAL ${expected_output_length}))
    message(FATAL_ERROR "expected ${expected_output_length} entries in output with all specifiers; found ${output_length}")
endif()

string(SUBSTRING ${output} 0 1 output0)
string(SUBSTRING ${output} 4 1 output4)

if(NOT((${output0} STREQUAL "-") OR (${output0} STREQUAL "+")))
    message(FATAL_ERROR "expected output[0] equ '+' or '-'; found: '${output0}'")
endif()

if(NOT((${output4} STREQUAL "0") OR (${output4} STREQUAL "5")))
    message(FATAL_ERROR "expected output[4] equ '0' or '5'; found: '${output4}'")
endif()
