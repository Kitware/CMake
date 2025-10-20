function(test_strip input expected)
  string(GENEX_STRIP "${input}" strip)
  if (NOT strip STREQUAL expected)
    message(FATAL_ERROR "message(GENEXP_STRIP \"${input}\")
evaluated to \"${strip}\"
expected \"${expected}\"")
  endif()
endfunction()

test_strip( # Simple case
  "$<BOOL:1>"
  ""
)
test_strip( # LHS contains generator expression
  "$<$<CONFIG:Release>:NDEBUG>;DEBUG"
  "DEBUG"
)
test_strip( # RHS contains generator expression
  "$<AND:1,$<BOOL:TRUE>>"
  ""
)
test_strip( # Empty and unfinished expressions
  "$<>$<$<>"
  "$<$<>"
)
test_strip( # Multiple independent expressions
  "$<IF:TRUE,TRUE,FALSE> / $<IF:TRUE,TRUE,FALSE>"
  " / "
)
test_strip( # Multiple : in one expression
  "$<1:2:3>"
  ""
)
test_strip( # Multiple case
  "1$<AND:1,0>2$<IF:$<$<BOOL:1>:$<CONFIG:RELEASE>>,TRUE,FALSE>3"
  "123"
)
test_strip( # No : inside of :
  "$<1:$<SEMICOLON>>1"
  "1"
)
