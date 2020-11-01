enable_language(C)

add_library(A STATIC A.c)

add_executable(buildge main.c)
target_link_libraries(buildge A)
set_target_properties(buildge PROPERTIES
  BUILD_RPATH $<1:/opt/foo/lib>
  )

add_executable(buildnoge main.c)
target_link_libraries(buildnoge A)
set_target_properties(buildnoge PROPERTIES
  BUILD_RPATH /opt/foo/lib
  )

add_executable(installge main.c)
target_link_libraries(installge A)
set_target_properties(installge PROPERTIES
  INSTALL_RPATH $<1:/opt/foo/lib>
  BUILD_WITH_INSTALL_RPATH 1
  )

add_executable(installnoge main.c)
target_link_libraries(installnoge A)
set_target_properties(installnoge PROPERTIES
  INSTALL_RPATH /opt/foo/lib
  BUILD_WITH_INSTALL_RPATH 1
  )
