AUX_SOURCE_DIRECTORY(ExtraSources LibrarySources)

# SOURCE_FILES_REMOVE is used for Coverage
SOURCE_FILES(LibrarySources file2 create_file.cxx)
SOURCE_FILES_REMOVE(LibrarySources create_file.cxx) 
ADD_LIBRARY(CMakeTestLibrary LibrarySources)

SOURCE_FILES(SharedLibrarySources sharedFile)
ADD_LIBRARY(CMakeTestLibraryShared SHARED SharedLibrarySources)

#
# Small utility used to create file
#
UTILITY_SOURCE(CREATE_FILE_EXE create_file "." create_file.cxx)
ADD_EXECUTABLE(create_file create_file.cxx)

#
# Attach a post-build custom-command to the lib.
# It run ${CREATE_FILE_EXE} which will create the file
# ${Complex_BINARY_DIR}/postbuild.txt.
# The 'complex' executable will then test if this file exists,
# and remove it.
#
ADD_DEPENDENCIES(CMakeTestLibraryShared create_file)

ADD_CUSTOM_COMMAND(SOURCE CMakeTestLibraryShared
                   COMMAND ${CREATE_FILE_EXE}
                   ARGS "${Complex_BINARY_DIR}/postbuild.txt"
                   TARGET CMakeTestLibraryShared)

#
# Add custom target
# It run ${CREATE_FILE_EXE} which will create the file
# ${Complex_BINARY_DIR}/custom_target1.txt.
# The 'complex' executable will then test if this file exists,
# and remove it.
#
ADD_CUSTOM_TARGET(custom_target1
                  ALL
                  ${CREATE_FILE_EXE} 
                  "${Complex_BINARY_DIR}/custom_target1.txt")

ADD_DEPENDENCIES(custom_target1 create_file)

