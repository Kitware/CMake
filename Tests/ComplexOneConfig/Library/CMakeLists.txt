AUX_SOURCE_DIRECTORY(ExtraSources LibrarySources)

SOURCE_FILES(LibrarySources file2)
ADD_LIBRARY(CMakeTestLibrary LibrarySources)

SOURCE_FILES(SharedLibrarySources sharedFile)
ADD_LIBRARY(CMakeTestLibraryShared SHARED SharedLibrarySources)

UTILITY_SOURCE(CREATE_FILE_EXE create_file "." create_file.cxx)
ADD_EXECUTABLE(create_file create_file.cxx)

ADD_DEPENDENCIES(CMakeTestLibraryShared create_file)

# Attach a post-build custom-command to the lib.
# It run ${CREATE_FILE_EXE} which will create the file
# ${Complex_BINARY_DIR}/postbuild.txt.
# The 'complex' executable will then test if this file exists,
# and remove it.

ADD_CUSTOM_COMMAND(SOURCE CMakeTestLibraryShared
                   COMMAND ${CREATE_FILE_EXE}
                   ARGS "${Complex_BINARY_DIR}/postbuild.txt"
                   TARGET CMakeTestLibraryShared)

