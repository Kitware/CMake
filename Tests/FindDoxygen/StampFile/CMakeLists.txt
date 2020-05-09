cmake_minimum_required(VERSION 3.8)
project(TestFindDoxygen VERSION 1.0 LANGUAGES NONE)

find_package(Doxygen REQUIRED)

set(DOXYGEN_OUTPUT_DIRECTORY noFiles)
doxygen_add_docs(docsWithoutFilesWithStamp USE_STAMP_FILE)
if(NOT EXISTS "${PROJECT_BINARY_DIR}/Doxyfile.docsWithoutFilesWithStamp")
  message(FATAL_ERROR "Missing generated file: Doxyfile.docsWithoutFilesWithStamp")
endif()
if(NOT TARGET docsWithoutFilesWithStamp)
  message(FATAL_ERROR "Target docsWithoutFilesWithStamp not created")
endif()

set(DOXYGEN_OUTPUT_DIRECTORY withFiles)
doxygen_add_docs(docsWithFilesWithStamp main.cpp main2.cpp USE_STAMP_FILE)
if(NOT EXISTS "${PROJECT_BINARY_DIR}/Doxyfile.docsWithFilesWithStamp")
  message(FATAL_ERROR "Missing generated file: Doxyfile.docsWithFilesWithStamp")
endif()
if(NOT TARGET docsWithFilesWithStamp)
  message(FATAL_ERROR "Target docsWithFilesWithStamp not created")
endif()

# Confirm that doxygen_add_docs() doesn't cause a fatal error if given a
# source file that is generated at build time
file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/genDox.cpp)
add_custom_command(OUTPUT genDox.cpp
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/main2.cpp genDox.cpp
  DEPENDS main2.cpp
)
set(DOXYGEN_OUTPUT_DIRECTORY withGenFiles)
doxygen_add_docs(docsWithGenFilesWithStamp
  main.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/genDox.cpp
  USE_STAMP_FILE
)

add_custom_target(allDocTargets)
add_dependencies(allDocTargets
  docsWithoutFilesWithStamp
  docsWithFilesWithStamp
  docsWithGenFilesWithStamp
)
