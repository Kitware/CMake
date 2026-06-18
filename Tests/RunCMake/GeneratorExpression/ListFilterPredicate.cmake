set(input "alpha;beta;gamma")
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/out.txt"
  CONTENT "$<LIST:FILTER,${input},INCLUDE,PREDICATE,$<STREQUAL:$<_0>,beta>>\n")
