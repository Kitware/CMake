set(CMAKE_INTERMEDIATE_DIR_STRATEGY FULL CACHE STRING "" FORCE)

project(unitybuild_fortran Fortran)

# Fixed-form sources go to a `.F` unity file, free-form sources to `.F90`.
set(srcs "")
foreach(s RANGE 1 4)
  set(src "${CMAKE_CURRENT_BINARY_DIR}/s${s}.f")
  file(WRITE "${src}" "      subroutine s${s}()\n      end subroutine s${s}\n")
  list(APPEND srcs "${src}")
endforeach()
foreach(s RANGE 5 8)
  set(src "${CMAKE_CURRENT_BINARY_DIR}/s${s}.f90")
  file(WRITE "${src}" "subroutine s${s}()\nend subroutine s${s}\n")
  list(APPEND srcs "${src}")
endforeach()

add_library(tgt SHARED ${srcs})

set_target_properties(tgt PROPERTIES UNITY_BUILD ON)
