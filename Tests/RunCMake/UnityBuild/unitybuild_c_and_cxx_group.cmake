project(unitybuild_c_and_cxx C CXX)

set(srcs f.c)
foreach(s RANGE 1 8)
  set(src_c "${CMAKE_CURRENT_BINARY_DIR}/s${s}.c")
  file(WRITE "${src_c}" "
int f(int);\n
int s${s}(void) { return f(${s}); }\n"
  )

  set(src_cxx "${CMAKE_CURRENT_BINARY_DIR}/s${s}.cxx")
  file(WRITE "${src_cxx}" "
extern \"C\" { \n
  int f(int); \n
}\n
  int s${s}(void) { return f(${s}); }\n"
  )

  list(APPEND srcs "${src_c}")
  list(APPEND srcs "${src_cxx}")
endforeach()



add_library(tgt SHARED ${srcs})

set_target_properties(tgt PROPERTIES UNITY_BUILD ON
                          UNITY_BUILD_MODE GROUP
                          #UNITY_BUILD_BATCH_SIZE will be ignored
                          UNITY_BUILD_BATCH_SIZE 2)

set_source_files_properties(s1.c s2.c s3.c s4.c
                            s1.cxx s2.cxx s3.cxx s4.cxx
                            PROPERTIES UNITY_GROUP "a"
                            )
set_source_files_properties(s5.c s6.c s7.c s8.c
                            s5.cxx s6.cxx s7.cxx s8.cxx
                            PROPERTIES UNITY_GROUP "b"
                            )
