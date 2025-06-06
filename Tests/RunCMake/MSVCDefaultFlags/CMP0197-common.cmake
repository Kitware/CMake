enable_language(C)

# Make sure the compile command is not hidden.
string(REPLACE "${CMAKE_START_TEMP_FILE}" "" CMAKE_C_LINK_EXECUTABLE "${CMAKE_C_LINK_EXECUTABLE}")
string(REPLACE "${CMAKE_END_TEMP_FILE}" "" CMAKE_C_LINK_EXECUTABLE "${CMAKE_C_LINK_EXECUTABLE}")

cmake_policy(GET CMP0197 cmp0197)
foreach(t EXE SHARED MODULE STATIC)
  if(cmp0197 STREQUAL "NEW")
    if("${CMAKE_${t}_LINKER_FLAGS}" MATCHES "([/-][Mm][Aa][Cc][Hh][Ii][Nn][Ee]:)")
      message(SEND_ERROR "CMAKE_${t}_LINKER_FLAGS has '${CMAKE_MATCH_1}' under NEW behavior")
    endif()
  else()
    if(NOT " ${CMAKE_${t}_LINKER_FLAGS} " MATCHES "[ ,]/machine:[A-Za-z0-9_]+ ")
      message(SEND_ERROR "CMAKE_${t}_LINKER_FLAGS does not have '/machine:' under OLD behavior")
    endif()
  endif()
endforeach()

add_executable(main main.c)
