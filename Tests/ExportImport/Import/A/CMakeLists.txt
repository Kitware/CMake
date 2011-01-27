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

#-----------------------------------------------------------------------------
# Create a custom target to generate a header for the libraries below.
# Drive the header generation through an indirect chain of imported
# target dependencies.

# testLib2tmp1.h
add_custom_command(
  OUTPUT testLib2tmp1.h
  VERBATIM COMMAND
    ${CMAKE_COMMAND} -E echo "extern int testLib2(void);" > testLib2tmp1.h
  )

# hdr_testLib2tmp1 needs testLib2tmp1.h
add_custom_target(hdr_testLib2tmp1 DEPENDS testLib2tmp1.h)

# exp_testExe2 needs hdr_testLib2tmp1
add_dependencies(exp_testExe2 hdr_testLib2tmp1)

# testLib2tmp.h needs exp_testExe2
add_custom_command(
  OUTPUT testLib2tmp.h
  VERBATIM COMMAND exp_testExe2
  COMMAND ${CMAKE_COMMAND} -E copy testLib2tmp1.h testLib2tmp.h
  )

# hdr_testLib2tmp needs testLib2tmp.h
add_custom_target(hdr_testLib2tmp DEPENDS testLib2tmp.h)

add_library(dep_testLib2tmp UNKNOWN IMPORTED)
set_property(TARGET dep_testLib2tmp PROPERTY
  IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/testLib2tmp.h)

# dep_testLib2tmp needs hdr_testLib2tmp
add_dependencies(dep_testLib2tmp hdr_testLib2tmp)

# testLib2.h needs dep_testLib2tmp
add_custom_command(
  OUTPUT testLib2.h
  VERBATIM COMMAND ${CMAKE_COMMAND} -E copy testLib2tmp.h testLib2.h
  DEPENDS dep_testLib2tmp
  )

# hdr_testLib2 needs testLib2.h
add_custom_target(hdr_testLib2 DEPENDS testLib2.h)

add_library(dep_testLib2 UNKNOWN IMPORTED)

# dep_testLib2 needs hdr_testLib2
add_dependencies(dep_testLib2 hdr_testLib2)

# exp_testLib2 and bld_testLib2 both need dep_testLib2
add_dependencies(bld_testLib2 dep_testLib2)
add_dependencies(exp_testLib2 dep_testLib2)

#-----------------------------------------------------------------------------
# Create a library to be linked by another directory in this project
# to test transitive linking to otherwise invisible imported targets.
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(imp_lib1 STATIC imp_lib1.c)
target_link_libraries(imp_lib1 exp_testLib2)
add_library(imp_lib1b STATIC imp_lib1.c)
target_link_libraries(imp_lib1b bld_testLib2)
