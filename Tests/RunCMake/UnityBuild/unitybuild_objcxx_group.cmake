project(unitybuild_objcxx_group OBJCXX)

set(srcs "")
foreach(s RANGE 1 4)
  set(src "${CMAKE_CURRENT_BINARY_DIR}/s${s}.mm")
  file(WRITE "${src}" "int s${s}(void) { return 0; }\n")
  list(APPEND srcs "${src}")
endforeach()

foreach(s RANGE 1 2)
  set(src "${CMAKE_CURRENT_BINARY_DIR}/odr${s}.mm")
  file(WRITE "${src}" "namespace odr { int s${s}(void) { return 0; } }\n")
  list(APPEND srcs "${src}")
endforeach()

add_library(tgt SHARED ${srcs})

set_target_properties(tgt PROPERTIES UNITY_BUILD ON
                          UNITY_BUILD_MODE GROUP
                          )

set_source_files_properties(s1.mm s2.mm odr1.mm
                            PROPERTIES UNITY_GROUP "a"
                            )
set_source_files_properties(s3.mm s4.mm odr2.mm
                            PROPERTIES UNITY_GROUP "b"
                            )
