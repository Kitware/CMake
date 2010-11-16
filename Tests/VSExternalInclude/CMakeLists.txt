cmake_minimum_required (VERSION 2.6)
PROJECT(VSExternalInclude)

IF(${CMAKE_GENERATOR} MATCHES "Visual Studio 6")
  SET(PROJECT_EXT dsp)
ELSE(${CMAKE_GENERATOR} MATCHES "Visual Studio 6")
  SET(PROJECT_EXT vcproj)
ENDIF(${CMAKE_GENERATOR} MATCHES "Visual Studio 6")
IF(${CMAKE_GENERATOR} MATCHES "Visual Studio 10")
  SET(PROJECT_EXT vcxproj)
ENDIF()

# make sure directories exists
SET(LIB1_BINARY_DIR ${VSExternalInclude_BINARY_DIR}/Lib1)
MAKE_DIRECTORY("${LIB1_BINARY_DIR}")

SET(LIB2_BINARY_DIR ${VSExternalInclude_BINARY_DIR}/Lib2)
MAKE_DIRECTORY("${LIB2_BINARY_DIR}")

# generate lib1
EXEC_PROGRAM("${CMAKE_COMMAND}" "${LIB1_BINARY_DIR}"  ARGS -G\"${CMAKE_GENERATOR}\" 
  \"${VSExternalInclude_SOURCE_DIR}/Lib1\" OUTPUT_VARIABLE OUT)
MESSAGE("CMAKE Ran with the following output:\n\"${OUT}\"")

# generate lib2
EXEC_PROGRAM("${CMAKE_COMMAND}" "${LIB2_BINARY_DIR}"  ARGS -G\"${CMAKE_GENERATOR}\" 
  \"${VSExternalInclude_SOURCE_DIR}/Lib2\" OUTPUT_VARIABLE OUT)
MESSAGE("CMAKE Ran with the following output:\n\"${OUT}\"")


INCLUDE_EXTERNAL_MSPROJECT(lib1 ${VSExternalInclude_BINARY_DIR}/Lib1/LIB1.${PROJECT_EXT})
# lib2 depends on lib1
INCLUDE_EXTERNAL_MSPROJECT(lib2 ${VSExternalInclude_BINARY_DIR}/Lib2/LIB2.${PROJECT_EXT} lib1)

INCLUDE_DIRECTORIES(${VSExternalInclude_SOURCE_DIR}/Lib2 ${VSExternalInclude_SOURCE_DIR}/Lib1)

SET(SOURCES main.cpp)

ADD_EXECUTABLE(VSExternalInclude ${SOURCES})

# target depends on lib2
ADD_DEPENDENCIES(VSExternalInclude lib2)
# VS 10 vcxproj files have depends in them
# Since lib1 and lib2 do not depend on each other
# then the vcxproj files do not depend on each other
# and the sln file can no longer be the only source
# of that depend.  So, for VS 10 make the executable
# depend on lib1 and lib2
IF(MSVC10)
  ADD_DEPENDENCIES(VSExternalInclude lib1)
ENDIF()

# Interaction testing between the FOLDER target property and
# INCLUDE_EXTERNAL_MSPROJECT targets:
set_target_properties(VSExternalInclude PROPERTIES FOLDER folder1/folder2)
set_target_properties(lib1 PROPERTIES FOLDER folder1/folder2)
set_target_properties(lib2 PROPERTIES FOLDER folder1/folder2)
add_custom_target(EmptyCustomTarget)
set_target_properties(EmptyCustomTarget PROPERTIES FOLDER folder1/folder2)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
