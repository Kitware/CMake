cmake_minimum_required (VERSION 2.6)
PROJECT(PrecompiledHeader C)

# Make sure the proper compiler is in use.
IF(NOT MSVC AND NOT "${CMAKE_C_COMPILER_ID}" MATCHES "^(Intel)$")
  MESSAGE(FATAL_ERROR "The PrecompiledHeader test works only with MSVC or Intel")
ENDIF()

# Compute a custom name for the precompiled header.
IF(CMAKE_CONFIGURATION_TYPES)
  SET(PCH_DIR "${CMAKE_CURRENT_BINARY_DIR}/PCH/${CMAKE_CFG_INTDIR}")
  FOREACH(cfg ${CMAKE_CONFIGURATION_TYPES})
    FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/PCH/${cfg})
  ENDFOREACH()
ELSE(CMAKE_CONFIGURATION_TYPES)
  SET(PCH_DIR "${CMAKE_CURRENT_BINARY_DIR}/PCH")
  FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/PCH)
ENDIF(CMAKE_CONFIGURATION_TYPES)

# The VS6 IDE does not support renaming .pch files with /Fp.
IF("${CMAKE_GENERATOR}" MATCHES "Visual Studio 6")
  SET(PCH_USE_INCLUDE_DIR 1)
  SET(PCH_FILE)
ELSE("${CMAKE_GENERATOR}" MATCHES "Visual Studio 6")
  SET(PCH_USE_INCLUDE_DIR 0)
  SET(PCH_FILE "\"/Fp${PCH_DIR}/foo_precompiled.pch\"")
ENDIF("${CMAKE_GENERATOR}" MATCHES "Visual Studio 6")

# Choose between an explicit include path and using /I during
# precompilation.  The /I form is used to test that the PCH is
# actually used.  In practice the include path form would be used.
IF(PCH_USE_INCLUDE_DIR)
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)
ELSE(PCH_USE_INCLUDE_DIR)
  SET(PCH_INCLUDE_DIR "\"/I${CMAKE_CURRENT_SOURCE_DIR}/include\"")
ENDIF(PCH_USE_INCLUDE_DIR)

# Create a target that will use a precompiled header.
SET(foo_SRCS foo1.c foo2.c)
ADD_EXECUTABLE(foo foo_precompile.c ${foo_SRCS})

# Setup flags on the target to create and use the precompiled header.
SET_TARGET_PROPERTIES(foo PROPERTIES
  COMPILE_FLAGS "/Yufoo_precompiled.h /FIfoo_precompiled.h ${PCH_FILE}")
SET_SOURCE_FILES_PROPERTIES(foo_precompile.c PROPERTIES
  COMPILE_FLAGS "/Ycfoo_precompiled.h ${PCH_INCLUDE_DIR}")

# Setup dependencies for precompiled header creation and use.  The VS
# IDE takes care of this automatically.
IF("${CMAKE_GENERATOR}" MATCHES "Makefile" OR
   "${CMAKE_GENERATOR}" MATCHES "Ninja")
  # This source file creates the precompiled header as a side-effect.
  SET_SOURCE_FILES_PROPERTIES(foo_precompile.c PROPERTIES
    OBJECT_OUTPUTS "${PCH_DIR}/foo_precompiled.pch")

  # These source files use the precompiled header.
  SET_SOURCE_FILES_PROPERTIES(${foo_SRCS} PROPERTIES
    OBJECT_DEPENDS "${PCH_DIR}/foo_precompiled.pch")
ENDIF("${CMAKE_GENERATOR}" MATCHES "Makefile")
