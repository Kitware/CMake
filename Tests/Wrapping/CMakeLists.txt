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
# Link to CMake lib
#
FIND_LIBRARY (CMAKE_LIB
              CMakeLib 
              ${Wrapping_BINARY_DIR}/../../Source
              ${Wrapping_BINARY_DIR}/../../Source/Debug
              ${Wrapping_BINARY_DIR}/../../Source/Release
              ${Wrapping_BINARY_DIR}/../../Source/MinSizeRel
              ${Wrapping_BINARY_DIR}/../../Source/RelWithDebInfo)

LINK_LIBRARIES (${CMAKE_LIB})

#
# Add exe
#
ADD_EXECUTABLE (wrapping wrapping.cxx)

#
# Test Cable
#
CABLE_CLASS_SET (Float float)
CABLE_CLASS_SET (Mesh "itk::Mesh<$Float>")

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
# and each Java wrapped class (here, vtkIncluded, although it is not created). 
# A custom target is created to trick 'make' so that the build succeeds.
#
SET (VTK_WRAP_JAVA "On")
SET (VTK_WRAP_JAVA_EXE "echo")
SET (VTK_PARSE_JAVA_EXE "echo")
SET (VTK_JAVA_HOME ".")
VTK_WRAP_JAVA (wraplibJava Java_SRCS SRCS)
ADD_CUSTOM_TARGET(${VTK_JAVA_HOME}/vtkIncluded.java
                  echo "Triggering custom target vtkIncluded.java")

