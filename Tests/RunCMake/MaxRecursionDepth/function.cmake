function(recursive x)
  message("${x}")
  math(EXPR y "${x} + 1")
  recursive(${y})
endfunction()

recursive(3)
