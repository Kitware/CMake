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

#
# Create shared library
#

# test SET_SOURCE_FILES_PROPERTIES 
SET_SOURCE_FILES_PROPERTIES(nonexisting_file2 GENERATED ABSTRACT WRAP_EXCLUDE COMPILE_FLAGS "-foo -bar")
GET_SOURCE_FILE_PROPERTY(ISABS nonexisting_file2 ABSTRACT)
GET_SOURCE_FILE_PROPERTY(WRAPEX nonexisting_file2 WRAP_EXCLUDE)
GET_SOURCE_FILE_PROPERTY(FLAGS nonexisting_file2 COMPILE_FLAGS)
MESSAGE("ISABS = ${ISABS}   WRAPEX =   ${WRAPEX}   FLAGS = ${FLAGS}")

CONFIGURE_FILE(
  ${Complex_SOURCE_DIR}/Library/cmTestLibraryConfigure.h.in
  ${Complex_BINARY_DIR}/Library/cmTestLibraryConfigure.h)


  

SOURCE_FILES(SharedLibrarySources nonexisting_file2)
SOURCE_FILES_REMOVE(SharedLibrarySources GENERATED nonexisting_file2)


SET_SOURCE_FILES_PROPERTIES(fileFlags COMPILE_FLAGS "-DEXTRA_FLAG" )
SOURCE_FILES(SharedLibrarySources sharedFile fileFlags)
ADD_LIBRARY(CMakeTestLibraryShared SHARED SharedLibrarySources)

#
# Attach a post-build custom-command to the lib.
# It runs ${CREATE_FILE_EXE} which will create a file.
# The 'complex' executable will then test if this file exists and remove it.
#
ADD_DEPENDENCIES(CMakeTestLibraryShared create_file)

ADD_CUSTOM_COMMAND(SOURCE CMakeTestLibraryShared
                   COMMAND ${CREATE_FILE_EXE}
                   ARGS "${Complex_BINARY_DIR}/Library/postbuild.txt"
                   TARGET CMakeTestLibraryShared)

#
# Add a custom target.
# It runs ${CREATE_FILE_EXE} which will create the file
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

