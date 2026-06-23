add_library(libA INTERFACE)
target_include_directories(libA INTERFACE /a/one /a/two)
add_library(libB INTERFACE)
target_include_directories(libB INTERFACE /b/one)

# Nested APPLY: the outer APPLY runs over targets; its body is an APPLY over
# each target's include directories that uppercases them.  The outer $<_0> (a
# target) feeds the inner list arg $<TARGET_PROPERTY:...>; the inner $<_0> (a
# directory) feeds $<UPPER_CASE> under the inner binding, while the outer
# binding stays intact.
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/nested.txt"
  CONTENT "$<LIST:TRANSFORM,libA;libB,APPLY,$<LIST:TRANSFORM,$<TARGET_PROPERTY:$<_0>,INTERFACE_INCLUDE_DIRECTORIES>,APPLY,$<UPPER_CASE:$<_0>>>>\n")

# Exact reference: the same result without the inner APPLY (uppercase each
# target's include dirs directly).  The nested APPLY must match it.
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/expected.txt"
  CONTENT "$<UPPER_CASE:$<TARGET_PROPERTY:libA,INTERFACE_INCLUDE_DIRECTORIES>>;$<UPPER_CASE:$<TARGET_PROPERTY:libB,INTERFACE_INCLUDE_DIRECTORIES>>\n")
