set(CMAKE_CUDA_ARCHITECTURES all-major)
project(unitybuild_c_and_cxx CUDA CXX)

set(srcs f.cu)
foreach(s RANGE 1 8)
  set(src_cu "${CMAKE_CURRENT_BINARY_DIR}/s${s}.cu")
  file(WRITE "${src_cu}" "
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

  list(APPEND srcs "${src_cu}")
  list(APPEND srcs "${src_cxx}")
endforeach()



add_library(tgt SHARED ${srcs})

set_target_properties(tgt PROPERTIES UNITY_BUILD ON
                          UNITY_BUILD_MODE GROUP
                          #UNITY_BUILD_BATCH_SIZE will be ignored
                          UNITY_BUILD_BATCH_SIZE 2)

set_source_files_properties(s1.cu s2.cu s3.cu s4.cu
                            s1.cxx s2.cxx s3.cxx s4.cxx
                            PROPERTIES UNITY_GROUP "a"
                            )
set_source_files_properties(s5.cu s6.cu s7.cu s8.cu
                            s5.cxx s6.cxx s7.cxx s8.cxx
                            PROPERTIES UNITY_GROUP "b"
                            )
