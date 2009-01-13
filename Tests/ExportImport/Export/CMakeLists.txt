cmake_minimum_required (VERSION 2.6)
project(Export C)

# We need ansi C support.
if(CMAKE_ANSI_CFLAGS)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_ANSI_CFLAGS}")
endif(CMAKE_ANSI_CFLAGS)

# Pretend that RelWithDebInfo should link to debug libraries to test
# the DEBUG_CONFIGURATIONS property.
set_property(GLOBAL PROPERTY DEBUG_CONFIGURATIONS Debug RelWithDebInfo)

add_library(testExe1lib STATIC testExe1lib.c) # not exported
add_executable(testExe1 testExe1.c)
target_link_libraries(testExe1 testExe1lib)
set_property(TARGET testExe1 PROPERTY VERSION 4)

add_library(testExe2libImp SHARED testExe2libImp.c)
set_property(TARGET testExe2libImp PROPERTY LIBRARY_OUTPUT_DIRECTORY impl)
add_library(testExe2lib SHARED testExe2lib.c)
target_link_libraries(testExe2lib testExe2libImp)
set_property(TARGET testExe2lib PROPERTY LINK_INTERFACE_LIBRARIES "")
add_executable(testExe2 testExe2.c)
set_property(TARGET testExe2 PROPERTY ENABLE_EXPORTS 1)
set_property(TARGET testExe2 PROPERTY LINK_INTERFACE_LIBRARIES testExe2lib)

add_library(testLib1 STATIC testLib1.c)
add_library(testLib2 STATIC testLib2.c)
target_link_libraries(testLib2 testLib1)

add_library(testLib3Imp SHARED testLib3Imp.c)
set_property(TARGET testLib3Imp PROPERTY LIBRARY_OUTPUT_DIRECTORY impl)
add_library(testLib3 SHARED testLib3.c)
target_link_libraries(testLib3 testLib3Imp)
set_property(TARGET testLib3 PROPERTY LINK_INTERFACE_LIBRARIES "")
set_property(TARGET testLib3 PROPERTY VERSION 1.2)
set_property(TARGET testLib3 PROPERTY SOVERSION 3)

add_library(testLib4 SHARED testLib4.c)
set_property(TARGET testLib4 PROPERTY FRAMEWORK 1)

# Work-around: Visual Studio 6 does not support per-target object files.
set(VS6)
if("${CMAKE_GENERATOR}" MATCHES "Visual Studio 6")
  set(VS6 1)
endif("${CMAKE_GENERATOR}" MATCHES "Visual Studio 6")

# Test using the target_link_libraries command to set the
# LINK_INTERFACE_LIBRARIES* properties.  We construct two libraries
# providing the same two symbols.  In each library one of the symbols
# will work and the other one will fail to link.  The import part of
# this test will try to use the symbol corresponding to the
# configuration in which it is built.  If the proper library is not
# used via the link interface the import test will fail to link.
add_library(testLib4lib STATIC testLib4lib.c)
add_library(testLib4libdbg STATIC testLib4libopt.c testLib4libdbg${VS6}.c)
add_library(testLib4libopt STATIC testLib4libdbg.c testLib4libopt${VS6}.c)
set_property(TARGET testLib4libdbg PROPERTY COMPILE_DEFINITIONS LIB_DBG)
set_property(TARGET testLib4libopt PROPERTY COMPILE_DEFINITIONS LIB_OPT)
target_link_libraries(testLib4
  LINK_INTERFACE_LIBRARIES
  testLib4lib debug testLib4libdbg optimized testLib4libopt
  )

add_executable(testExe3 testExe3.c)
set_property(TARGET testExe3 PROPERTY MACOSX_BUNDLE 1)

# Install and export from install tree.
install(
  TARGETS
  testExe1 testLib1 testLib2 testExe2 testLib3 testLib4 testExe3
  testExe2lib testLib4lib testLib4libdbg testLib4libopt
  EXPORT exp
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib NAMELINK_SKIP
  ARCHIVE DESTINATION lib
  FRAMEWORK DESTINATION Frameworks
  BUNDLE DESTINATION Applications
  )
install(
  TARGETS
  testExe2libImp testLib3Imp
  EXPORT exp
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib/impl
  ARCHIVE DESTINATION lib/impl
  )
install(EXPORT exp NAMESPACE exp_ DESTINATION lib/exp)

# Export from build tree.
export(TARGETS testExe1 testLib1 testLib2 testLib3
  testExe2libImp testLib3Imp
  NAMESPACE bld_
  FILE ExportBuildTree.cmake
  )
export(TARGETS testExe2 testLib4 testExe3 testExe2lib
  testLib4lib testLib4libdbg testLib4libopt
  NAMESPACE bld_
  APPEND FILE ExportBuildTree.cmake
  )
