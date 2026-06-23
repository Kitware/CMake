set(input "net;audio;video")
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/out.txt"
  CONTENT "$<LIST:TRANSFORM,${input},APPLY,$<UPPER_CASE:$<_0>>>\n")
