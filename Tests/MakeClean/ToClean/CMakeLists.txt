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

set(DUMMY_CONTENT_FILE ${CSD}/toclean.cxx)

# Build a simple project whose compiled objects should be cleaned.
add_executable(toclean toclean.cxx)
addCleanFile(
  "${CBD}${CMAKE_FILES_DIRECTORY}/toclean.dir/toclean.cxx${CMAKE_CXX_OUTPUT_EXTENSION}")

# Create a post build custom command that copies a dummy file
# to a custom location
function(addPostBuildFile TARGET FILENAME)
  add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy ${DUMMY_CONTENT_FILE} ${FILENAME})
endfunction()

# Create a custom command whose output should be cleaned.
set(CustomCommandFile "${CBD}/CustomCommandFile.txt")
add_custom_command(OUTPUT ${CustomCommandFile}
  DEPENDS ${DUMMY_CONTENT_FILE}
  COMMAND ${CMAKE_COMMAND}
  ARGS -E copy ${DUMMY_CONTENT_FILE} ${CustomCommandFile})
add_custom_target(customTarget ALL DEPENDS ${CustomCommandFile})
addCleanFile(${CustomCommandFile})


### Tests ADDITIONAL_MAKE_CLEAN_FILES directory property
if("${CMAKE_GENERATOR}" MATCHES "Makefile")
  # Create a file that must be registered for cleaning.
  set(MakeDirPropFileAbs "${CBD}/MakeDirPropFile.txt")
  writeCleanFile("${MakeDirPropFileAbs}")
  set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${MakeDirPropFileAbs}")
  addCleanFile(${MakeDirPropFileAbs})

  # Create a custom command whose output should be cleaned, but whose name
  # is not known until generate-time
  set(MakeDirPropExpFileRel "MakeDirProp_copy${CMAKE_EXECUTABLE_SUFFIX}")
  set(MakeDirPropExpFileAbs "$<TARGET_FILE_DIR:toclean>/${MakeDirPropExpFileRel}")
  addPostBuildFile(toclean "${MakeDirPropExpFileAbs}")
  set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${MakeDirPropExpFileAbs})
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
set(DirPropExpFileAbs "$<TARGET_FILE_DIR:toclean>/${DirPropExpFileRel}")
addPostBuildFile(toclean "${DirPropExpFileAbs}")
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${DirPropExpFileAbs})
addCleanFile("${CBD}/${DirPropExpFileRel}")


### Tests ADDITIONAL_CLEAN_FILES target property

function(test_target_property TARGET)
    # Register a file path relative to the build directory
    set(TgtPropFileRel "${TARGET}_TargetPropFileRel.txt")
    writeCleanFile("${CBD}/${TgtPropFileRel}")
    set_target_properties(${TARGET} PROPERTIES ADDITIONAL_CLEAN_FILES ${TgtPropFileRel})
    addCleanFile("${CBD}/${TgtPropFileRel}")

    # Register an absolute file path
    set(TgtPropFileAbs "${CBD}/${TARGET}_TargetPropFileAbs.txt")
    writeCleanFile("${TgtPropFileAbs}")
    set_property(TARGET ${TARGET} APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${TgtPropFileAbs})
    addCleanFile("${TgtPropFileAbs}")

    # Create a custom command whose output should be cleaned, but whose name
    # is not known until generate-time
    set(TgtPropExpFileRel "${TARGET}_TargetPropGenExp.txt")
    set(TgtPropExpFileAbs "$<TARGET_FILE_DIR:toclean>/${TgtPropExpFileRel}")
    addPostBuildFile(${TARGET} "${TgtPropExpFileAbs}")
    set_property(TARGET ${TARGET} APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${TgtPropExpFileAbs})
    addCleanFile("${CBD}/${TgtPropExpFileRel}")
endfunction()

# Test target property for various target types
add_executable(acf_exec toclean.cxx)
test_target_property(acf_exec)
add_library(acf_lib toclean.cxx)
test_target_property(acf_lib)
add_custom_target(acf_custom ALL DEPENDS ${CustomCommandFile})
test_target_property(acf_custom)

# Process subdirectory without targets
add_subdirectory(EmptySubDir)


# Configure a file listing these build-time-generated files.
get_property(TOCLEAN_FILES GLOBAL PROPERTY TOCLEAN_FILES)
configure_file(${CSD}/ToCleanFiles.cmake.in ${CBD}/ToCleanFiles.cmake @ONLY)
