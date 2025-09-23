function(check_output name expected)
  set(output "${${name}}")
  if(NOT output STREQUAL expected)
    message(FATAL_ERROR "\"string(REGEX)\" set ${name} to \"${output}\", expected \"${expected}\"")
  endif()
endfunction()

# OLD
cmake_policy(SET CMP0186 OLD)

string(REGEX MATCHALL "^0" out "0000")
check_output(out "0;0;0;0")

string(REGEX MATCHALL "^0+" out "0000")
check_output(out "0000")

string(REGEX MATCHALL "^(0|a)" out "0000" )
check_output(out "0;0;0;0")

string(REGEX MATCHALL "^(0|a)" out "aaaa")
check_output(out "a;a;a;a")

string(REGEX MATCHALL "^(0|a)" out "a0a0")
check_output(out "a;0;a;0")

string(REGEX MATCHALL "(^|a)0" out "00a0")
check_output(out "0;0;a0")

string(REGEX REPLACE "^0" "" out "0000")
check_output(out "")

string(REGEX REPLACE "^0" "x" out "0000")
check_output(out "xxxx")

string(REGEX REPLACE "^0+" "x" out "0000")
check_output(out "x")

string(REGEX REPLACE "^(0|a)" "x" out "0000")
check_output(out "xxxx")

string(REGEX REPLACE "^(0|a)" "x" out "aaaa")
check_output(out "xxxx")

string(REGEX REPLACE "^(0|a)" "x" out "a0a0")
check_output(out "xxxx")

string(REGEX REPLACE "(^|a)0" "x" out "00a0")
check_output(out "xxx")

# NEW, same cases as above
cmake_policy(SET CMP0186 NEW)

string(REGEX MATCHALL "^0" out "0000")
check_output(out "0")

string(REGEX MATCHALL "^0+" out "0000")
check_output(out "0000")

string(REGEX MATCHALL "^(0|a)" out "0000")
check_output(out "0")

string(REGEX MATCHALL "^(0|a)" out "aaaa")
check_output(out "a")

string(REGEX MATCHALL "^(0|a)" out "a0a0")
check_output(out "a")

string(REGEX MATCHALL "(^|a)0" out "00a0")
check_output(out "0;a0")

string(REGEX REPLACE "^0" "" out "0000")
check_output(out "000")

string(REGEX REPLACE "^0" "x" out "0000")
check_output(out "x000")

string(REGEX REPLACE "^0+" "x" out "0000")
check_output(out "x")

string(REGEX REPLACE "^(0|a)" "x" out "0000")
check_output(out "x000")

string(REGEX REPLACE "^(0|a)" "x" out "aaaa")
check_output(out "xaaa")

string(REGEX REPLACE "^(0|a)" "x" out "a0a0")
check_output(out "x0a0")

string(REGEX REPLACE "(^|a)0" "x" out "00a0")
check_output(out "x0x")
