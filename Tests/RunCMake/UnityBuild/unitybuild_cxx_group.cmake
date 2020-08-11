project(unitybuild_cxx CXX)

set(srcs "")
foreach(s RANGE 1 4)
  set(src "${CMAKE_CURRENT_BINARY_DIR}/s${s}.cxx")
  file(WRITE "${src}" "int s${s}(void) { return 0; }\n")
  list(APPEND srcs "${src}")
endforeach()

foreach(s RANGE 1 2)
  set(src "${CMAKE_CURRENT_BINARY_DIR}/odr${s}.cxx")
  file(WRITE "${src}" "namespace odr { int s${s}(void) { return 0; } }\n")
  list(APPEND srcs "${src}")
endforeach()

add_library(tgt SHARED ${srcs})

set_target_properties(tgt PROPERTIES UNITY_BUILD ON
                          UNITY_BUILD_MODE GROUP
                          )

set_source_files_properties(s1.cxx s2.cxx odr1.cxx
                            PROPERTIES UNITY_GROUP "a"
                            )
set_source_files_properties(s3.cxx s4.cxx odr2.cxx
                            PROPERTIES UNITY_GROUP "b"
                            )
