# AND and OR are the same precedence
if(1 OR 0 AND 0) # equivalent to ((1 OR 0) AND 0)
  message(FATAL_ERROR "AND incorrectly evaluated before OR")
else()
  message(STATUS "OR and AND correctly evaluated left to right")
endif()
