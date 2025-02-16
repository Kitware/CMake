cmake_policy(SET CMP0186 NEW)

function(check_output name expected)
  set(output "${${name}}")
  if(NOT output STREQUAL expected)
    message(FATAL_ERROR "\"string(REGEX)\" set ${name} to \"${output}\", expected \"${expected}\"")
  endif()
endfunction()

# Zero-length matches in REGEX MATCH

string(REGEX MATCH "" out "")
check_output(out "")

string(REGEX MATCH "" out "a")
check_output(out "")

string(REGEX MATCH "a*" out "")
check_output(out "")

string(REGEX MATCH "a*" out "a")
check_output(out "a")

string(REGEX MATCH "a*" out "b")
check_output(out "")

string(REGEX MATCH "a*" out "ba")
check_output(out "")

# Zero-length matches in REGEX MATCHALL

string(REGEX MATCHALL "" out "")
check_output(out "")

string(REGEX MATCHALL "" out "ab")
check_output(out ";;")

string(REGEX MATCHALL "^" out "ab")
check_output(out "")

string(REGEX MATCHALL "(^|,)" out "a,b")
check_output(out ";,")

string(REGEX MATCHALL "(,|^)" out "a,b")
check_output(out ";,")

string(REGEX MATCHALL "(^|)" out "")
check_output(out "")

string(REGEX MATCHALL "(^|)" out "ab")
check_output(out ";;")

string(REGEX MATCHALL "a|^" out "ab")
check_output(out "a")

string(REGEX MATCHALL "$" out "ab")
check_output(out "")

string(REGEX MATCHALL "($|,)" out "a,b")
check_output(out ",;")

string(REGEX MATCHALL "(,|$)" out "a,b")
check_output(out ",;")

string(REGEX MATCHALL "(|$)" out "")
check_output(out "")

string(REGEX MATCHALL "(|$)" out "ab")
check_output(out ";;")

string(REGEX MATCHALL "(b|)" out "abc")
check_output(out ";b;;")

string(REGEX MATCHALL "(|b)" out "abc")
check_output(out ";;b;;")

string(REGEX MATCHALL "a*" out "aaa")
check_output(out "aaa;")

string(REGEX MATCHALL "(a)?(b)?" out "")
check_output(out "")

string(REGEX MATCHALL "(a)?(b)?" out "abba")
check_output(out "ab;b;a;")

# Zero-length matches in REGEX REPLACE

string(REGEX REPLACE "" "" out "")
check_output(out "")

string(REGEX REPLACE "" "x" out "")
check_output(out "x")

string(REGEX REPLACE "" "x" out "ab")
check_output(out "xaxbx")

string(REGEX REPLACE "^" "x" out "ab")
check_output(out "xab")

string(REGEX REPLACE "(^|,)" "x" out "a,b")
check_output(out "xaxb")

string(REGEX REPLACE "(,|^)" "x" out "a,b")
check_output(out "xaxb")

string(REGEX REPLACE "(^|)" "x" out "")
check_output(out "x")

string(REGEX REPLACE "(^|)" "x" out "ab")
check_output(out "xaxbx")

string(REGEX REPLACE "a|^" "x" out "ab")
check_output(out "xb")

string(REGEX REPLACE "$" "x" out "ab")
check_output(out "abx")

string(REGEX REPLACE "($|,)" "x" out "a,b")
check_output(out "axbx")

string(REGEX REPLACE "(,|$)" "x" out "a,b")
check_output(out "axbx")

string(REGEX REPLACE "(|$)" "x" out "")
check_output(out "x")

string(REGEX REPLACE "(|$)" "x" out "ab")
check_output(out "xaxbx")

string(REGEX REPLACE "(b|)" "x" out "abc")
check_output(out "xaxxcx")

string(REGEX REPLACE "(|b)" "x" out "abc")
check_output(out "xaxxxcx")

string(REGEX REPLACE "a*" "x" out "aaa")
check_output(out "xx")

string(REGEX REPLACE "(a)?(b)?" "x" out "")
check_output(out "x")

string(REGEX REPLACE "(a)?(b)?" "x" out "abba")
check_output(out "xxxx")
