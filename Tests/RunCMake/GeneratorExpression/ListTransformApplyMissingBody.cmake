# APPLY with no body argument is an error.
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/x.txt"
  CONTENT "$<LIST:TRANSFORM,a;b,APPLY>")
