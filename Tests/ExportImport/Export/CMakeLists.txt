project(Export C)

# We need ansi C support.
if(CMAKE_ANSI_CFLAGS)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_ANSI_CFLAGS}")
endif(CMAKE_ANSI_CFLAGS)

add_library(testExe1lib STATIC testExe1lib.c) # not exported
add_executable(testExe1 testExe1.c)
target_link_libraries(testExe1 testExe1lib)

add_executable(testExe2 testExe2.c)
set_property(TARGET testExe2 PROPERTY ENABLE_EXPORTS 1)

add_library(testLib1 STATIC testLib1.c)
add_library(testLib2 STATIC testLib2.c)
target_link_libraries(testLib2 testLib1)

add_library(testLib3 SHARED testLib3.c)

add_library(testLib4 SHARED testLib4.c)
set_property(TARGET testLib4 PROPERTY FRAMEWORK 1)

# Install and export from install tree.
install(
  TARGETS testExe1 testLib1 testLib2 testExe2 testLib3 testLib4
  EXPORT exp
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  FRAMEWORK DESTINATION Frameworks
  )
install(EXPORT exp NAMESPACE exp_ DESTINATION lib/exp)

# Export from build tree.
export(TARGETS testExe1 testLib1 testLib2
  NAMESPACE bld_
  FILE ExportBuildTree.cmake
  )
export(TARGETS testExe2 testLib3 testLib4
  NAMESPACE bld_
  APPEND FILE ExportBuildTree.cmake
  )
