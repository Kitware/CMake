cmake_minimum_required (VERSION 2.8)
project(AllFindModules)

if (POLICY CMP0017)
  cmake_policy(SET CMP0017 NEW)
endif ()

# Avoid ctest truncation of output
message(STATUS "CTEST_FULL_OUTPUT")

file(GLOB FIND_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/../../../Modules/Find*.cmake" )

macro(do_find MODULE_NAME)
    message(STATUS "   Checking Find${MODULE_NAME}")
    find_package(${MODULE_NAME})
endmacro(do_find)

# It is only possible to use either Qt3 or Qt4 in one project.
# Since FindQt will complain if both are found we explicitely
# filter out this and FindQt3. FindKDE3 also depends on Qt3 and
# is therefore also blocked
set(NO_QT4_MODULES "Qt3" "KDE3")

# These modules are named Find*.cmake, but are nothing that works in
# find_package().
set(NO_FIND_MODULES "PackageHandleStandardArgs" "PackageMessage")

set(DESIRED_QT_VERSION 4)
foreach(FIND_MODULE ${FIND_MODULES})
    string(REGEX REPLACE ".*/Find(.*)\\.cmake$" "\\1" MODULE_NAME "${FIND_MODULE}")

    list(FIND NO_QT4_MODULES ${MODULE_NAME} NO_QT4_INDEX)
    list(FIND NO_FIND_MODULES ${MODULE_NAME} NO_FIND_INDEX)
    if (NO_QT4_INDEX EQUAL -1 AND NO_FIND_INDEX EQUAL -1)
        do_find(${MODULE_NAME})
    endif ()

endforeach(FIND_MODULE)

# Qt4 is not present, so we can check Qt3
if (NOT QT4_FOUND)
    set(DESIRED_QT_VERSION 3)
    foreach(FIND_MODULE ${NO_QT4_MODULES} "Qt")
        do_find(${FIND_MODULE})
    endforeach(FIND_MODULE)
endif (NOT QT4_FOUND)
