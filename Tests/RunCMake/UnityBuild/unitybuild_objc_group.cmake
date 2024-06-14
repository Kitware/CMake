project(unitybuild_objc_group OBJC)

set(srcs "")
foreach(s RANGE 1 4)
  set(src "${CMAKE_CURRENT_BINARY_DIR}/s${s}.m")
  file(WRITE "${src}" "int s${s}(void) { return 0; }\n")
  list(APPEND srcs "${src}")
endforeach()

foreach(s RANGE 1 2)
  set(src "${CMAKE_CURRENT_BINARY_DIR}/odr${s}.m")
  file(WRITE "${src}" "namespace odr { int s${s}(void) { return 0; } }\n")
  list(APPEND srcs "${src}")
endforeach()

add_library(tgt SHARED ${srcs})

set_target_properties(tgt PROPERTIES UNITY_BUILD ON
                          UNITY_BUILD_MODE GROUP
                          )

set_source_files_properties(s1.m s2.m odr1.m
                            PROPERTIES UNITY_GROUP "a"
                            )
set_source_files_properties(s3.m s4.m odr2.m
                            PROPERTIES UNITY_GROUP "b"
                            )
