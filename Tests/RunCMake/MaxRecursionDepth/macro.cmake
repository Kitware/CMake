macro(recursive x)
  message("${x}")
  math(EXPR y "${x} + 1")
  recursive(${y})
endmacro()

recursive(3)
