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
cmakemain
)

IF (WIN32)
  SOURCE_FILES(SRCS cmDSWWriter cmDSPWriter cmMSProjectGenerator)
ELSE (WIN32)
  SOURCE_FILES(SRCS cmUnixMakefileGenerator)
ENDIF (WIN32)

ADD_EXECUTABLE(cmake SRCS)

ADD_TEST(burn cmake)

INSTALL_TARGETS(/bin cmake)

