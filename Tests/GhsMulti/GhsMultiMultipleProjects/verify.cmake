# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#test project was generated
unset(fileName CACHE)
find_file(fileName lib3.gpj
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/lib3
  ${CMAKE_CURRENT_BINARY_DIR}/examples
  )

if (fileName)
  message("Found target lib3: ${fileName}")
else()
  message(SEND_ERROR "Could not find target lib3: ${fileName}")
endif()

#test project was generated
unset(fileName CACHE)
find_file (fileName exe3.gpj
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/exe3
  ${CMAKE_CURRENT_BINARY_DIR}/examples
  )

if (fileName)
  message("Found target exe3: ${fileName}")
else()
  message(SEND_ERROR "Could not find target exe3: ${fileName}")
endif()

#test project was not built
unset(fileName CACHE)
find_file (fileName lib3.a
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/lib3
  ${CMAKE_CURRENT_BINARY_DIR}/examples
  )

if (fileName)
  message(SEND_ERROR "Found target lib3: ${fileName}")
else()
  message("Could not find target lib3: ${fileName}")
endif()

unset(fileName CACHE)
find_file (fileName NAMES exe3.as exe3
  HINTS
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/exe3
  ${CMAKE_CURRENT_BINARY_DIR}/examples
  )

if (fileName)
  message(SEND_ERROR "Found target exe3: ${fileName}")
else()
  message("Could not find target exe3: ${fileName}")
endif()
