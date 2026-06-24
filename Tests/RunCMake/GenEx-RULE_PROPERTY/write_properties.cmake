
math(EXPR end "${CMAKE_ARGC} - 1")

file(WRITE "${OUTPUT_FILE}" "# Rule Properties\n")

foreach (index RANGE 5 ${end})
  if (CMAKE_ARGV${index} MATCHES "^([A-Z_]+)=(.*)$")
    file(APPEND "${OUTPUT_FILE}" "set(${CMAKE_MATCH_1} \"${CMAKE_MATCH_2}\")\n")
  endif()
endforeach()
