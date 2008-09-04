cmake_minimum_required (VERSION 2.6)
project(Import C)

# We need ansi C support.
if(CMAKE_ANSI_CFLAGS)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_ANSI_CFLAGS}")
endif(CMAKE_ANSI_CFLAGS)

# Import targets from the exported build tree.
include(${Import_BINARY_DIR}/../Export/ExportBuildTree.cmake)

# Import targets from the exported install tree.
include(${CMAKE_INSTALL_PREFIX}/lib/exp/exp.cmake)

# Try referencing an executable imported from the install tree.
add_custom_command(
  OUTPUT ${Import_BINARY_DIR}/exp_generated.c
  COMMAND exp_testExe1 ${Import_BINARY_DIR}/exp_generated.c
  DEPENDS exp_testExe1
  )
add_custom_command(
  OUTPUT ${Import_BINARY_DIR}/exp_generated3.c
  COMMAND exp_testExe3 ${Import_BINARY_DIR}/exp_generated3.c
  DEPENDS exp_testExe3
  )

add_executable(imp_testExe1
  imp_testExe1.c
  ${Import_BINARY_DIR}/exp_generated.c
  ${Import_BINARY_DIR}/exp_generated3.c
  )

# Try linking to a library imported from the install tree.
target_link_libraries(imp_testExe1 exp_testLib2 exp_testLib3 exp_testLib4)

# Try building a plugin to an executable imported from the install tree.
add_library(imp_mod1 MODULE imp_mod1.c)
target_link_libraries(imp_mod1 exp_testExe2)

# Try referencing an executable imported from the build tree.
add_custom_command(
  OUTPUT ${Import_BINARY_DIR}/bld_generated.c
  COMMAND bld_testExe1 ${Import_BINARY_DIR}/bld_generated.c
  DEPENDS bld_testExe1
  )
add_custom_command(
  OUTPUT ${Import_BINARY_DIR}/bld_generated3.c
  COMMAND bld_testExe3 ${Import_BINARY_DIR}/bld_generated3.c
  DEPENDS bld_testExe3
  )

add_executable(imp_testExe1b
  imp_testExe1.c
  ${Import_BINARY_DIR}/bld_generated.c
  ${Import_BINARY_DIR}/bld_generated3.c
  )

# Try linking to a library imported from the build tree.
target_link_libraries(imp_testExe1b bld_testLib2 bld_testLib3 bld_testLib4)

# Try building a plugin to an executable imported from the build tree.
add_library(imp_mod1b MODULE imp_mod1.c)
target_link_libraries(imp_mod1b bld_testExe2)

# Export/CMakeLists.txt pretends the RelWithDebInfo (as well as Debug)
# configuration should link to debug libs.
foreach(c DEBUG RELWITHDEBINFO)
  set_property(TARGET imp_testExe1 PROPERTY COMPILE_DEFINITIONS_${c} EXE_DBG)
  set_property(TARGET imp_testExe1b PROPERTY COMPILE_DEFINITIONS_${c} EXE_DBG)
endforeach(c)
