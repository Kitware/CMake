macro(wrap_angles in out)
  set(${out} "<${in}>")
endmacro()

set(mylist alpha bravo charlie)
list(TRANSFORM mylist APPLY wrap_angles)
