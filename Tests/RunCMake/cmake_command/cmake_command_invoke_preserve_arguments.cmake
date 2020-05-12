function(foo arg1 arg2)
  math(EXPR last "${ARGC} - 1")
  foreach(i RANGE 0 ${last})
    message("[${ARGV${i}}]")
  endforeach()
endfunction()

message("foo(...)")
foo("a;b" "c;d")

message("cmake_command(INVOKE foo ...)")
cmake_command(INVOKE foo "a;b" "c;d")
