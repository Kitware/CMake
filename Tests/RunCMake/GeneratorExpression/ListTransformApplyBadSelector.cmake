# 'AT' with no following index is a malformed selector; APPLY must report the
# same diagnostic the canned TRANSFORM actions do.
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/x.txt"
  CONTENT "$<LIST:TRANSFORM,a;b,APPLY,$<_0>,AT>")
