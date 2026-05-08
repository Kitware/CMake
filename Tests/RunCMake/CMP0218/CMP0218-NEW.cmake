cmake_policy(SET CMP0218 NEW)

function(expect NAME ACTUAL EXPECTED)
  if(NOT "${ACTUAL}" STREQUAL "${EXPECTED}")
    message(SEND_ERROR "${NAME} is '${ACTUAL}'; should be '${EXPECTED}'")
  endif()
endfunction()

if(DEFINED CMAKE_WARN_DEPRECATED)
  message(SEND_ERROR "CMAKE_WARN_DEPRECATED should not be defined")
endif()

if(DEFINED CMAKE_ERROR_DEPRECATED)
  message(SEND_ERROR "CMAKE_ERROR_DEPRECATED should not be defined")
endif()

cmake_diagnostic(GET CMD_DEPRECATED action)
if(NOT "${action}" STREQUAL "WARN")
  message(SEND_ERROR "CMD_DEPRECATED is ${action}; should be WARN")
endif()

# -----------------------------------------------------------------------------
# Setting CMAKE_WARN_DEPRECATED should have no effect on CMD_DEPRECATED
block()
  set(CMAKE_WARN_DEPRECATED OFF)

  cmake_diagnostic(GET CMD_DEPRECATED action)
  if(NOT "${action}" STREQUAL "WARN")
    message(SEND_ERROR "CMD_DEPRECATED is ${action}; should be WARN")
  endif()
endblock()

# -----------------------------------------------------------------------------
# Setting CMAKE_ERROR_DEPRECATED should have no effect on CMD_DEPRECATED
block()
  set(CMAKE_ERROR_DEPRECATED ON)

  cmake_diagnostic(GET CMD_DEPRECATED action)
  if(NOT "${action}" STREQUAL "WARN")
    message(SEND_ERROR "CMD_DEPRECATED is ${action}; should be WARN")
  endif()
endblock()

# -----------------------------------------------------------------------------
# Changing CMD_DEPRECATED should have no effect on variables
block()
  cmake_diagnostic(SET CMD_DEPRECATED IGNORE)

  if(DEFINED CMAKE_WARN_DEPRECATED)
    message(SEND_ERROR "CMAKE_WARN_DEPRECATED should not be defined")
  endif()

  if(DEFINED CMAKE_ERROR_DEPRECATED)
    message(SEND_ERROR "CMAKE_ERROR_DEPRECATED should not be defined")
  endif()

  expect(CMAKE_WARN_DEPRECATED "${CMAKE_WARN_DEPRECATED}" "")
  expect(CMAKE_ERROR_DEPRECATED "${CMAKE_ERROR_DEPRECATED}" "")

  cmake_diagnostic(SET CMD_DEPRECATED SEND_ERROR)

  if(DEFINED CMAKE_WARN_DEPRECATED)
    message(SEND_ERROR "CMAKE_WARN_DEPRECATED should not be defined")
  endif()

  if(DEFINED CMAKE_ERROR_DEPRECATED)
    message(SEND_ERROR "CMAKE_ERROR_DEPRECATED should not be defined")
  endif()

  expect(CMAKE_WARN_DEPRECATED "${CMAKE_WARN_DEPRECATED}" "")
  expect(CMAKE_ERROR_DEPRECATED "${CMAKE_ERROR_DEPRECATED}" "")
endblock()
