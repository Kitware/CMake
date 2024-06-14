enable_testing()
foreach(i RANGE 1 6)
  add_test(NAME test${i} COMMAND ${CMAKE_COMMAND} -E true)
endforeach()
