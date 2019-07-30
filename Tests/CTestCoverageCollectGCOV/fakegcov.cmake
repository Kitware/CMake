function(create_gcov_file gcda_full_path)
  get_filename_component(gcda_name ${gcda_full_path} NAME)
  string(REPLACE ".gcda" ".gcov" gcov_name "${gcda_name}")

  file(STRINGS "${gcda_full_path}" source_file LIMIT_COUNT 1 ENCODING UTF-8)

  file(WRITE "${CMAKE_SOURCE_DIR}/${gcov_name}"
    "        -:    0:Source:${source_file}"
  )
endfunction()

foreach(I RANGE 0 ${CMAKE_ARGC})
  if("${CMAKE_ARGV${I}}" MATCHES ".*\\.gcda")
    set(gcda_file "${CMAKE_ARGV${I}}")
    create_gcov_file(${gcda_file})
  endif()
endforeach()
