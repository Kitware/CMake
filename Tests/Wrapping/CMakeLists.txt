#
# Wrapping
#
PROJECT (Wrapping)

#
# Lib and exe path
#
SET (LIBRARY_OUTPUT_PATH 
     ${Wrapping_BINARY_DIR}/bin/ CACHE PATH 
     "Single output directory for building all libraries.")

SET (EXECUTABLE_OUTPUT_PATH 
     ${Wrapping_BINARY_DIR}/bin/ CACHE PATH 
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

#
# Test VTK wrappers
#
SOURCE_FILES (SRCS
  vtkExcluded
  vtkIncluded
)

WRAP_EXCLUDE_FILES (
  vtkExcluded
)

#
# Specify (empty) VTK hint files
#
FIND_FILE(VTK_WRAP_HINTS hints ${Wrapping_SOURCE_DIR})

#
# Wrap VTK -> Tcl
#
SET (VTK_WRAP_TCL "On")
SET (VTK_WRAP_TCL_EXE "echo")
VTK_WRAP_TCL (wraplibTCL TCL_SRCS SRCS)

#
# Wrap VTK -> Python
#
SET (VTK_WRAP_PYTHON "On")
SET (VTK_WRAP_PYTHON_EXE "echo")
VTK_WRAP_PYTHON (wraplibPython Python_SRCS SRCS)

#
# Wrap VTK -> Java
#
# VTK_WRAP_JAVA puts dependencie between wraplibJavaJavaClasses (in ALL)
# and each Java wrapped class (here, vtkIncluded, so let's create a dummy one
# so that the build succeeds.
#
SET (VTK_WRAP_JAVA "On")
SET (VTK_WRAP_JAVA_EXE "echo")
SET (VTK_PARSE_JAVA_EXE "echo")
SET (VTK_JAVA_HOME "${Wrapping_BINARY_DIR}/java")
VTK_WRAP_JAVA (wraplibJava Java_SRCS SRCS)
CONFIGURE_FILE(
  ${Wrapping_SOURCE_DIR}/dummy
  ${VTK_JAVA_HOME}/vtkIncluded.java
  COPYONLY IMMEDIATE)

#
# QT Wrappers
#

SET (QT_WRAP_CPP "On")
SET (QT_MOC_EXE "echo")

INCLUDE( ${CMAKE_ROOT}/Modules/FindQt.cmake )

IF (QT_FOUND)

ADD_EXECUTABLE (qtwrapping qtwrappingmain.cxx QTUI_S_SRCS)

INCLUDE_DIRECTORIES( ${QT_INCLUDE_DIR} )
INCLUDE_DIRECTORIES( ./ )

LINK_LIBRARIES( ${QT_LIBRARIES} )

QT_WRAP_CPP (wraplibQT QT_SRCS SRCS)

SET (QT_WRAP_UI "On")
SET (QT_UIC_EXE "echo")
SET (QTUI_SRCS
  qtwrapping.ui
)
QT_WRAP_UI (qtwrapping QTUI_H_SRCS QTUI_S_SRCS QTUI_SRCS)

ENDIF (QT_FOUND)

#
# FLTK Wrappers
#
# Since FLTK_FLUID_EXE is supposed to create a .cxx/.h from a .fl/.fld,
# create an empty one so that the dependencies can be met.
#
SOURCE_FILES (FLTK_SRCS
  fltk1.fl
)
SET (FLTK_WRAP_UI "On")
SET (FLTK_FLUID_EXECUTABLE "echo")
FLTK_WRAP_UI (wraplibFLTK FLTK_SRCS)
ADD_LIBRARY (wraplibFLTK)
CONFIGURE_FILE(
  ${Wrapping_SOURCE_DIR}/fltk1.fl
  ${Wrapping_BINARY_DIR}/fltk1.cxx
  COPYONLY)

#
# Mangled Mesa
#
CONFIGURE_FILE(
  ${Wrapping_SOURCE_DIR}/dummy
  ${Wrapping_BINARY_DIR}/gl.h
  COPYONLY IMMEDIATE)
USE_MANGLED_MESA (${Wrapping_BINARY_DIR} ${Wrapping_BINARY_DIR}/mangled_mesa)

#
# Instantiator
#
VTK_MAKE_INSTANTIATOR(vtkWraplibInstantiator 
                      Instantiator_SRCS
                      SRCS
                      EXPORT_MACRO VTK_WRAPLIB_EXPORT
                      HEADER_LOCATION ${Wrapping_BINARY_DIR}
                      GROUP_SIZE 5)

#
# ITK Tcl Wrapping
#
SET(CABLE "echo")
SET(CABLE_INCLUDE_DIR ${Wrapping_SOURCE_DIR})
ADD_LIBRARY(wrapITK)
ITK_WRAP_TCL(wrapITK itkWrapperConfig.cxx)
CONFIGURE_FILE(${Wrapping_SOURCE_DIR}/itkWrapperConfig.cxx
	       ${Wrapping_BINARY_DIR}/itkWrapperConfig_tcl.cxx
               COPYONLY IMMEDIATE)
