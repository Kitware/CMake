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
  // This is a comment
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
  "object": {
    "foo": "bar"  // This is an inline comment
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

# test GET
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

string(JSON result GET "\"Hello\"")
assert_strequal("${result}" Hello)
string(JSON result GET "5")
assert_strequal("${result}" 5)
string(JSON result GET "null")
assert_strequal("${result}" "")
string(JSON result ERROR_VARIABLE error GET "{}")
assert_json_equal("${error}" "${result}" "{}")
string(JSON result ERROR_VARIABLE error GET "[]")
assert_json_equal("${error}" "${result}" "[]")

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

# Test GET_RAW
string(JSON result GET_RAW "${json1}" values null)
assert_strequal("${result}" null)
string(JSON result GET_RAW "${json1}" values number)
assert_strequal("${result}" 5)
string(JSON result GET_RAW "${json1}" values string)
assert_strequal("${result}" "\"foo\"")
string(JSON result GET_RAW "${json1}" values false)
assert_strequal("${result}" false)
string(JSON result GET_RAW "${json1}" values true)
assert_strequal("${result}" true)
string(JSON result ERROR_VARIABLE error GET_RAW "${json1}" array)
assert_json_equal("${error}" "${result}" [=[ [5, "val", {"some": "other"}, null] ]=])
string(JSON result ERROR_VARIABLE error GET_RAW "${json1}" object)
assert_json_equal("${error}" "${result}" [=[ { "foo": "bar" } ]=])

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
string(JSON result TYPE "null")
assert_strequal("${result}" NULL)
string(JSON result TYPE "5")
assert_strequal("${result}" NUMBER)
string(JSON result TYPE "\"Hello\"")
assert_strequal("${result}" STRING)

# Test LENGTH
string(JSON result ERROR_VARIABLE error LENGTH "${json1}")
assert_strequal("${result}" 6)
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

# Test STRING_ENCODE
string(JSON result STRING_ENCODE Hello)
assert_strequal("${result}" "\"Hello\"")
string(JSON result STRING_ENCODE "\"Hello\"")
assert_strequal("${result}" "\"\\\"Hello\\\"\"")
string(JSON result STRING_ENCODE null)
assert_strequal("${result}" "\"null\"")
string(JSON result STRING_ENCODE 0)
assert_strequal("${result}" "\"0\"")
string(JSON result STRING_ENCODE false)
assert_strequal("${result}" "\"false\"")
string(JSON result STRING_ENCODE {})
assert_strequal("${result}" "\"{}\"")
string(JSON result STRING_ENCODE [])
assert_strequal("${result}" "\"[]\"")

string(JSON result PARTIAL_EQUAL
[=[
{
  "foo":"bar"
}
]=]
[=[
{
  "foo": "bar",
  "extra": 1
}
]=])

if(NOT result)
  message(SEND_ERROR "Expected ON got ${result}")
endif()

string(JSON result PARTIAL_EQUAL
[=[
{
  "foo":"bar"
}
]=]
[=[
{
  "foo1": "bar"
}
]=])
if(result)
  message(SEND_ERROR "EXPECTED OFF got ${result}")
endif()

string(JSON result PARTIAL_EQUAL
[=[
{
  "types" : {
    "number" : 5
  }
}
]=]
[=[
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
  "object": {
    "foo": "bar"
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
if(NOT result)
  message(SEND_ERROR "EXPECTED ON got ${result} for nested subset")
endif()

string(JSON result PARTIAL_EQUAL
[=[
{
  "types" : {
    "number" : 6
  }
}
]=]
[=[
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
  "object": {
    "foo": "bar"
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
if(result)
  message(SEND_ERROR "EXPECTED OFF got ${result} for nested mismatch")
endif()

string(JSON result PARTIAL_EQUAL
[=[
[
  2,
  1
]
]=]
[=[
[
  1,
  2,
  3
]
]=])
if(NOT result)
  message(SEND_ERROR "Expected ON got ${result} for unordered array subset")
endif()

string(JSON result PARTIAL_EQUAL
[=[
[
  2,
  4
]
]=]
[=[
[
  1,
  2,
  3
]
]=])
if(result)
  message(SEND_ERROR "Expected OFF got ${result} for missing array element")
endif()

string(JSON result PARTIAL_EQUAL
[=[
[
  1,
  2,
  3
]
]=]
[=[
[
  1,
  2
]
]=])
if(result)
  message(SEND_ERROR "Expected OFF got ${result} for pattern larger than actual")
endif()

string(JSON result PARTIAL_EQUAL
  [=[
[
  1,
  1
]
]=]
[=[
[
  1,
  2,
  1
]
]=])
if(NOT result)
  message(SEND_ERROR "EXPECTED ON got ${result} for duplicate match")
endif()

string(JSON result PARTIAL_EQUAL
  [=[
[
  {
    "id":2
  },
  {
    "id":1
  }
]
]=]
[=[
[
  {
    "id":1,
    "name":"A"
  },
  {
    "id":2,
    "name":"B"
  },
  {
    "id":3,
    "name":"C"
  }
]
]=])
if(NOT result)
  message(SEND_ERROR "EXPECTED ON got ${result} for array of object subset")
endif()

string(JSON result PARTIAL_EQUAL
[=[
[
  {
    "id":4
  }
]
]=]
[=[
[
  {
    "id":1
  },
  {
    "id":2
  }
]
]=])
if(result)
  message(SEND_ERROR "Expected OFF got ${result} for array of object mismatch")
endif()

string(JSON result PARTIAL_EQUAL
[=[
{
  "tags": [
    "b",
    "a"
  ]
}
]=]
[=[
{
  "tags": [
    "a",
    "b",
    "c"
  ],
  "other": 123
}
]=])
if(NOT result)
  message(SEND_ERROR "EXPECTED ON got ${result} for object+array partial match")
endif()

string(JSON result PARTIAL_EQUAL
[=[
{
  "tags": [
    "a",
    "a"
  ]
}
]=]
[=[
{
  "tags": [
    "a",
    "b"
  ]
}
]=])
if(result)
  message(SEND_ERROR "Expected OFF got ${result} for duplicate array mismatch in object")
endif()

# Test ARRAY_SPLIT

# Split the top-level array in <json>, assert the element count matches LENGTH
# and each element is semantically equal to the corresponding source element,
# then return the split list in <out-var> (if given).  Elements are always read
# back through a list-aware reader so the encoded list is exercised end-to-end.
function(assert_array_split json)
  string(JSON _split ERROR_VARIABLE _err ARRAY_SPLIT "${json}")
  if(_err)
    message(SEND_ERROR "Unexpected ARRAY_SPLIT error: ${_err}\n for: ${json}")
    return()
  endif()
  string(JSON _len LENGTH "${json}")
  list(LENGTH _split _n)
  if(NOT _n EQUAL _len)
    message(SEND_ERROR "ARRAY_SPLIT gave ${_n} elements, expected ${_len}\n for: ${json}")
    return()
  endif()
  set(_i 0)
  foreach(_element IN LISTS _split)
    string(JSON _src GET_RAW "${json}" ${_i})
    string(JSON _eq ERROR_VARIABLE _eqerr EQUAL "${_element}" "${_src}")
    if(_eqerr)
      message(SEND_ERROR "ARRAY_SPLIT element ${_i} is not valid JSON: ${_eqerr}\n element: ${_element}")
    elseif(NOT _eq)
      message(SEND_ERROR "ARRAY_SPLIT element ${_i}\n ${_element}\n not equal to source\n ${_src}")
    endif()
    math(EXPR _i "${_i} + 1")
  endforeach()
  if(ARGC GREATER 1)
    set(${ARGV1} "${_split}" PARENT_SCOPE)
  endif()
endfunction()

# Basic array of objects: cardinality, order, and member access.
assert_array_split([=[
[
  { "A": 1, "B": "one" },
  { "A": 2, "B": "two" },
  { "A": 3, "B": "three" }
]
]=] elements)
list(GET elements 0 element0)
string(JSON a GET "${element0}" A)
assert_strequal("${a}" 1)
string(JSON b GET "${element0}" B)
assert_strequal("${b}" one)
list(GET elements 2 element2)
string(JSON b GET "${element2}" B)
assert_strequal("${b}" three)

# Empty array, top-level and via a path.
assert_array_split("[]" empty)
list(LENGTH empty emptyLen)
assert_strequal("${emptyLen}" 0)
string(JSON pathEmpty ERROR_VARIABLE error ARRAY_SPLIT [=[{"data":[]}]=] data)
if(error)
  message(SEND_ERROR "Unexpected error: ${error}")
endif()
list(LENGTH pathEmpty pathEmptyLen)
assert_strequal("${pathEmptyLen}" 0)

# Path-located array.
set(doc [=[{ "outer": { "data": [10, 20, 30] } }]=])
string(JSON pathSplit ERROR_VARIABLE error ARRAY_SPLIT "${doc}" outer data)
if(error)
  message(SEND_ERROR "Unexpected error: ${error}")
endif()
list(LENGTH pathSplit pathSplitLen)
assert_strequal("${pathSplitLen}" 3)
list(GET pathSplit 1 pathElement1)
assert_strequal("${pathElement1}" 20)

# Scalars: cardinality preserved (no empty-element collapsing) and types kept.
assert_array_split([=[[null, false, 0, ""]]=] scalars)
list(LENGTH scalars scalarsLen)
assert_strequal("${scalarsLen}" 4)
list(GET scalars 0 scalar0)
string(JSON scalarType TYPE "${scalar0}")
assert_strequal("${scalarType}" NULL)
list(GET scalars 1 scalar1)
string(JSON scalarType TYPE "${scalar1}")
assert_strequal("${scalarType}" BOOLEAN)
list(GET scalars 2 scalar2)
string(JSON scalarType TYPE "${scalar2}")
assert_strequal("${scalarType}" NUMBER)
list(GET scalars 3 scalar3)
string(JSON scalarType TYPE "${scalar3}")
assert_strequal("${scalarType}" STRING)

# Numbers: integer, real, exponent, and large integer round-trip.
assert_array_split([=[[1, 1000.0, 1e3, 1234567890]]=])

# Nested arrays and objects: structural brackets left intact.
assert_array_split([=[[[1,2],[3,4]]]=] nested)
list(GET nested 0 nested0)
string(JSON nestedType TYPE "${nested0}")
assert_strequal("${nestedType}" ARRAY)
string(JSON nestedVal GET "${nested0}" 1)
assert_strequal("${nestedVal}" 2)
assert_array_split([=[[{"x":1},{"y":2}]]=] objects)
list(GET objects 1 objects1)
string(JSON objectsType TYPE "${objects1}")
assert_strequal("${objectsType}" OBJECT)
string(JSON objectsVal GET "${objects1}" y)
assert_strequal("${objectsVal}" 2)

# Whitespace and object-member-order canonicalization compare EQUAL.
assert_array_split([=[[  {  "b" : 1 ,  "a" : 2  }  ]]=])

# Adversarial: ';' inside a string value must not split the element.
assert_array_split([=[[{"cmd":"a;b"},{"d":4}]]=] semicolon)
list(LENGTH semicolon semicolonLen)
assert_strequal("${semicolonLen}" 2)
list(GET semicolon 0 semicolon0)
string(JSON semicolonVal GET "${semicolon0}" cmd)
assert_strequal("${semicolonVal}" "a;b")

# Adversarial: an unbalanced '[' or ']' inside a string must not merge elements.
assert_array_split([=[[{"a":"["},{"b":2}]]=] openBracket)
list(LENGTH openBracket openBracketLen)
assert_strequal("${openBracketLen}" 2)
list(GET openBracket 0 openBracket0)
string(JSON openBracketVal GET "${openBracket0}" a)
assert_strequal("${openBracketVal}" "[")
assert_array_split([=[[{"a":"]"},{"b":2}]]=] closeBracket)
list(LENGTH closeBracket closeBracketLen)
assert_strequal("${closeBracketLen}" 2)
list(GET closeBracket 0 closeBracket0)
string(JSON closeBracketVal GET "${closeBracket0}" a)
assert_strequal("${closeBracketVal}" "]")

# Adversarial: several brackets in one string value.
assert_array_split([=[[{"s":"[[]]["},{"z":0}]]=] brackets)
list(GET brackets 0 brackets0)
string(JSON bracketsVal GET "${brackets0}" s)
assert_strequal("${bracketsVal}" "[[]][")

# Adversarial: a bracket in an object *key* (keys are strings too).
assert_array_split([=[[{"[k]":1},{"y":2}]]=] keyBracket)
list(GET keyBracket 0 keyBracket0)
string(JSON keyBracketVal GET "${keyBracket0}" "[k]")
assert_strequal("${keyBracketVal}" 1)
string(JSON keyBracketName MEMBER "${keyBracket0}" 0)
assert_strequal("${keyBracketName}" "[k]")

# Adversarial: ';' adjacent to backslashes (value is a\;b).
assert_array_split([==[[{"cmd":"a\\;b"},{"d":4}]]==] semiBackslash)
list(GET semiBackslash 0 semiBackslash0)
string(JSON semiBackslashVal GET "${semiBackslash0}" cmd)
assert_strequal("${semiBackslashVal}" [==[a\;b]==])

# Adversarial: an escaped quote before a bracket (value is x"[y).
assert_array_split([==[[{"a":"x\"[y"},{"b":2}]]==] quoteBracket)
list(GET quoteBracket 0 quoteBracket0)
string(JSON quoteBracketVal GET "${quoteBracket0}" a)
assert_strequal("${quoteBracketVal}" [==[x"[y]==])

# Adversarial: a string value ending in an escaped backslash (value is x\).
assert_array_split([==[[{"a":"x\\"},{"b":2}]]==] endBackslash)
list(GET endBackslash 0 endBackslash0)
string(JSON endBackslashVal GET "${endBackslash0}" a)
assert_strequal("${endBackslashVal}" [==[x\]==])

# Adversarial: literal "\u005B" text (six characters) must stay unchanged.
assert_array_split([==[[{"a":"\\u005B"},{"b":2}]]==] literalEscape)
list(GET literalEscape 0 literalEscape0)
string(JSON literalEscapeVal GET "${literalEscape0}" a)
assert_strequal("${literalEscapeVal}" [==[\u005B]==])

# Unicode survives compact re-serialization (reuse the unicode fixture).
file(READ ${CMAKE_CURRENT_LIST_DIR}/json/unicode.json unicode)
assert_array_split("[${unicode}]" unicodeSplit)
list(GET unicodeSplit 0 unicodeElement)
string(JSON unicodeVal GET "${unicodeElement}" datalinkescape)
string(JSON unicodeSrc GET "${unicode}" datalinkescape)
assert_strequal("${unicodeVal}" "${unicodeSrc}")

# Error handling with ERROR_VARIABLE: non-array target must mirror the NOTFOUND
# form that LENGTH produces for the same input, and report an ARRAY message.

# Non-array (string) at top level.
string(JSON asResult ERROR_VARIABLE asError ARRAY_SPLIT "\"text\"")
string(JSON lenResult ERROR_VARIABLE lenError LENGTH "\"text\"")
assert_strequal("${asResult}" "${lenResult}")
assert_strequal("${asError}" "ARRAY_SPLIT needs to be called with an element of type ARRAY, got STRING")

# Object at top level is also rejected (LENGTH would accept it, so no cross-check
# on the message, but the NOTFOUND form matches the string case above).
string(JSON objResult ERROR_VARIABLE objError ARRAY_SPLIT "{}")
assert_strequal("${objResult}" "${asResult}")
assert_strequal("${objError}" "ARRAY_SPLIT needs to be called with an element of type ARRAY, got OBJECT")

# Non-array (string) at a path.
string(JSON pathResult ERROR_VARIABLE pathError ARRAY_SPLIT [=[{"data":"text"}]=] data)
string(JSON pathLenResult ERROR_VARIABLE pathLenError LENGTH [=[{"data":"text"}]=] data)
assert_strequal("${pathResult}" "${pathLenResult}")
assert_strequal("${pathResult}" "data-NOTFOUND")
assert_strequal("${pathError}" "ARRAY_SPLIT needs to be called with an element of type ARRAY, got STRING")

# Non-existent path mirrors LENGTH's <path>-NOTFOUND and reports an error.
string(JSON missResult ERROR_VARIABLE missError ARRAY_SPLIT [=[{"a":1}]=] b)
string(JSON missLenResult ERROR_VARIABLE missLenError LENGTH [=[{"a":1}]=] b)
assert_strequal("${missResult}" "${missLenResult}")
assert_strequal("${missResult}" "b-NOTFOUND")
if(NOT missError)
  message(SEND_ERROR "Expected an error for a non-existent ARRAY_SPLIT path")
endif()
