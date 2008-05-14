cmake_minimum_required(VERSION 2.6)
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

#-----------------------------------------------------------------------------
IF("${CMAKE_GENERATOR}" MATCHES "Make")
  # Test the IMPLICIT_DEPENDS feature.
  SET(ZOT_DEPENDS IMPLICIT_DEPENDS CXX ${CMAKE_CURRENT_SOURCE_DIR}/dep.cxx)
  SET(ZOT_CUSTOM_DEP
    IMPLICIT_DEPENDS CXX ${CMAKE_CURRENT_SOURCE_DIR}/dep_custom.cxx)
ELSE("${CMAKE_GENERATOR}" MATCHES "Make")
  # No IMPLICIT_DEPENDS...just depend directly.
  SET(ZOT_DEPENDS DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/zot.hxx.in)
  SET(ZOT_CUSTOM_DEP DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/zot_custom.hxx.in)
ENDIF("${CMAKE_GENERATOR}" MATCHES "Make")
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/zot.hxx
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_BINARY_DIR}/zot.hxx.in
  ${CMAKE_CURRENT_BINARY_DIR}/zot.hxx
  ${ZOT_DEPENDS}
  )

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/zot_custom.hxx
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_BINARY_DIR}/zot_custom.hxx.in
  ${CMAKE_CURRENT_BINARY_DIR}/zot_custom.hxx
  ${ZOT_CUSTOM_DEP}
  )
add_custom_target(zot_custom ALL DEPENDS
  ${CMAKE_CURRENT_BINARY_DIR}/zot_custom.hxx)

add_executable(zot zot.cxx ${CMAKE_CURRENT_BINARY_DIR}/zot.hxx
  zot_macro_dir.cxx zot_macro_tgt.cxx)
add_dependencies(zot zot_custom)

# Test the #include line macro transformation rule support.
set_property(
  TARGET zot
  PROPERTY IMPLICIT_DEPENDS_INCLUDE_TRANSFORM "ZOT_TGT(%)=<zot_%_tgt.hxx>"
  )

set_property(
  DIRECTORY
  PROPERTY IMPLICIT_DEPENDS_INCLUDE_TRANSFORM "ZOT_DIR(%)=<zot_%_dir.hxx>"
  )
