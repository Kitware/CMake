project(unitybuild_cxx_absolute_path CXX)

set(srcs "")
foreach(s RANGE 1 3)
  set(src "${CMAKE_CURRENT_BINARY_DIR}/s${s}.cxx")
  file(WRITE "${src}" "int s${s}(void) { return 0; }\n")
  list(APPEND srcs "${src}")
endforeach()

add_library(tgt SHARED ${srcs})

set_target_properties(tgt PROPERTIES UNITY_BUILD ON
                                     UNITY_BUILD_RELOCATABLE FALSE)
