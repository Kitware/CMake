cmake_minimum_required(VERSION 3.14)
project(ToClean)

# Utility variables
set(CSD ${CMAKE_CURRENT_SOURCE_DIR})
set(CBD ${CMAKE_CURRENT_BINARY_DIR})
set(CLEAN_FILE_CONTENT "File registered for cleaning.\n")

# Lists build-time-generated files that should be cleaned away
set_property(GLOBAL PROPERTY TOCLEAN_FILES "")
function(addCleanFile FILENAME)
  set_property(GLOBAL APPEND PROPERTY TOCLEAN_FILES "${FILENAME}")
endfunction()
function(writeCleanFile FILENAME)
  file(WRITE "${FILENAME}" ${CLEAN_FILE_CONTENT})
endfunction()

# Build a simple project whose compiled objects should be cleaned.
add_executable(toclean toclean.cxx)
addCleanFile("${CBD}${CMAKE_FILES_DIRECTORY}/toclean.dir/toclean.cxx${CMAKE_CXX_OUTPUT_EXTENSION}")

# Create a post build custom command that copies the toclean output executable
# to a custom location
function(addToCleanPostBuildCopy FILENAME)
  add_custom_command(TARGET toclean POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy $<TARGET_FILE:toclean> ${FILENAME})
endfunction()

# Create a custom command whose output should be cleaned.
set(CustomCommandFile "${CBD}/CustomCommandFile.txt")
add_custom_command(OUTPUT ${CustomCommandFile}
  DEPENDS ${CSD}/toclean.cxx
  COMMAND ${CMAKE_COMMAND}
  ARGS -E copy ${CSD}/toclean.cxx ${CustomCommandFile})
add_custom_target(generate ALL DEPENDS ${CustomCommandFile})
addCleanFile(${CustomCommandFile})


### Tests ADDITIONAL_MAKE_CLEAN_FILES directory property
if("${CMAKE_GENERATOR}" MATCHES "Makefile")
  # Create a file that must be registered for cleaning.
  set(MakeDirPropFile "${CBD}/MakeDirPropFile.txt")
  writeCleanFile("${MakeDirPropFile}")
  set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${MakeDirPropFile}")
  addCleanFile(${MakeDirPropFile})

  # Create a custom command whose output should be cleaned, but whose name
  # is not known until generate-time
  set(MakeDirPropExpFileRel "MakeDirProp_copy${CMAKE_EXECUTABLE_SUFFIX}")
  set(MakeDirPropExpFile "$<TARGET_FILE_DIR:toclean>/${MakeDirPropExpFileRel}")
  addToCleanPostBuildCopy("${MakeDirPropExpFile}")
  set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${MakeDirPropExpFile})
  addCleanFile("${CBD}/${MakeDirPropExpFileRel}")
endif()


### Tests ADDITIONAL_CLEAN_FILES directory property

# Register a file path relative to the build directory
set(DirPropFileRel "DirPropFileRel.txt")
writeCleanFile("${CBD}/${DirPropFileRel}")
set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES ${DirPropFileRel})
addCleanFile("${CBD}/${DirPropFileRel}")

# Register an absolute file path
set(DirPropFileAbs "${CBD}/DirPropFileAbs.txt")
writeCleanFile("${DirPropFileAbs}")
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${DirPropFileAbs})
addCleanFile("${DirPropFileAbs}")

# Create a custom command whose output should be cleaned, but whose name
# is not known until generate-time
set(DirPropExpFileRel "DirProp_copy${CMAKE_EXECUTABLE_SUFFIX}")
set(DirPropExpFile "$<TARGET_FILE_DIR:toclean>/${DirPropExpFileRel}")
addToCleanPostBuildCopy("${DirPropExpFile}")
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${DirPropExpFile})
addCleanFile("${CBD}/${DirPropExpFileRel}")


### Tests ADDITIONAL_CLEAN_FILES target property

# Register a file path relative to the build directory
set(TgtPropFileRel "TargetPropFileRel.txt")
writeCleanFile("${CBD}/${TgtPropFileRel}")
set_target_properties(toclean PROPERTIES ADDITIONAL_CLEAN_FILES ${TgtPropFileRel})
addCleanFile("${CBD}/${TgtPropFileRel}")

# Register an absolute file path
set(TgtPropFileAbs "${CBD}/TargetPropFileAbs.txt")
writeCleanFile("${TgtPropFileAbs}")
set_property(TARGET toclean APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${TgtPropFileAbs})
addCleanFile("${TgtPropFileAbs}")

# Create a custom command whose output should be cleaned, but whose name
# is not known until generate-time
set(TgtPropExpFileRel "TgtProp_copy${CMAKE_EXECUTABLE_SUFFIX}")
set(TgtPropExpFile "$<TARGET_FILE_DIR:toclean>/${TgtPropExpFileRel}")
addToCleanPostBuildCopy("${TgtPropExpFile}")
set_property(TARGET toclean APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${TgtPropExpFile})
addCleanFile("${CBD}/${TgtPropExpFileRel}")


# Process subdirectory without targets
add_subdirectory(EmptySubDir)


# Configure a file listing these build-time-generated files.
get_property(TOCLEAN_FILES GLOBAL PROPERTY TOCLEAN_FILES)
configure_file(${CSD}/ToCleanFiles.cmake.in ${CBD}/ToCleanFiles.cmake @ONLY)
