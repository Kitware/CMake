# - Find the KDE3 include and library dirs, KDE preprocessors and define a some macros
#
# This module defines the following variables:
# KDE3_DEFINITIONS         - compiler definitions required for compiling KDE software
# KDE3_INCLUDE_DIR         - the KDE include directory
# KDE3_INCLUDE_DIRS        - the KDE and the Qt include directory, for use with INCLUDE_DIRECTORIES()
# KDE3_LIB_DIR             - the directory where the KDE libraries are installed, for use with LINK_DIRECTORIES()
# QT_AND_KDECORE_LIBS      - this contains both the Qt and the kdecore library
# KDE3_DCOPIDL_EXECUTABLE  - the dcopidl executable
# KDE3_DCOPIDL2CPP_EXECUTABLE - the dcopidl2cpp executable
# KDE3_KCFGC_EXECUTABLE    - the kconfig_compiler executable
# KDE3_FOUND               - set to TRUE if all of the above has been found
#
# The following user adjustable options are provided:
#
# KDE3_BUILD_TESTS - enable this to build KDE testcases
#
#
# It also adds the following macros (from KDE3Macros.cmake)
# SRCS_VAR is always the variable which contains the list of source files for your application or library.
#
# KDE3_AUTOMOC(file1 ... fileN)
#    Call this if you want to have automatic moc file handling.
#    This means if you include "foo.moc" in the source file foo.cpp
#    a moc file for the header foo.h will be created automatically.
#    You can set the property SKIP_AUTOMAKE using SET_SOURCE_FILES_PROPERTIES()
#    to exclude some files in the list from being processed.
#
# KDE3_ADD_MOC_FILES(SRCS_VAR file1 ... fileN )
#    If you don't use the KDE3_AUTOMOC() macro, for the files
#    listed here moc files will be created (named "foo.moc.cpp")
#
# KDE3_ADD_DCOP_SKELS(SRCS_VAR header1.h ... headerN.h )
#    Use this to generate DCOP skeletions from the listed headers.
#
# KDE3_ADD_DCOP_STUBS(SRCS_VAR header1.h ... headerN.h )
#     Use this to generate DCOP stubs from the listed headers.
#
# KDE3_ADD_UI_FILES(SRCS_VAR file1.ui ... fileN.ui )
#    Use this to add the Qt designer ui files to your application/library.
#
# KDE3_ADD_KCFG_FILES(SRCS_VAR file1.kcfgc ... fileN.kcfgc )
#    Use this to add KDE kconfig compiler files to your application/library.
#
# KDE3_INSTALL_LIBTOOL_FILE(target)
#    This will create and install a simple libtool file for the given target.
#
# KDE3_ADD_EXECUTABLE(name file1 ... fileN )
#    Currently identical to ADD_EXECUTABLE(), may provide some advanced features in the future.
#
# KDE3_ADD_KPART(name [WITH_PREFIX] file1 ... fileN )
#    Create a KDE plugin (KPart, kioslave, etc.) from the given source files.
#    If WITH_PREFIX is given, the resulting plugin will have the prefix "lib", otherwise it won't.
#    It creates and installs an appropriate libtool la-file.
#
# KDE3_ADD_KDEINIT_EXECUTABLE(name file1 ... fileN )
#    Create a KDE application in the form of a module loadable via kdeinit.
#    A library named kdeinit_<name> will be created and a small executable which links to it.
#
# The option KDE3_ENABLE_FINAL to enable all-in-one compilation is
# no longer supported.
#
#
# Author: Alexander Neundorf <neundorf@kde.org>

IF(NOT UNIX)
   MESSAGE(FATAL_ERROR "Compiling KDE3 applications and libraries under Windows is not supported")
ENDIF(NOT UNIX)


SET(QT_MT_REQUIRED TRUE)
#SET(QT_MIN_VERSION "3.0.0")

#this line includes FindQt.cmake, which searches the Qt library and headers
FIND_PACKAGE(Qt3 REQUIRED)
FIND_PACKAGE(X11 REQUIRED)


#add some KDE specific stuff
SET(KDE3_DEFINITIONS -DQT_CLEAN_NAMESPACE -D_GNU_SOURCE)

#only on linux, but NOT e.g. on FreeBSD:
IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
   SET (KDE3_DEFINITIONS ${KDE3_DEFINITIONS} -D_XOPEN_SOURCE=500 -D_BSD_SOURCE)
   SET ( CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -Wno-long-long -ansi -Wundef -Wcast-align -Wconversion -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security -Wmissing-format-attribute -fno-common")
   SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Wno-long-long -ansi -Wundef -Wcast-align -Wconversion -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security -fno-exceptions -fno-check-new -fno-common")
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Linux")

# works on FreeBSD, NOT tested on NetBSD and OpenBSD
IF (CMAKE_SYSTEM_NAME MATCHES BSD)
   SET ( CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -Wno-long-long -ansi -Wundef -Wcast-align -Wconversion -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security -Wmissing-format-attribute -fno-common")
   SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Wno-long-long -Wundef -Wcast-align -Wconversion -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security -Wmissing-format-attribute -fno-exceptions -fno-check-new -fno-common")
ENDIF (CMAKE_SYSTEM_NAME MATCHES BSD)

# if no special buildtype is selected, add -O2 as default optimization
IF (NOT CMAKE_BUILD_TYPE)
   SET ( CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -O2")
   SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
ENDIF (NOT CMAKE_BUILD_TYPE)


#SET(CMAKE_SHARED_LINKER_FLAGS "-avoid-version -module -Wl,--no-undefined -Wl,--allow-shlib-undefined")
#SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,--fatal-warnings -avoid-version -Wl,--no-undefined -lc")
#SET(CMAKE_MODULE_LINKER_FLAGS "-Wl,--fatal-warnings -avoid-version -Wl,--no-undefined -lc")

# all calls to FIND_PROGRAM/PATH/LIBRARY() follow the same scheme:
# at first try some special paths with the argument "NO_DEFAULT_PATH"
# so only these paths are checked
# this is followed by a second call to FIND_PROGRAM/PATH/LIBRARY()
# but this time without any paths and without NO_DEFAULT_PATH
# this second call will do nothing if the first call already found
# what it was looking for, if not, it will search only in the default
# directories (/usr, /usr/local, etc.)

#now try to find some kde stuff
FIND_PROGRAM(KDECONFIG_EXECUTABLE NAMES kde-config PATHS
   $ENV{KDEDIR}/bin
  /opt/kde3/bin
  /opt/kde/bin
  NO_DEFAULT_PATH
  )


FIND_PROGRAM(KDECONFIG_EXECUTABLE kde-config)


SET(KDE3PREFIX)
IF(KDECONFIG_EXECUTABLE)
   EXECUTE_PROCESS(COMMAND ${KDECONFIG_EXECUTABLE} --version
                   OUTPUT_VARIABLE kde_config_version )

   STRING(REGEX MATCH "KDE: .\\." kde_version ${kde_config_version})
   IF (${kde_version} MATCHES "KDE: 3\\.")
      EXECUTE_PROCESS(COMMAND ${KDECONFIG_EXECUTABLE} --prefix
                        OUTPUT_VARIABLE kdedir )
      STRING(REGEX REPLACE "\n" "" KDE3PREFIX "${kdedir}")

    ENDIF (${kde_version} MATCHES "KDE: 3\\.")
ENDIF(KDECONFIG_EXECUTABLE)



# at first the KDE include direcory
# kpassdlg.h comes from kdeui and doesn't exist in KDE4 anymore
FIND_PATH(KDE3_INCLUDE_DIR kpassdlg.h
  $ENV{KDEDIR}/include
  ${KDE3PREFIX}/include
  /opt/kde3/include
  /opt/kde/include
  /usr/include/kde
  /usr/local/include/kde
  NO_DEFAULT_PATH
  )

FIND_PATH(KDE3_INCLUDE_DIR kpassdlg.h)

#now the KDE library directory
FIND_LIBRARY(KDE3_KDECORE_LIBRARY NAMES kdecore
  PATHS
  $ENV{KDEDIR}/lib
  ${KDE3PREFIX}/lib
  /opt/kde3/lib
  /opt/kde/lib
  NO_DEFAULT_PATH
)

FIND_LIBRARY(KDE3_KDECORE_LIBRARY NAMES kdecore)

SET(QT_AND_KDECORE_LIBS ${QT_LIBRARIES} ${KDE3_KDECORE_LIBRARY})

GET_FILENAME_COMPONENT(KDE3_LIB_DIR ${KDE3_KDECORE_LIBRARY} PATH )

IF(NOT KDE3_LIBTOOL_DIR)
   IF(KDE3_KDECORE_LIBRARY MATCHES lib64)
     SET(KDE3_LIBTOOL_DIR /lib64/kde3)
   ELSE(KDE3_KDECORE_LIBRARY MATCHES lib64)
     SET(KDE3_LIBTOOL_DIR /lib/kde3)
   ENDIF(KDE3_KDECORE_LIBRARY MATCHES lib64)
ENDIF(NOT KDE3_LIBTOOL_DIR)

#now search for the dcop utilities
FIND_PROGRAM(KDE3_DCOPIDL_EXECUTABLE NAMES dcopidl PATHS
  $ENV{KDEDIR}/bin
  ${KDE3PREFIX}/bin
  /opt/kde3/bin
  /opt/kde/bin
  NO_DEFAULT_PATH
  )

FIND_PROGRAM(KDE3_DCOPIDL_EXECUTABLE NAMES dcopidl)

FIND_PROGRAM(KDE3_DCOPIDL2CPP_EXECUTABLE NAMES dcopidl2cpp PATHS
  $ENV{KDEDIR}/bin
  ${KDE3PREFIX}/bin
  /opt/kde3/bin
  /opt/kde/bin
  NO_DEFAULT_PATH
  )

FIND_PROGRAM(KDE3_DCOPIDL2CPP_EXECUTABLE NAMES dcopidl2cpp)

FIND_PROGRAM(KDE3_KCFGC_EXECUTABLE NAMES kconfig_compiler PATHS
  $ENV{KDEDIR}/bin
  ${KDE3PREFIX}/bin
  /opt/kde3/bin
  /opt/kde/bin
  NO_DEFAULT_PATH
  )

FIND_PROGRAM(KDE3_KCFGC_EXECUTABLE NAMES kconfig_compiler)

# KDE3Macros.cmake contains all the KDE specific macros
INCLUDE(KDE3Macros)


#SET KDE3_FOUND
IF (KDE3_INCLUDE_DIR AND KDE3_LIB_DIR AND KDE3_DCOPIDL_EXECUTABLE AND KDE3_DCOPIDL2CPP_EXECUTABLE AND KDE3_KCFGC_EXECUTABLE)
   SET(KDE3_FOUND TRUE)
ELSE (KDE3_INCLUDE_DIR AND KDE3_LIB_DIR AND KDE3_DCOPIDL_EXECUTABLE AND KDE3_DCOPIDL2CPP_EXECUTABLE AND KDE3_KCFGC_EXECUTABLE)
   SET(KDE3_FOUND FALSE)
ENDIF (KDE3_INCLUDE_DIR AND KDE3_LIB_DIR AND KDE3_DCOPIDL_EXECUTABLE AND KDE3_DCOPIDL2CPP_EXECUTABLE AND KDE3_KCFGC_EXECUTABLE)


MACRO (KDE3_PRINT_RESULTS)
   IF(KDE3_INCLUDE_DIR)
      MESSAGE(STATUS "Found KDE3 include dir: ${KDE3_INCLUDE_DIR}")
   ELSE(KDE3_INCLUDE_DIR)
      MESSAGE(STATUS "Didn't find KDE3 headers")
   ENDIF(KDE3_INCLUDE_DIR)

   IF(KDE3_LIB_DIR)
      MESSAGE(STATUS "Found KDE3 library dir: ${KDE3_LIB_DIR}")
   ELSE(KDE3_LIB_DIR)
      MESSAGE(STATUS "Didn't find KDE3 core library")
   ENDIF(KDE3_LIB_DIR)

   IF(KDE3_DCOPIDL_EXECUTABLE)
      MESSAGE(STATUS "Found KDE3 dcopidl preprocessor: ${KDE3_DCOPIDL_EXECUTABLE}")
   ELSE(KDE3_DCOPIDL_EXECUTABLE)
      MESSAGE(STATUS "Didn't find the KDE3 dcopidl preprocessor")
   ENDIF(KDE3_DCOPIDL_EXECUTABLE)

   IF(KDE3_DCOPIDL2CPP_EXECUTABLE)
      MESSAGE(STATUS "Found KDE3 dcopidl2cpp preprocessor: ${KDE3_DCOPIDL2CPP_EXECUTABLE}")
   ELSE(KDE3_DCOPIDL2CPP_EXECUTABLE)
      MESSAGE(STATUS "Didn't find the KDE3 dcopidl2cpp preprocessor")
   ENDIF(KDE3_DCOPIDL2CPP_EXECUTABLE)

   IF(KDE3_KCFGC_EXECUTABLE)
      MESSAGE(STATUS "Found KDE3 kconfig_compiler preprocessor: ${KDE3_KCFGC_EXECUTABLE}")
   ELSE(KDE3_KCFGC_EXECUTABLE)
      MESSAGE(STATUS "Didn't find the KDE3 kconfig_compiler preprocessor")
   ENDIF(KDE3_KCFGC_EXECUTABLE)

ENDMACRO (KDE3_PRINT_RESULTS)


IF (KDE3_FIND_REQUIRED AND NOT KDE3_FOUND)
   #bail out if something wasn't found
   KDE3_PRINT_RESULTS()
   MESSAGE(FATAL_ERROR "Could NOT find everything required for compiling KDE 3 programs")

ENDIF (KDE3_FIND_REQUIRED AND NOT KDE3_FOUND)


IF (NOT KDE3_FIND_QUIETLY)
   KDE3_PRINT_RESULTS()
ENDIF (NOT KDE3_FIND_QUIETLY)

#add the found Qt and KDE include directories to the current include path
SET(KDE3_INCLUDE_DIRS ${QT_INCLUDE_DIR} ${KDE3_INCLUDE_DIR})

