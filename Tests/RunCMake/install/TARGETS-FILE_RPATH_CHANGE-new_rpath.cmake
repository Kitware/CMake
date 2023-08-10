enable_language(C)

# test matrix
#
# A :=
#                        | no cmake syntax | cmake syntax |
# -----------------------+-----------------+--------------+
# absolute install RPATH |       exe1      |     exe4     |
# relative install RPATH |       exe2      |     exe3     |
#
# all := A * CMP005_OLD + A * CMP0095_WARN + A * CMP0095_NEW

add_library(utils SHARED obj1.c)
set(targets utils)

set(exe1_install_rpath "/foo/bar")
set(exe2_install_rpath "\$ORIGIN/../lib")
set(exe3_install_rpath "\${ORIGIN}/../lib")
set(exe4_install_rpath "/foo/bar/\${PLATFORM}")

macro(A_CMP0095 policy_value)
  cmake_policy(PUSH)
  if(NOT "x${policy_value}x" STREQUAL "xWARNx")
    cmake_policy(SET CMP0095 ${policy_value})
  endif()
  string(TOLOWER "${policy_value}" p)

  # exe1: absolute install RPATH, no cmake syntax
  set(case "exe1")
  set(target "${case}_cmp0095_${p}")
  list(APPEND targets ${target})
  add_executable(${target} main.c)
  target_link_libraries(${target} PRIVATE utils)
  set_target_properties(${target} PROPERTIES
    INSTALL_RPATH "${${case}_install_rpath}")

  # exe2: relative install RPATH, no cmake syntax
  set(case "exe2")
  set(target "${case}_cmp0095_${p}")
  list(APPEND targets ${target})
  add_executable(${target} main.c)
  target_link_libraries(${target} PRIVATE utils)
  set_target_properties(${target} PROPERTIES
    INSTALL_RPATH "${${case}_install_rpath}")

  # exe3: relative install RPATH, cmake syntax
  set(case "exe3")
  set(target "${case}_cmp0095_${p}")
  list(APPEND targets ${target})
  add_executable(${target} main.c)
  target_link_libraries(${target} PRIVATE utils)
  set_target_properties(${target} PROPERTIES
    INSTALL_RPATH "${${case}_install_rpath}")

  # exe4: absolute install RPATH, cmake syntax
  set(case "exe4")
  set(target "${case}_cmp0095_${p}")
  list(APPEND targets ${target})
  add_executable(${target} main.c)
  target_link_libraries(${target} PRIVATE utils)
  set_target_properties(${target} PROPERTIES
    INSTALL_RPATH "${${case}_install_rpath}")

  cmake_policy(POP)
endmacro()

A_CMP0095("OLD")
A_CMP0095("WARN") # exe3 and exe4 are expected to issue an author warning
A_CMP0095("NEW")

install(TARGETS ${targets})
