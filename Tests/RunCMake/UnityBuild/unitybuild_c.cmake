project(unitybuild_c C)

set(srcs "")
foreach(s RANGE 1 8)
  set(src "${CMAKE_CURRENT_BINARY_DIR}/s${s}.c")
  file(WRITE "${src}" "int s${s}(void) { return 0; }\n")
  list(APPEND srcs "${src}")
endforeach()

add_library(tgt SHARED ${srcs})

set_target_properties(tgt PROPERTIES UNITY_BUILD ON)
