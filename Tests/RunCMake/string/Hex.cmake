function(assert_strequal input actual expected)
  if(NOT expected STREQUAL actual)
    message(SEND_ERROR "Output did not match expected\nInput string:\n  ${input}\nExpected:\n  ${expected}\nActual:\n  ${actual}")
  endif()
endfunction()

set(_input1 "The quick brown fox jumps over the lazy dog.")
string(HEX "${_input1}" _result1)
assert_strequal("${_input1}" "${_result1}" "54686520717569636b2062726f776e20666f78206a756d7073206f76657220746865206c617a7920646f672e")

set(_input2 "Hello world!")
string(HEX "${_input2}" _result2)
assert_strequal("${_input2}" "${_result2}" "48656c6c6f20776f726c6421")

set(_input3 "Ash nazg durbatulûk\nAsh nazg gimbatul\nAsh nazg thrakatulûk\nAgh burzum-ishi krimpatul")
string(HEX "${_input3}" _result3)
assert_strequal("${_input3}" "${_result3}" "417368206e617a6720647572626174756cc3bb6b0a417368206e617a672067696d626174756c0a417368206e617a6720746872616b6174756cc3bb6b0a416768206275727a756d2d69736869206b72696d706174756c")

string(HEX "" _result_empty)
assert_strequal("" "${_result_empty}" "")
