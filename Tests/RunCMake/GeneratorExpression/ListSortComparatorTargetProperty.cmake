add_custom_target(a)
add_custom_target(b)
add_custom_target(c)
set_property(TARGET a PROPERTY MY_RANK 3)
set_property(TARGET b PROPERTY MY_RANK 1)
set_property(TARGET c PROPERTY MY_RANK 2)

# Sort the target names by their MY_RANK property, ascending.
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/out.txt"
  CONTENT "$<LIST:SORT,a;b;c,COMPARATOR,$<STRLESS:$<TARGET_PROPERTY:$<_0>,MY_RANK>,$<TARGET_PROPERTY:$<_1>,MY_RANK>>>\n")
