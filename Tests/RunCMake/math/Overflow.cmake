foreach(expr IN ITEMS
    "-4 <<   1"
    "-4 >>   1"
    " 4 << -63"
    " 4 >> -63"
    " 4 <<  65"
    " 4 >>  65"
    " 0x7FFFFFFFFFFFFFFF + 1"
    "-0x7FFFFFFFFFFFFFFF - 2"
    " 0x7FFFFFFFFFFFFFFF * 2"
  )
  math(EXPR result "${expr}")
  message(STATUS "${expr}: ${result}")
endforeach()
