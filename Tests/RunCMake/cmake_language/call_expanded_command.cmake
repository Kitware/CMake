function (itsok)
  message(OK!)
endfunction()

set (cmd CALL itsok)
cmake_language (${empty} ${cmd})
