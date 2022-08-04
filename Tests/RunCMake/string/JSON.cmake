function(assert_strequal actual expected)
  if(NOT expected STREQUAL actual)
    message(SEND_ERROR "Output:\n${actual}\nDid not match expected:\n${expected}\n")
  endif()
endfunction()

function(assert_strequal_error actual expected error)
  if(error)
    message(SEND_ERROR "Unexpected error: ${error}")
  endif()
  assert_strequal("${actual}" "${expected}")
endfunction()

function(assert_json_equal error actual expected)
  if(error)
    message(SEND_ERROR "Unexpected error: ${error}")
  endif()
  string(JSON eql EQUAL "${actual}" "${expected}")
  if(NOT eql)
    message(SEND_ERROR "Expected equality got\n ${actual}\n expected\n${expected}")
  endif()
endfunction()

# test EQUAL
string(JSON result EQUAL
[=[ {"foo":"bar"} ]=]
[=[
{
"foo": "bar"
}
]=])
if(NOT result)
  message(SEND_ERROR "Expected ON got ${result}")
endif()

string(JSON result EQUAL
[=[ {"foo":"bar"} ]=]
[=[
{
"foo1": "bar"
}
]=])
if(result)
  message(SEND_ERROR "Expected OFF got ${result}")
endif()



set(json1 [=[
{
  "foo" : "bar",
  "array" : [5, "val", {"some": "other"}, null],
  "types" : {
    "null" : null,
    "number" : 5,
    "string" : "foo",
    "boolean" : false,
    "array" : [1,2,3],
    "object" : {}
  },
  "values" : {
    "null" : null,
    "number" : 5,
    "string" : "foo",
    "false" : false,
    "true" : true
  },
  "special" : {
    "foo;bar" : "value1",
    ";" : "value2",
    "semicolon" : ";",
    "list" : ["one", "two;three", "four"],
    "quote" : "\"",
    "\"" : "quote",
    "backslash" : "\\",
    "\\" : "backslash",
    "slash" : "\/",
    "\/" : "slash",
    "newline" : "\n",
    "\n" : "newline",
    "return" : "\r",
    "\r" : "return",
    "tab" : "\t",
    "\t" : "tab",
    "backspace" : "\b",
    "\b" : "backspace",
    "formfeed" : "\f",
    "\f" : "formfeed"
   }
}
]=])

string(JSON result GET "${json1}" foo)
assert_strequal("${result}" bar)
string(JSON result GET "${json1}" array 0)
assert_strequal("${result}" 5)
string(JSON result GET "${json1}" array 1)
assert_strequal("${result}" val)
string(JSON result GET "${json1}" array 2 some)
assert_strequal("${result}" other)

string(JSON result GET "${json1}" values null)
assert_strequal("${result}" "")
string(JSON result GET "${json1}" values number)
assert_strequal("${result}" 5)
string(JSON result GET "${json1}" values string)
assert_strequal("${result}" "foo")
string(JSON result GET "${json1}" values true)
assert_strequal("${result}" "ON")
if(NOT result)
  message(SEND_ERROR "Output did not match expected: TRUE actual: ${result}")
endif()
string(JSON result GET "${json1}" values false)
assert_strequal("${result}" "OFF")
if(result)
  message(SEND_ERROR "Output did not match expected: FALSE actual: ${result}")
endif()

string(JSON result ERROR_VARIABLE error GET "${json1}" foo)
assert_strequal_error("${result}" "bar" "${error}")

string(JSON result ERROR_VARIABLE error GET "${json1}" notThere)
assert_strequal("${result}" "notThere-NOTFOUND")
assert_strequal("${error}" "member 'notThere' not found")

string(JSON result ERROR_VARIABLE error GET "${json1}" 0)
assert_strequal("${result}" "0-NOTFOUND")
assert_strequal("${error}" "member '0' not found")

string(JSON result ERROR_VARIABLE error GET "${json1}" array 10)
assert_strequal("${result}" "array-10-NOTFOUND")
assert_strequal("${error}" "expected an index less than 4 got '10'")

string(JSON result ERROR_VARIABLE error GET "${json1}" array 2 some notThere)
assert_strequal("${result}" "array-2-some-notThere-NOTFOUND")
assert_strequal("${error}" "invalid path 'array 2 some notThere', need element of OBJECT or ARRAY type to lookup 'notThere' got STRING")

# special chars
string(JSON result ERROR_VARIABLE error GET "${json1}" special "foo;bar")
assert_strequal_error("${result}" "value1" "${error}")
string(JSON result ERROR_VARIABLE error GET "${json1}" special ";")
assert_strequal_error("${result}" "value2" "${error}")
string(JSON result ERROR_VARIABLE error GET "${json1}" special semicolon)
assert_strequal_error("${result}" ";" "${error}")

string(JSON result ERROR_VARIABLE error GET "${json1}" special list 1)
assert_strequal_error("${result}" "two;three" "${error}")

string(JSON result ERROR_VARIABLE error GET "${json1}")
assert_json_equal("${error}" "${result}" "${json1}")

string(JSON result ERROR_VARIABLE error GET "${json1}" array)
assert_json_equal("${error}" "${result}" [=[ [5, "val", {"some": "other"}, null] ]=])

string(JSON result ERROR_VARIABLE error GET "${json1}" special quote)
assert_strequal_error("${result}" "\"" "${error}")
string(JSON result ERROR_VARIABLE error GET "${json1}" special "\"")
assert_strequal_error("${result}" "quote" "${error}")

string(JSON result ERROR_VARIABLE error GET "${json1}" special backslash)
assert_strequal_error("${result}" "\\" "${error}")
string(JSON result ERROR_VARIABLE error GET "${json1}" special "\\")
assert_strequal_error("${result}" "backslash" "${error}")

string(JSON result ERROR_VARIABLE error GET "${json1}" special slash)
assert_strequal_error("${result}" "/" "${error}")
string(JSON result ERROR_VARIABLE error GET "${json1}" special "/")
assert_strequal_error("${result}" "slash" "${error}")

string(JSON result ERROR_VARIABLE error GET "${json1}" special newline)
assert_strequal_error("${result}" "\n" "${error}")
string(JSON result ERROR_VARIABLE error GET "${json1}" special "\n")
assert_strequal_error("${result}" "newline" "${error}")

string(JSON result ERROR_VARIABLE error GET "${json1}" special return)
assert_strequal_error("${result}" "\r" "${error}")
string(JSON result ERROR_VARIABLE error GET "${json1}" special "\r")
assert_strequal_error("${result}" "return" "${error}")

string(JSON result ERROR_VARIABLE error GET "${json1}" special tab)
assert_strequal_error("${result}" "\t" "${error}")
string(JSON result ERROR_VARIABLE error GET "${json1}" special "\t")
assert_strequal_error("${result}" "tab" "${error}")

file(READ ${CMAKE_CURRENT_LIST_DIR}/json/unicode.json unicode)
string(JSON char ERROR_VARIABLE error GET "${unicode}" backspace)
string(JSON result ERROR_VARIABLE error GET "${unicode}" "${char}")
assert_strequal_error("${result}" "backspace" "${error}")

file(READ ${CMAKE_CURRENT_LIST_DIR}/json/unicode.json unicode)
string(JSON char ERROR_VARIABLE error GET "${unicode}" backspace)
string(JSON result ERROR_VARIABLE error GET "${unicode}" "${char}")
assert_strequal_error("${result}" "backspace" "${error}")

string(JSON char ERROR_VARIABLE error GET "${unicode}" formfeed)
string(JSON result ERROR_VARIABLE error GET "${unicode}" "${char}")
assert_strequal_error("${result}" "formfeed" "${error}")

string(JSON char ERROR_VARIABLE error GET "${unicode}" datalinkescape)
string(JSON result ERROR_VARIABLE error GET "${unicode}" "${char}")
assert_strequal_error("${result}" "datalinkescape" "${error}")

# Test TYPE
string(JSON result TYPE "${json1}" types null)
assert_strequal("${result}" NULL)
string(JSON result TYPE "${json1}" types number)
assert_strequal("${result}" NUMBER)
string(JSON result TYPE "${json1}" types string)
assert_strequal("${result}" STRING)
string(JSON result TYPE "${json1}" types boolean)
assert_strequal("${result}" BOOLEAN)
string(JSON result TYPE "${json1}" types array)
assert_strequal("${result}" ARRAY)
string(JSON result TYPE "${json1}" types object)
assert_strequal("${result}" OBJECT)

# Test LENGTH
string(JSON result ERROR_VARIABLE error LENGTH "${json1}")
assert_strequal("${result}" 5)
if(error)
  message(SEND_ERROR "Unexpected error: ${error}")
endif()

string(JSON result ERROR_VARIABLE error LENGTH "${json1}" array)
assert_strequal("${result}" 4)
if(error)
  message(SEND_ERROR "Unexpected error: ${error}")
endif()

string(JSON result ERROR_VARIABLE error LENGTH "${json1}" foo)
assert_strequal("${result}" "foo-NOTFOUND")
assert_strequal("${error}" "LENGTH needs to be called with an element of type ARRAY or OBJECT, got STRING")

# Test MEMBER
string(JSON result ERROR_VARIABLE error MEMBER "${json1}" values 2)
assert_strequal("${result}" "number")
if(error)
  message(SEND_ERROR "Unexpected error: ${error}")
endif()

string(JSON result ERROR_VARIABLE error MEMBER "${json1}" values 100)
assert_strequal("${result}" "values-100-NOTFOUND")
assert_strequal("${error}" "expected an index less than 5 got '100'")

# Test length loops
string(JSON arrayLength ERROR_VARIABLE error LENGTH "${json1}" types array)
if(error)
  message(SEND_ERROR "Unexpected error: ${error}")
endif()
set(values "")
math(EXPR arrayLength "${arrayLength}-1")
foreach(index RANGE ${arrayLength})
  string(JSON value ERROR_VARIABLE error GET "${json1}" types array ${index})
  if(error)
    message(SEND_ERROR "Unexpected error: ${error}")
  endif()
  list(APPEND values "${value}")
endforeach()
assert_strequal("${values}" "1;2;3")

string(JSON valuesLength ERROR_VARIABLE error LENGTH "${json1}" values)
if(error)
  message(SEND_ERROR "Unexpected error: ${error}")
endif()
set(values "")
set(members "")
math(EXPR valuesLength "${valuesLength}-1")
foreach(index RANGE ${valuesLength})
  string(JSON member ERROR_VARIABLE error MEMBER "${json1}" values ${index})
  if(error)
    message(SEND_ERROR "Unexpected error: ${error}")
  endif()
  string(JSON value ERROR_VARIABLE error GET "${json1}" values ${member})
  if(error)
    message(SEND_ERROR "Unexpected error: ${error}")
  endif()

  list(APPEND members "${member}")
  list(APPEND values "${value}")
endforeach()
assert_strequal("${members}" "false;null;number;string;true")
assert_strequal("${values}" "OFF;;5;foo;ON")

# Test REMOVE
set(json2 [=[{
  "foo" : "bar",
  "array" : [5, "val", {"some": "other"}, null]
}]=])
string(JSON result ERROR_VARIABLE error REMOVE ${json2} foo)
assert_json_equal("${error}" "${result}"
[=[{
  "array" : [5, "val", {"some": "other"}, null]
}]=])

string(JSON result ERROR_VARIABLE error REMOVE ${json2} array 1)
assert_json_equal("${error}" "${result}"
[=[{
  "foo" : "bar",
  "array" : [5, {"some": "other"}, null]
}]=])

string(JSON result ERROR_VARIABLE error REMOVE ${json2} array 100)
assert_strequal("${result}" "array-100-NOTFOUND")
assert_strequal("${error}" "expected an index less than 4 got '100'")

# Test SET
string(JSON result ERROR_VARIABLE error SET ${json2} new 5)
assert_json_equal("${error}" "${result}"
[=[{
  "foo" : "bar",
  "array" : [5, "val", {"some": "other"}, null],
  "new" : 5
}]=])

string(JSON result ERROR_VARIABLE error SET ${json2} new [=[ {"obj" : false} ]=])
assert_json_equal("${error}" "${result}"
[=[{
  "foo" : "bar",
  "array" : [5, "val", {"some": "other"}, null],
  "new" : {"obj" : false}
}]=])

string(JSON result ERROR_VARIABLE error SET ${json2} array 0 6)
assert_json_equal("${error}" "${result}"
[=[{
  "foo" : "bar",
  "array" : [6, "val", {"some": "other"}, null]
}]=])

string(JSON result ERROR_VARIABLE error SET ${json2} array 5 [["append"]])
assert_json_equal("${error}" "${result}"
[=[{
  "foo" : "bar",
  "array" : [5, "val", {"some": "other"}, null, "append"]
}]=])

string(JSON result ERROR_VARIABLE error SET ${json2} array 100 [["append"]])
assert_json_equal("${error}" "${result}"
[=[{
  "foo" : "bar",
  "array" : [5, "val", {"some": "other"}, null, "append"]
}]=])
