function(some_function_1)
  message(1)
endfunction()

function(some_function_2)
  message(2)
endfunction()

set(function_version 1)

cmake_language(CALL some_function_${function_version})
