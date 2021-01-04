message(STATUS "testname='${testname}'")

function(do_end content)
  file(WRITE "${dir}/${testname}.cmake" "${content}")
  execute_process(COMMAND ${CMAKE_COMMAND} -P "${dir}/${testname}.cmake"
    RESULT_VARIABLE rv)
  if(NOT rv EQUAL 0)
    message(FATAL_ERROR "${testname} failed")
  endif()
endfunction()

if(testname STREQUAL bad_else) # fail
  do_end("else()\n")

elseif(testname STREQUAL bad_elseif) # fail
  do_end("elseif()\n")

elseif(testname STREQUAL bad_endforeach) # fail
  do_end("endforeach()\n")

elseif(testname STREQUAL bad_endfunction) # fail
  do_end("endfunction()\n")

elseif(testname STREQUAL bad_endif) # fail
  do_end("cmake_minimum_required(VERSION 2.8.12)\nendif()\n")

elseif(testname STREQUAL endif_low_min_version) # fail
  do_end("cmake_minimum_required(VERSION 1.2)\nendif()\n")

elseif(testname STREQUAL endif_no_min_version) # fail
  do_end("endif()\n")

elseif(testname STREQUAL bad_endmacro) # fail
  do_end("endmacro()\n")

elseif(testname STREQUAL bad_endwhile) # fail
  do_end("endwhile()\n")

else() # fail
  message(FATAL_ERROR "testname='${testname}' - error: no such test in '${CMAKE_CURRENT_LIST_FILE}'")

endif()
