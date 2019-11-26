cmake_minimum_required(VERSION 3.8)
project(TestFindDoxygen VERSION 1.0 LANGUAGES NONE)

find_package(Doxygen REQUIRED)

doxygen_add_docs(docsWithoutFilesWithStamp USE_STAMP_FILE)
if(NOT EXISTS "${PROJECT_BINARY_DIR}/Doxyfile.docsWithoutFilesWithStamp")
    message(FATAL_ERROR "Missing generated file: Doxyfile.docsWithoutFilesWithStamp")
endif()
if(NOT TARGET docsWithoutFilesWithStamp)
    message(FATAL_ERROR "Target docsWithoutFilesWithStamp not created")
endif()

doxygen_add_docs(docsWithFilesWithStamp main.cpp main2.cpp USE_STAMP_FILE)
if(NOT EXISTS "${PROJECT_BINARY_DIR}/Doxyfile.docsWithFilesWithStamp")
    message(FATAL_ERROR "Missing generated file: Doxyfile.docsWithFilesWithStamp")
endif()
if(NOT TARGET docsWithFilesWithStamp)
    message(FATAL_ERROR "Target docsWithFilesWithStamp not created")
endif()


add_custom_target(allDocTargets)
add_dependencies(allDocTargets docsWithoutFilesWithStamp docsWithFilesWithStamp)
