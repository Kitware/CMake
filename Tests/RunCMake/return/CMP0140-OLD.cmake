
cmake_policy(SET CMP0140 OLD)

function(FUNC)
  return(PROPAGATE VAR)
endfunction()

func()
