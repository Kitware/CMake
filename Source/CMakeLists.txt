INCLUDE (${CMAKE_ROOT}/Modules/FindFLTK.cmake)


IF(FLTK_LIBRARY)
  IF(FLTK_INCLUDE_PATH)
    SUBDIRS(FLTKDialog)
  ENDIF(FLTK_INCLUDE_PATH)
ENDIF(FLTK_LIBRARY)

SOURCE_FILES(SRCS
cmake  
cmMakeDepend
cmMakefile
cmMakefileGenerator
cmRegularExpression
cmSourceFile
cmSystemTools
cmDirectory
cmCommands
cmTarget
cmCustomCommand
cmCacheManager
cmCableClassSet
cmSourceGroup
)

IF (WIN32)
  SOURCE_FILES(SRCS cmDSWWriter cmDSPWriter cmMSProjectGenerator)
ELSE (WIN32)
  SOURCE_FILES(SRCS cmUnixMakefileGenerator)
ENDIF (WIN32)

# create a library used by the command line and the GUI
ADD_LIBRARY(CMakeLib SRCS)
# always link in the library
LINK_LIBRARIES(CMakeLib)
# the library is found here
LINK_DIRECTORIES(${CMAKE}/Sources)

ADD_EXECUTABLE(cmake cmakemain)

ADD_TEST(burn cmake)

INSTALL_TARGETS(/bin cmake)

