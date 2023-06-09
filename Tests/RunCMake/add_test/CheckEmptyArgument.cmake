if (NOT CMAKE_ARGV4 STREQUAL "A")
  message(FATAL_ERROR "wrong parsing of arguments")
endif()

if (NOT CMAKE_ARGV5 STREQUAL "")
  message(FATAL_ERROR "Empty argument was dropped")
endif()

if (NOT CMAKE_ARGV6 STREQUAL "B")
  message(FATAL_ERROR "wrong parsing of arguments")
endif()
