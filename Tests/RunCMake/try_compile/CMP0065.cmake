enable_language(C)
set(obj "${CMAKE_C_OUTPUT_EXTENSION}")
if(BORLAND)
  set(pre -)
endif()
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS ${pre}BADFLAG${obj})

#-----------------------------------------------------------------------------
message("before try_compile with CMP0065 WARN-default")
try_compile(RESULT ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src.c
  OUTPUT_VARIABLE out
  )
string(REPLACE "\n" "\n  " out "  ${out}")
if(NOT RESULT)
  message(FATAL_ERROR "try_compile failed but should have passed:\n${out}")
elseif("x${out}" MATCHES "BADFLAG")
  message(FATAL_ERROR "try_compile output mentions BADFLAG:\n${out}")
else()
  message(STATUS "try_compile with CMP0065 WARN-default worked as expected")
endif()
message("after try_compile with CMP0065 WARN-default")

#-----------------------------------------------------------------------------
set(CMAKE_POLICY_WARNING_CMP0065 ON)
try_compile(RESULT ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src.c
  OUTPUT_VARIABLE out
  )
string(REPLACE "\n" "\n  " out "  ${out}")
if(NOT RESULT)
  message(FATAL_ERROR "try_compile failed but should have passed:\n${out}")
elseif("x${out}" MATCHES "BADFLAG")
  message(FATAL_ERROR "try_compile output mentions BADFLAG:\n${out}")
else()
  message(STATUS "try_compile with CMP0065 WARN-enabled worked as expected")
endif()

#-----------------------------------------------------------------------------
cmake_policy(SET CMP0065 OLD)
try_compile(RESULT ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src.c
  OUTPUT_VARIABLE out
  )
string(REPLACE "\n" "\n  " out "  ${out}")
if(NOT RESULT)
  message(FATAL_ERROR "try_compile failed but should have passed:\n${out}")
elseif("x${out}" MATCHES "BADFLAG")
  message(FATAL_ERROR "try_compile output mentions BADFLAG:\n${out}")
else()
  message(STATUS "try_compile with CMP0065 OLD worked as expected")
endif()

#-----------------------------------------------------------------------------
cmake_policy(SET CMP0065 NEW)
try_compile(RESULT ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src.c
  OUTPUT_VARIABLE out
  )
string(REPLACE "\n" "\n  " out "  ${out}")
if(RESULT)
  message(FATAL_ERROR "try_compile passed but should have failed:\n${out}")
elseif(NOT "x${out}" MATCHES "BADFLAG")
  message(FATAL_ERROR "try_compile did not fail with BADFLAG:\n${out}")
else()
  message(STATUS "try_compile with CMP0065 NEW worked as expected")
endif()
