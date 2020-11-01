project(unitybuild_c_and_cxx C CXX)

set(srcs "")
foreach(s RANGE 1 8)
  set(src_c "${CMAKE_CURRENT_BINARY_DIR}/s${s}.c")
  file(WRITE "${src_c}" "int s${s}(void) { return 0; }\n")

  set(src_cxx "${CMAKE_CURRENT_BINARY_DIR}/s${s}.cxx")
  file(WRITE "${src_cxx}" "int s${s}(void) { return 0; }\n")

  list(APPEND srcs "${src_c}")
  list(APPEND srcs "${src_cxx}")
endforeach()

add_library(tgt SHARED ${srcs})

set_target_properties(tgt PROPERTIES UNITY_BUILD ON)
