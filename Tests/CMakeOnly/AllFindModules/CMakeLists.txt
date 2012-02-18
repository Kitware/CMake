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
# Since FindQt will complain if both are found we explicitly request Qt4 here
# and filter out FindQt3. FindKDE3 also depends on Qt3 and
# is therefore also blocked
set(DESIRED_QT_VERSION 4)
set(NO_QT4_MODULES "Qt3" "KDE3")

# These modules are named Find*.cmake, but are nothing that works in
# find_package().
set(NO_FIND_MODULES "PackageHandleStandardArgs" "PackageMessage")

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

# If any of these modules reported that it was found a version number should have been
# reported.
set(VERSIONS_REQUIRED
    ALSA BISON BZIP2 CUPS CURL DOXYGEN EXPAT FLEX FREETYPE GETTEXT GIF GIT
    ImageMagick JASPER LibArchive LIBXML2 LIBXSLT PERL PKG_CONFIG PostgreSQL
    SWIG TIFF ZLIB)

foreach(VTEST ${VERSIONS_REQUIRED})
    if (${VTEST}_FOUND)
        if (DEFINED ${VTEST}_VERSION_STRING)
            if (NOT ${VTEST}_VERSION_STRING MATCHES "^[0-9][0-9\\.]*[A-Za-z_]*[0-9\\.]*$")
                message(SEND_ERROR "${VTEST}_VERSION_STRING has unexpected content ${${VTEST}_VERSION_STRING}")
            endif()
        elseif (DEFINED ${VTEST}_VERSION)
            if (NOT ${VTEST}_VERSION MATCHES "^[0-9][0-9\\.]*[A-Za-z_]*[0-9\\.]*$")
                message(SEND_ERROR "${VTEST}_VERSION has unexpected content ${${VTEST}_VERSION}")
            endif()
        else()
            message(SEND_ERROR "${VTEST}_FOUND is set but no version number is defined")
        endif()
    endif(${VTEST}_FOUND)
endforeach(VTEST)
