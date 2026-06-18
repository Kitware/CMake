# A body that errors for an element must fail the whole expression, not be
# silently dropped.  $<LIST:GET,...,9> is out of range for a single-element
# operand.
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/x.txt"
  CONTENT "$<LIST:TRANSFORM,a;b,APPLY,$<LIST:GET,$<_0>,9>>")
