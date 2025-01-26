set(mylist 0000 1001 0002)

# OLD
cmake_policy(SET CMP0186 OLD)

unset(output)
list(TRANSFORM mylist REPLACE "^0" "" OUTPUT_VARIABLE output)
if (NOT output STREQUAL ";1001;2")
  message(FATAL_ERROR "TRANSFORM(REPLACE) is \"${output}\", expected is \";1001;2\"")
endif()

unset(output)
list(TRANSFORM mylist REPLACE "^(a|0)" "x" OUTPUT_VARIABLE output)
if (NOT output STREQUAL "xxxx;1001;xxx2")
  message(FATAL_ERROR "TRANSFORM(REPLACE) is \"${output}\", expected is \"xxxx;1001;xxx2\"")
endif()

unset(output)
list(TRANSFORM mylist REPLACE "(1|^)0" "x" OUTPUT_VARIABLE output)
if (NOT output STREQUAL "xxxx;xx1;xxx2")
  message(FATAL_ERROR "TRANSFORM(REPLACE) is \"${output}\", expected is \"xxxx;xx1;xxx2\"")
endif()

# NEW, same cases as above
cmake_policy(SET CMP0186 NEW)

unset(output)
list(TRANSFORM mylist REPLACE "^0" "" OUTPUT_VARIABLE output)
if (NOT output STREQUAL "000;1001;002")
  message(FATAL_ERROR "TRANSFORM(REPLACE) is \"${output}\", expected is \"000;1001;002\"")
endif()

unset(output)
list(TRANSFORM mylist REPLACE "^(a|0)" "x" OUTPUT_VARIABLE output)
if (NOT output STREQUAL "x000;1001;x002")
  message(FATAL_ERROR "TRANSFORM(REPLACE) is \"${output}\", expected is \"x000;1001;x002\"")
endif()

unset(output)
list(TRANSFORM mylist REPLACE "(1|^)0" "x" OUTPUT_VARIABLE output)
if (NOT output STREQUAL "x000;x01;x002")
  message(FATAL_ERROR "TRANSFORM(REPLACE) is \"${output}\", expected is \"x000;xx1;x002\"")
endif()
