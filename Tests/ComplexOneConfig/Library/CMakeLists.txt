#
# Small utility used to create file
# UTILITY_SOURCE is used for coverage and for getting the exact name
# of the executable.
#
UTILITY_SOURCE(CREATE_FILE_EXE create_file "." create_file.cxx)
ADD_EXECUTABLE(create_file create_file.cxx)

#
# Create static library
# SOURCE_FILES_REMOVE is used for Coverage. empty.h is included for coverage
#
AUX_SOURCE_DIRECTORY(ExtraSources LibrarySources)
SOURCE_FILES(LibrarySources 
  file2 
  empty 
  create_file.cxx 
  GENERATED 
  nonexisting_file)
SOURCE_FILES_REMOVE(LibrarySources create_file.cxx GENERATED nonexisting_file) 
ADD_LIBRARY(CMakeTestLibrary ${LibrarySources})

IF(WIN32)
  IF(NOT CYGWIN)
    IF(NOT BORLAND)
      IF(NOT MINGW)
        TARGET_LINK_LIBRARIES(CMakeTestLibrary
          debug 
          user32.lib)
        TARGET_LINK_LIBRARIES(CMakeTestLibrary
          optimized 
          kernel32.lib)
      ENDIF(NOT MINGW)
    ENDIF(NOT BORLAND)
  ENDIF(NOT CYGWIN)
ENDIF(WIN32)

#
# Create shared library
#
SOURCE_FILES(SharedLibrarySources sharedFile)
ADD_LIBRARY(CMakeTestLibraryShared SHARED ${SharedLibrarySources})
ADD_LIBRARY(CMakeTestModule MODULE moduleFile.c)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DTEST_C_FLAGS")
ADD_LIBRARY(CMakeTestCLibraryShared SHARED testConly.c)
SET_TARGET_PROPERTIES(CMakeTestCLibraryShared PROPERTIES FOO BAR)
SET_TARGET_PROPERTIES(CMakeTestCLibraryShared PROPERTIES LINK_FLAGS "-lm")
GET_TARGET_PROPERTY(FOO_BAR_VAR CMakeTestCLibraryShared FOO)
IF(APPLE)
  SET_TARGET_PROPERTIES(CMakeTestModule PROPERTIES LINK_FLAGS "-bundle_loader ${Complex_BINARY_DIR}/bin/create_file")
ENDIF(APPLE)
IF(${FOO_BAR_VAR} MATCHES "BAR")
ELSE(${FOO_BAR_VAR} MATCHES "BAR")
  MESSAGE(SEND_ERROR "SET_TARGET_PROPERTIES or GET_TARGET_PROPERTY failed, FOO_BAR_VAR should be BAR, but is ${FOO_BAR_VAR}")
ENDIF(${FOO_BAR_VAR} MATCHES "BAR")


#
# Attach a post-build custom-command to the lib.
# It runs ${CREATE_FILE_EXE} which will create a file.
# The 'complex' executable will then test if this file exists and remove it.
#
ADD_DEPENDENCIES(CMakeTestLibraryShared create_file)
MESSAGE("complex bin dir is ${Complex_BINARY_DIR}")
ADD_CUSTOM_COMMAND(COMMAND ${CREATE_FILE_EXE}
                   ARGS "${Complex_BINARY_DIR}/Library/postbuild.txt"
                   TARGET CMakeTestLibraryShared)

ADD_CUSTOM_COMMAND(COMMAND ${CMAKE_COMMAND}
                   ARGS -E copy
                        "${Complex_BINARY_DIR}/Library/postbuild.txt"
                        "${Complex_BINARY_DIR}/Library/postbuild2.txt"
                   TARGET CMakeTestLibraryShared)

#
# Add a custom target.
# It runs ${CREATE_FILE_EXE} which will create a file.
# The 'complex' executable will then test if this file exists and remove it.
#
ADD_CUSTOM_TARGET(custom_target1
                  ALL
                  ${CREATE_FILE_EXE} 
                  "${Complex_BINARY_DIR}/Library/custom_target1.txt")

ADD_DEPENDENCIES(custom_target1 create_file)

#
# Extra coverage
#
ABSTRACT_FILES(
  file2
)

INSTALL_FILES(/tmp .h ${Complex_BINARY_DIR}/cmTestConfigure.h)
INSTALL_FILES(/tmp .cxx ${Complex_BINARY_DIR}/cmTestConfigure.h)

