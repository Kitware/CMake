# $<_1> requires a binary binding (e.g. SORT COMPARATOR); using it in a unary
# APPLY body, which binds only $<_0>, is an error.
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/x.txt"
  CONTENT "$<LIST:TRANSFORM,a;b,APPLY,X$<_1>Y>")
