# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#test project was generated
unset(fileName CACHE)
find_file (fileName lib1.gpj
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/lib1
  )

if (fileName)
  message("Found target lib1: ${fileName}")
else()
  message(SEND_ERROR "Could not find target lib1: ${fileName}")
endif()

#test project was built
unset(fileName CACHE)
find_file (fileName lib1.a
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/lib1
  )

if (fileName)
  message(SEND_ERROR "Found target lib1: ${fileName}")
else()
  message("Could not find target lib1: ${fileName}")
endif()

#test project was generated
unset(fileName CACHE)
find_file (fileName lib2.gpj
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/lib2
  )

if (fileName)
  message("Found target lib2 ${fileName}")
else()
  message(SEND_ERROR "Could not find target lib2: ${fileName}")
endif()

#test project was built
unset(fileName CACHE)
find_file (fileName lib2.a
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/lib2
  )

if (fileName)
  message(SEND_ERROR "Found target lib2: ${fileName}")
else()
  message("Could not find target lib2: ${fileName}")
endif()
