function (itsok)
  message(OK!)
endfunction()

set (cmd INVOKE itsok)
cmake_command (${cmd})
