cmake_minimum_required (VERSION 2.8)
project(AllFindModules)

if (POLICY CMP0017)
  cmake_policy(SET CMP0017 NEW)
endif ()

file(GLOB FIND_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/../../../Modules/Find*.cmake" )

foreach(FIND_MODULE ${FIND_MODULES})
    string(REGEX REPLACE ".*/Find(.*)\\.cmake$" "\\1" MODULE_NAME "${FIND_MODULE}")

    # It is only possible to use either Qt3 or Qt4 in one project.
    # Since FindQt will complain if both are found we explicitely
    # filter out this and FindQt3. FindKDE3 also depends on Qt3 and
    # is therefore also blocked

    if (NOT MODULE_NAME STREQUAL "Qt" AND
        NOT MODULE_NAME STREQUAL "Qt3" AND
        NOT MODULE_NAME STREQUAL "KDE3")
        message(STATUS "   Checking Find${MODULE_NAME}")
        find_package(${MODULE_NAME})
    endif ()

endforeach(FIND_MODULE)
