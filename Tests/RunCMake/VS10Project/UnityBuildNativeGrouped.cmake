project(unitybuild_c C)

set(srcs "")
foreach(s RANGE 1 8)
  set(src "${CMAKE_CURRENT_BINARY_DIR}/s${s}.c")
  file(WRITE "${src}" "int s${s}(void) { return 0; }\n")
  list(APPEND srcs "${src}")
endforeach()

add_library(tgt SHARED ${srcs})

set_target_properties(tgt PROPERTIES UNITY_BUILD ON UNITY_BUILD_MODE GROUP)

set_source_files_properties(s1.c s2.c s3.c s4.c
                            PROPERTIES UNITY_GROUP "poolA"
                            )

set_source_files_properties(s5.c s6.c s7.c
                            PROPERTIES UNITY_GROUP "poolB"
                            )
