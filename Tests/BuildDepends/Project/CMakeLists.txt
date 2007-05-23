project(testRebuild)
add_library(foo STATIC ${testRebuild_BINARY_DIR}/foo.cxx)

# Add a generated header that regenerates when the generator is
# rebuilt.
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/regen.h
  COMMAND generator ${CMAKE_CURRENT_BINARY_DIR}/regen.h regen
  DEPENDS generator # adds file-level dependency to re-run rule
  )

# Add a generated header that does NOT regenerate when the generator
# is rebuilt.
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/noregen.h
  COMMAND generator ${CMAKE_CURRENT_BINARY_DIR}/noregen.h noregen
  )

# Test that the generator rebuilds when the static library source file
# changes.  This should cause regen.h to be recreated also.
add_executable(generator generator.cxx)
target_link_libraries(generator foo)

# Build an executable to drive the build and rebuild.
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(bar bar.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/regen.h
  ${CMAKE_CURRENT_BINARY_DIR}/noregen.h
  )
target_link_libraries(bar foo)
