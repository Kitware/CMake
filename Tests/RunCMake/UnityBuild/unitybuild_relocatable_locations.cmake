project(unitybuild_relocatable_locations C)

# Binary path relative source file
set(srcs "")
foreach(s RANGE 1 3)
  set(src "${CMAKE_CURRENT_BINARY_DIR}/s${s}.c")
  file(WRITE "${src}" "int s${s}(void) { return 0; }\n")
  list(APPEND srcs "${src}")
endforeach()
foreach(s RANGE 1 3)
  set(src "${CMAKE_CURRENT_BINARY_DIR}/subFolder/sub${s}.c")
  file(WRITE "${src}" "int sub${s}(void) { return 0; }\n")
  list(APPEND srcs "${src}")
endforeach()

# Source path relative source file
list(APPEND srcs "${CMAKE_SOURCE_DIR}/f.c")
list(APPEND srcs "${CMAKE_SOURCE_DIR}/relocatable/foo.c")

add_library(tgt SHARED ${srcs})

set_target_properties(tgt PROPERTIES UNITY_BUILD ON
                                     UNITY_BUILD_RELOCATABLE TRUE)
