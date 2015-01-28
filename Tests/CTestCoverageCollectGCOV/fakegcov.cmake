foreach(I RANGE 0 ${CMAKE_ARGC})
  if("${CMAKE_ARGV${I}}" MATCHES ".*\\.gcda")
    set(gcda_file "${CMAKE_ARGV${I}}")
  endif()
endforeach()
get_filename_component(gcda_file ${gcda_file} NAME_WE)
file(WRITE "${CMAKE_SOURCE_DIR}/${gcda_file}.gcov"
"fake gcov file")
