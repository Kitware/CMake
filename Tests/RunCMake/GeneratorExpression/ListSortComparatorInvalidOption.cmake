# An unrecognized trailing option on the COMPARATOR path goes through the
# NotRecognized branch of the shared option parser.
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/x.txt"
  CONTENT "$<LIST:SORT,a;b,COMPARATOR,$<STRLESS:$<_0>,$<_1>>,BOGUS:X>")
