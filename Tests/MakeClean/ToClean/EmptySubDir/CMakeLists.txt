cmake_minimum_required(VERSION 3.14)

# Subdirectory CMakeLists.txt without targets
set(CSD ${CMAKE_CURRENT_SOURCE_DIR})
set(CBD ${CMAKE_CURRENT_BINARY_DIR})

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
