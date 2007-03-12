PROJECT(PrecompiledHeader C)

# Make sure the proper compiler is in use.
IF(NOT MSVC)
  MESSAGE(FATAL_ERROR "The PrecompiledHeader test works only with MSVC")
ENDIF(NOT MSVC)

# Compute a custom name for the precompiled header.
IF(CMAKE_CONFIGURATION_TYPES)
  SET(PCH_DIR "${CMAKE_CURRENT_BINARY_DIR}/PCH/${CMAKE_CFG_INTDIR}")
ELSE(CMAKE_CONFIGURATION_TYPES)
  SET(PCH_DIR "${CMAKE_CURRENT_BINARY_DIR}/PCH")
ENDIF(CMAKE_CONFIGURATION_TYPES)
FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/PCH)

# The VS6 IDE does not support renaming .pch files so we cannot use a
# separate target.
IF("${CMAKE_GENERATOR}" MATCHES "Visual Studio 6")
  SET(PCH_USE_TARGET 0)
  SET(PCH_USE_INCLUDE_DIR 1)
ELSE("${CMAKE_GENERATOR}" MATCHES "Visual Studio 6")
  SET(PCH_USE_TARGET 1)
  SET(PCH_USE_INCLUDE_DIR 0)
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
IF(PCH_USE_TARGET)
  ADD_EXECUTABLE(foo ${foo_SRCS})

  # Create a target to precompile the header for the executable.
  ADD_LIBRARY(foo_precompile foo_precompile.c include/foo_precompiled.h)
  SET_TARGET_PROPERTIES(foo_precompile PROPERTIES OUTPUT_NAME foo)
  ADD_DEPENDENCIES(foo foo_precompile)

  SET(PCH_TARGETS foo foo_precompile)
  SET(PCH_FILE "\"/Fp${PCH_DIR}/foo_precompiled.pch\"")
ELSE(PCH_USE_TARGET)
  # Put the precompiled header source directly in the target.
  ADD_EXECUTABLE(foo foo_precompile.c ${foo_SRCS})
  SET(PCH_TARGETS foo)
ENDIF(PCH_USE_TARGET)

# Add the PCH to the list of files to clean.  It is created as a
# side-effect so CMake does not know about it.
SET_DIRECTORY_PROPERTIES(PROPERTIES
  ADDITIONAL_MAKE_CLEAN_FILES ${PCH_DIR}/foo_precompiled.pch
  )

# Setup flags on the two targets to create and use the precompiled header.
SET_TARGET_PROPERTIES(${PCH_TARGETS} PROPERTIES COMPILE_FLAGS
  "/Yufoo_precompiled.h /FIfoo_precompiled.h ${PCH_FILE}")
SET_SOURCE_FILES_PROPERTIES(foo_precompile.c PROPERTIES COMPILE_FLAGS
  "/Ycfoo_precompiled.h ${PCH_INCLUDE_DIR}")

# Make sure the object files rebuild when their precompiled header has changed.
# The VS IDE takes care of this automatically.
IF("${CMAKE_GENERATOR}" MATCHES "Makefile")
  SET_SOURCE_FILES_PROPERTIES(${foo_SRCS} PROPERTIES
    OBJECT_DEPENDS "${PCH_DIR}/foo_precompiled.pch")
ENDIF("${CMAKE_GENERATOR}" MATCHES "Makefile")
