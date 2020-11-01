project(unitybuild_c_no_unity_build C)

set(srcs "")
foreach(s RANGE 1 8)
  set(src "${CMAKE_CURRENT_BINARY_DIR}/s${s}.c")
  file(WRITE "${src}" "int s${s}(void) { return 0; }\n")
  list(APPEND srcs "${src}")
endforeach()

add_library(tgt SHARED ${srcs})

#These should be ignored as UNITY_BUILD is off
set_target_properties(tgt PROPERTIES UNITY_BUILD_MODE GROUP)
set_source_files_properties(s1.c s2.c s3.c s4.c s5.c s6.c s7.c s8.c
                            PROPERTIES UNITY_GROUP "a"
                            )
