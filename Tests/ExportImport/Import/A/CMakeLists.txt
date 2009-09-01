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
target_link_libraries(imp_testExe1
  exp_testLib2
  exp_testLib3
  exp_testLib4
  exp_testLib5
  exp_testLib6
  exp_testLibCycleA
  )

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
target_link_libraries(imp_testExe1b
  bld_testLib2
  bld_testLib3
  bld_testLib4
  bld_testLib5
  bld_testLib6
  bld_testLibCycleA
  )

# Try building a plugin to an executable imported from the build tree.
add_library(imp_mod1b MODULE imp_mod1.c)
target_link_libraries(imp_mod1b bld_testExe2)

# Export/CMakeLists.txt pretends the RelWithDebInfo (as well as Debug)
# configuration should link to debug libs.
foreach(c DEBUG RELWITHDEBINFO)
  set_property(TARGET imp_testExe1 PROPERTY COMPILE_DEFINITIONS_${c} EXE_DBG)
  set_property(TARGET imp_testExe1b PROPERTY COMPILE_DEFINITIONS_${c} EXE_DBG)
endforeach(c)

# Create a library to be linked by another directory in this project
# to test transitive linking to otherwise invisible imported targets.
add_library(imp_lib1 STATIC imp_lib1.c)
target_link_libraries(imp_lib1 exp_testLib2)
add_library(imp_lib1b STATIC imp_lib1.c)
target_link_libraries(imp_lib1b bld_testLib2)
