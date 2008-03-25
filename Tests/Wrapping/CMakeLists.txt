#
# Wrapping
#
cmake_minimum_required (VERSION 2.6)
PROJECT (Wrapping)

# Disable cleaning of custom command outputs to preserve the hacks
# used to generate the files using CONFIGURE_FILE.
SET_DIRECTORY_PROPERTIES(PROPERTIES CLEAN_NO_CUSTOM 1)

#
# Lib and exe path
#
SET (LIBRARY_OUTPUT_PATH 
  ${Wrapping_BINARY_DIR}/bin/ CACHE INTERNAL 
  "Single output directory for building all libraries.")

SET (EXECUTABLE_OUTPUT_PATH 
  ${Wrapping_BINARY_DIR}/bin/ CACHE INTERNAL 
  "Single output directory for building all executables.")

#
# Where will executable tests be written ?
#
IF (EXECUTABLE_OUTPUT_PATH)
  SET (CXX_TEST_PATH ${EXECUTABLE_OUTPUT_PATH})
ELSE (EXECUTABLE_OUTPUT_PATH)
  SET (CXX_TEST_PATH .)
ENDIF (EXECUTABLE_OUTPUT_PATH)

#
# Add exe
#
ADD_EXECUTABLE (wrapping wrapping.cxx)

ADD_EXECUTABLE (Wrap Wrap.c)
IF(WIN32)
  SET(EXE_EXT ".exe")
ENDIF(WIN32)
SET(WRAP ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/Wrap${EXE_EXT})

#
# QT Wrappers
#

SET (QT_WRAP_CPP "On")
SET (QT_MOC_EXE "echo")
INCLUDE( FindQt3 )

IF (QT_FOUND AND QT_WRAP_UI)
  message("found qt 3 test it...")
  INCLUDE_DIRECTORIES( ${QT_INCLUDE_DIR} )
  INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )


  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/foo.ui.in
    ${CMAKE_CURRENT_BINARY_DIR}/foo.ui IMMEDIATE)

  SET (QT_WRAP_UI "On")
  SET (QT_UIC_EXE "${QT_UIC_EXECUTABLE}")


  SET (QTUI_SRCS
    qtwrapping.ui
    ${CMAKE_CURRENT_BINARY_DIR}/foo.ui
    )
  QT_WRAP_UI (myqtlib QTUI_H_SRCS QTUI_S_SRCS ${QTUI_SRCS})
  QT_WRAP_CPP (myqtlib QT_MOC_SRCS ${SRCS} vtkTestMoc.h)

  MESSAGE("QT files are ${QTUI_S_SRCS}")
  MESSAGE("QT other files are ${QTUI_H_SRCS}")
  ADD_DEFINITIONS(${QT_DEFINITIONS})
  ADD_LIBRARY(myqtlib ${QTUI_S_SRCS} ${QT_MOC_SRCS})
  ADD_EXECUTABLE (qtwrapping qtwrappingmain.cxx)
  TARGET_LINK_LIBRARIES(qtwrapping myqtlib)

  TARGET_LINK_LIBRARIES( qtwrapping ${QT_LIBRARIES} )
ELSE (QT_FOUND AND QT_WRAP_UI)
  ADD_EXECUTABLE (qtwrapping qtnoqtmain.cxx)
ENDIF (QT_FOUND AND QT_WRAP_UI)

#
# FLTK Wrappers
#
# Since FLTK_FLUID_EXE is supposed to create a .cxx/.h from a .fl/.fld,
# create an empty one so that the dependencies can be met.
#
SET (FLTK_SRCS
  fltk1.fl
  )
ADD_EXECUTABLE(fakefluid fakefluid.cxx)
GET_TARGET_PROPERTY(FLUID_LOC fakefluid LOCATION)
SET (FLTK_WRAP_UI "On")
SET (FLTK_FLUID_EXECUTABLE "${FLUID_LOC}")
FLTK_WRAP_UI (wraplibFLTK ${FLTK_SRCS})
ADD_LIBRARY(wraplibFLTK ${wraplibFLTK_FLTK_UI_SRCS})
ADD_DEPENDENCIES(wraplibFLTK fakefluid)
ADD_DEPENDENCIES(fakefluid Wrap)
#
# Mangled Mesa
#
CONFIGURE_FILE(
  ${Wrapping_SOURCE_DIR}/dummy
  ${Wrapping_BINARY_DIR}/gl.h
  COPYONLY IMMEDIATE)
USE_MANGLED_MESA (${Wrapping_BINARY_DIR} ${Wrapping_BINARY_DIR}/mangled_mesa)

