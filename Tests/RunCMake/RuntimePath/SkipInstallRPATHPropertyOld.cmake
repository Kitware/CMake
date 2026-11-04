cmake_policy(SET CMP0229 OLD)
enable_language(C)

add_library(A STATIC A.c)

add_executable(skipped main.c)
target_link_libraries(skipped A)
set_target_properties(skipped PROPERTIES
  INSTALL_RPATH /opt/foo/lib
  BUILD_WITH_INSTALL_RPATH 1
  )

# Control target created with the variable unset.
add_executable(kept main.c)
target_link_libraries(kept A)
set_target_properties(kept PROPERTIES
  INSTALL_RPATH /opt/foo/lib
  BUILD_WITH_INSTALL_RPATH 1
  SKIP_INSTALL_RPATH OFF
  )

set(CMAKE_SKIP_INSTALL_RPATH ON)
