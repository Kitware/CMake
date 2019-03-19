function(update_x)
  message("${x}")
  math(EXPR y "${x} + 2")
  variable_watch(x update_x)
  set(x "${y}")
endfunction()

variable_watch(x update_x)
set(x 4)
