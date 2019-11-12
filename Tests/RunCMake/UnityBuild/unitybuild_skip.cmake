project(unitybuild_skip C)

set(srcs "")
foreach(s RANGE 1 8)
  set(src "${CMAKE_CURRENT_BINARY_DIR}/s${s}.c")
  file(WRITE "${src}" "int s${s}(void) { return 0; }\n")
  list(APPEND srcs "${src}")
endforeach()

add_library(tgt SHARED ${srcs})

set_target_properties(tgt PROPERTIES UNITY_BUILD ON)

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/s1.c
  PROPERTIES HEADER_FILE_ONLY ON)

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/s2.c
  PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/s3.c
  PROPERTIES COMPILE_OPTIONS "val")

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/s4.c
  PROPERTIES COMPILE_DEFINITIONS "val")

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/s5.c
  PROPERTIES COMPILE_FLAGS "val")

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/s6.c
  PROPERTIES INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}")
