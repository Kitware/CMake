function(exit_macro)
  cmake_language(EXIT 9)
  message(FATAL_ERROR "This should not be reached!")
endfunction()

function(exit_function)
  exit_macro()
  message(FATAL_ERROR "This should not be reached!")
endfunction()

block()
  if(1)
    foreach(i IN ITEMS a b)
      while(1)
        exit_function()
        message(FATAL_ERROR "This should not be reached!")
      endwhile()
      message(FATAL_ERROR "This should not be reached!")
    endforeach()
    message(FATAL_ERROR "This should not be reached!")
  endif()
  message(FATAL_ERROR "This should not be reached!")
endblock()
message(FATAL_ERROR "This should not be reached!")
