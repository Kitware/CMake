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
ADD_LIBRARY(CMakeTestLibrary LibrarySources)

IF(WIN32)
  IF(NOT CYGWIN)
    TARGET_LINK_LIBRARIES(CMakeTestLibrary
                      debug 
                      user32.lib)
    TARGET_LINK_LIBRARIES(CMakeTestLibrary
                      optimized 
                      kernel32.lib)
  ENDIF(NOT CYGWIN)
ENDIF(WIN32)

#
# Create shared library
#
SOURCE_FILES(SharedLibrarySources sharedFile)
ADD_LIBRARY(CMakeTestLibraryShared SHARED SharedLibrarySources)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DTEST_C_FLAGS")
ADD_LIBRARY(CMakeTestCLibraryShared SHARED testConly.c)

#
# Attach a post-build custom-command to the lib.
# It runs ${CREATE_FILE_EXE} which will create a file.
# The 'complex' executable will then test if this file exists and remove it.
#
ADD_DEPENDENCIES(CMakeTestLibraryShared create_file)

ADD_CUSTOM_COMMAND(COMMAND ${CREATE_FILE_EXE}
                   ARGS "${Complex_BINARY_DIR}/Library/postbuild.txt"
                   TARGET CMakeTestLibraryShared)

ADD_CUSTOM_COMMAND(COMMAND ${CCOMMAND_COMMAND}
                   ARGS copy
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

