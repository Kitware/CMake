cmake_minimum_required(VERSION 3.10)
project(TestFindDoxygen VERSION 1.0 LANGUAGES NONE)
enable_testing()

#[[
The ADDITIONAL_CLEAN_FILES property "only works for the Ninja
and the Makefile generators. It is ignored by other generators."

Therefore, we must skip this test for other generators.

https://cmake.org/cmake/help/latest/prop_tgt/ADDITIONAL_CLEAN_FILES.html#prop_tgt:ADDITIONAL_CLEAN_FILES
]]

set(DISABLE_THIS_TEST TRUE)

if(CMAKE_GENERATOR MATCHES "Ninja|Makefile")
    set(DISABLE_THIS_TEST FALSE)
endif()

# Skip calling Doxygen if the generator does not support the 'clean' target
if(NOT DISABLE_THIS_TEST)
    find_package(Doxygen REQUIRED)

    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/main.cpp
    [[/**
     * \file
     * \brief One C++ file w/ sample Doxygen comment just to produce any docs...
     */
    ]])

    set(OUTPUT_FORMATS DOCBOOK HTML LATEX MAN RTF SQLITE3 XML)

    # test one target with direct parent/child directories
    set(DOXYGEN_OUTPUT_DIRECTORY "docs dir with spaces")
    set(DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${DOXYGEN_OUTPUT_DIRECTORY}")

    file(REMOVE_RECURSE "${DOXYGEN_OUTPUT_DIR}")

    foreach(format IN LISTS OUTPUT_FORMATS)
        set(DOXYGEN_GENERATE_${format} YES)
        set(DOXYGEN_${format}_OUTPUT "my ${format} output")
        cmake_path(ABSOLUTE_PATH DOXYGEN_${format}_OUTPUT
                   BASE_DIRECTORY "${DOXYGEN_OUTPUT_DIR}"
                   NORMALIZE
                   OUTPUT_VARIABLE EXPECTED_${format}_DIR)

        list(APPEND DIRS_TO_CLEAN "${EXPECTED_${format}_DIR}")
    endforeach()

    doxygen_add_docs(docsClean ALL ${CMAKE_CURRENT_BINARY_DIR}/main.cpp)

    # test another target with relative paths
    set(DOXYGEN_OUTPUT_DIRECTORY "docs dir/with/subdirs")
    set(DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${DOXYGEN_OUTPUT_DIRECTORY}")

    file(REMOVE_RECURSE "${DOXYGEN_OUTPUT_DIR}")

    SET(DOXYGEN_DOCBOOK_OUTPUT "docbook")
    SET(DOXYGEN_HTML_OUTPUT "make/this/dir/html")
    SET(DOXYGEN_LATEX_OUTPUT "../latex")
    SET(DOXYGEN_MAN_OUTPUT "../r/t/f/m")
    SET(DOXYGEN_RTF_OUTPUT "../../r t f")
    SET(DOXYGEN_SQLITE3_OUTPUT "../../../sqlite3")
    SET(DOXYGEN_XML_OUTPUT "./../xml")

    foreach(format IN LISTS OUTPUT_FORMATS)
        set(_doxygen_format_output "DOXYGEN_${format}_OUTPUT")
        cmake_path(ABSOLUTE_PATH DOXYGEN_${format}_OUTPUT
                   BASE_DIRECTORY "${DOXYGEN_OUTPUT_DIR}"
                   NORMALIZE
                   OUTPUT_VARIABLE EXPECTED_${format}_DIR)
        list(APPEND DIRS_TO_CLEAN "${EXPECTED_${format}_DIR}")
    endforeach()

    doxygen_add_docs(docsCleanRelative ALL ${CMAKE_CURRENT_BINARY_DIR}/main.cpp)
endif()

add_test(NAME testDocsClean
    COMMAND ${CMAKE_COMMAND}
    -D BUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
    -D "OUTPUT_DIRS=${DIRS_TO_CLEAN}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/TestClean.cmake"
)

set_tests_properties(testDocsClean PROPERTIES DISABLED ${DISABLE_THIS_TEST})
