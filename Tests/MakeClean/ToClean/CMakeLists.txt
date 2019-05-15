cmake_minimum_required(VERSION 3.14)
project(ToClean)

# Utility variables
set(TSD ${ToClean_SOURCE_DIR})
set(TBD ${ToClean_BINARY_DIR})
set(CLEAN_FILE_CONTENT "File registered for cleaning.\n")

# Lists build-time-generated files that should be cleaned away
set(TOCLEAN_FILES)

# Build a simple project whose compiled objects should be cleaned.
add_executable(toclean toclean.cxx)
list(APPEND TOCLEAN_FILES
  "${TBD}${CMAKE_FILES_DIRECTORY}/toclean.dir/toclean.cxx${CMAKE_CXX_OUTPUT_EXTENSION}")

# Create a custom command whose output should be cleaned.
set(CustomCommandFile "${TBD}/CustomCommandFile.txt")
add_custom_command(OUTPUT ${CustomCommandFile}
  DEPENDS ${TSD}/toclean.cxx
  COMMAND ${CMAKE_COMMAND}
  ARGS -E copy ${TSD}/toclean.cxx ${CustomCommandFile})
add_custom_target(generate ALL DEPENDS ${CustomCommandFile})
list(APPEND TOCLEAN_FILES ${CustomCommandFile})


### Tests ADDITIONAL_MAKE_CLEAN_FILES directory property
if("${CMAKE_GENERATOR}" MATCHES "Makefile")
  # Create a file that must be registered for cleaning.
  set(MakeDirPropFile "${TBD}/MakeDirPropFile.txt")
  file(WRITE "${MakeDirPropFile}" ${CLEAN_FILE_CONTENT})
  set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${MakeDirPropFile}")
  list(APPEND TOCLEAN_FILES "${MakeDirPropFile}")

  # Create a custom command whose output should be cleaned, but whose name
  # is not known until generate-time
  set(MakeDirPropExpFileRel "MakeDirProp_copy${CMAKE_EXECUTABLE_SUFFIX}")
  set(MakeDirPropExpFile "$<TARGET_FILE_DIR:toclean>/${MakeDirPropExpFileRel}")
  add_custom_command(TARGET toclean POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy $<TARGET_FILE:toclean> ${MakeDirPropExpFile})
  set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${MakeDirPropExpFile})
  list(APPEND TOCLEAN_FILES "${TBD}/${MakeDirPropExpFileRel}")
endif()


### Tests ADDITIONAL_CLEAN_FILES directory property

# Register a file path relative to the build directory
set(DirPropFileRel "DirPropFileRel.txt")
file(WRITE "${TBD}/${DirPropFileRel}" ${CLEAN_FILE_CONTENT})
set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES ${DirPropFileRel})
list(APPEND TOCLEAN_FILES "${TBD}/${DirPropFileRel}")

# Register an absolute file path
set(DirPropFileAbs "${TBD}/DirPropFileAbs.txt")
file(WRITE "${DirPropFileAbs}" ${CLEAN_FILE_CONTENT})
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${DirPropFileAbs})
list(APPEND TOCLEAN_FILES "${DirPropFileAbs}")

# Create a custom command whose output should be cleaned, but whose name
# is not known until generate-time
set(DirPropExpFileRel "DirProp_copy${CMAKE_EXECUTABLE_SUFFIX}")
set(DirPropExpFile "$<TARGET_FILE_DIR:toclean>/${DirPropExpFileRel}")
add_custom_command(TARGET toclean POST_BUILD
  COMMAND ${CMAKE_COMMAND}
  ARGS -E copy $<TARGET_FILE:toclean> ${DirPropExpFile})
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${DirPropExpFile})
list(APPEND TOCLEAN_FILES "${TBD}/${DirPropExpFileRel}")


### Tests ADDITIONAL_CLEAN_FILES target property

# Register a file path relative to the build directory
set(TgtPropFileRel "TargetPropFileRel.txt")
file(WRITE "${TBD}/${TgtPropFileRel}" ${CLEAN_FILE_CONTENT})
set_target_properties(toclean PROPERTIES ADDITIONAL_CLEAN_FILES ${TgtPropFileRel})
list(APPEND TOCLEAN_FILES "${TBD}/${TgtPropFileRel}")

# Register an absolute file path
set(TgtPropFileAbs "${TBD}/TargetPropFileAbs.txt")
file(WRITE "${TgtPropFileAbs}" ${CLEAN_FILE_CONTENT})
set_property(TARGET toclean APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${TgtPropFileAbs})
list(APPEND TOCLEAN_FILES "${TgtPropFileAbs}")

# Create a custom command whose output should be cleaned, but whose name
# is not known until generate-time
set(TgtPropExpFileRel "TgtProp_copy${CMAKE_EXECUTABLE_SUFFIX}")
set(TgtPropExpFile "$<TARGET_FILE_DIR:toclean>/${TgtPropExpFileRel}")
add_custom_command(TARGET toclean POST_BUILD
  COMMAND ${CMAKE_COMMAND}
  ARGS -E copy $<TARGET_FILE:toclean> ${TgtPropExpFile})
set_property(TARGET toclean APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${TgtPropExpFile})
list(APPEND TOCLEAN_FILES "${TBD}/${TgtPropExpFileRel}")


# Configure a file listing these build-time-generated files.
configure_file(${TSD}/ToCleanFiles.cmake.in ${TBD}/ToCleanFiles.cmake @ONLY)
