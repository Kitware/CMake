cmake_policy(SET CMP0229 NEW)
enable_language(C)

add_library(A STATIC A.c)

set(CMAKE_SKIP_INSTALL_RPATH ON)
add_executable(skipped main.c)
target_link_libraries(skipped A)
set_target_properties(skipped PROPERTIES
  INSTALL_RPATH /opt/foo/lib
  BUILD_WITH_INSTALL_RPATH 1
  )

set(CMAKE_SKIP_INSTALL_RPATH OFF)
add_executable(kept main.c)
target_link_libraries(kept A)
set_target_properties(kept PROPERTIES
  INSTALL_RPATH /opt/foo/lib
  BUILD_WITH_INSTALL_RPATH 1
  )
