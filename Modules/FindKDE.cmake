# Locate KDE include paths and libraries

# KDE_INCLUDE_DIR
# KDE_LIB_DIR
# KDE_KDE_CONFIG
# KDE_DCOP_IDL
# KDE_VERSION

#the macro ADD_DCOP_SOURCES(src_headers dest_srcs)
#
#usage:
#SET(MY_APP_SOURCES main.cpp kfoo.cpp)
#SET(MY_DCOP_SOURCES kfooiface.h)
#ADD_DCOP_SOURCES(MY_DCOP_SOURCES MY_APP_SOURCES)
#and then it should work :-)

#uses KDE_MIN_VERSION

CMAKE_MINIMUM_REQUIRED(VERSION 2.0.0) 

FIND_PATH(KDE_INCLUDE_DIR kurl.h
  $ENV{KDEDIR}/include
  /opt/kde/include
  /opt/kde3/include
  /usr/local/include
  /usr/include/
  /usr/include/kde
  /usr/local/include/kde
  )

IF(KDE_INCLUDE_DIR)
   MESSAGE(STATUS "Found KDE header files in ${KDE_INCLUDE_DIR}")
ELSE(KDE_INCLUDE_DIR)
   MESSAGE(FATAL_ERROR "Didn't find KDE headers")
ENDIF(KDE_INCLUDE_DIR)

FIND_PROGRAM(KDE_CONFIG NAME kde-config PATHS 
  $ENV{KDEDIR}/bin
  /opt/kde/bin
  /opt/kde3/bin)

IF(NOT KDE_CONFIG)
   MESSAGE(FATAL_ERROR "Didn't find the kde-config utility")
ENDIF(NOT KDE_CONFIG)

FIND_PATH(KDE_LIB_DIR libkdecore.so
  $ENV{KDEDIR}/lib
  /opt/kde/lib
  /opt/kde3/lib
  )

IF(KDE_LIB_DIR)
   MESSAGE(STATUS "Found KDE libraries in ${KDE_LIB_DIR}")
ELSE(KDE_LIB_DIR)
   MESSAGE(FATAL_ERROR "Didn't find KDE core library")
ENDIF(KDE_LIB_DIR)

IF (KDE_MIN_VERSION)
   #extract the version from kdeversion.h

   FILE(READ ${KDE_INCLUDE_DIR}/kdeversion.h KDE_VERSION_H)

   STRING(REGEX REPLACE ".*#define[\\t\\ ]+KDE_VERSION_STRING[\\t\\ ]+\"([0-9]+\\.[0-9]+\\.[0-9]+)\".*" "\\1" KDE_VERSION "${KDE_VERSION_H}")
   STRING(REGEX REPLACE ".*#define[\\t\\ ]+KDE_VERSION_MAJOR[\\t\\ ]+([0-9]+).*" "\\1" kde_major_vers "${KDE_VERSION_H}")
   STRING(REGEX REPLACE ".*#define[\\t\\ ]+KDE_VERSION_MINOR[\\t\\ ]+([0-9]+).*" "\\1" kde_minor_vers "${KDE_VERSION_H}")
   STRING(REGEX REPLACE ".*#define[\\t\\ ]+KDE_VERSION_RELEASE[\\t\\ ]+([0-9]+).*" "\\1" kde_vers_release "${KDE_VERSION_H}")

#now parse the parts of the user given version string into variables 
   STRING(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+$" req_kde_major_vers "${KDE_MIN_VERSION}")
   IF (NOT req_kde_major_vers)
      MESSAGE( FATAL_ERROR "Invalid KDE version string given: \"${KDE_MIN_VERSION}\", expected e.g. \"3.1.5\"")
   ENDIF (NOT req_kde_major_vers)
      
   STRING(REGEX REPLACE "([0-9]+)\\.[0-9]+\\.[0-9]+" "\\1" req_kde_major_vers "${KDE_MIN_VERSION}")
   STRING(REGEX REPLACE "[0-9]+\\.([0-9])+\\.[0-9]+" "\\1" req_kde_minor_vers "${KDE_MIN_VERSION}")
   STRING(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1" req_kde_patch_vers "${KDE_MIN_VERSION}")

# req = "6.5.4", qt = "3.2.1"

   IF (req_kde_major_vers GREATER kde_major_vers)                  # (6 > 3) ?
      MESSAGE(  FATAL_ERROR "KDE major version not matched (required: ${KDE_MIN_VERSION}, found: ${KDE_VERSION})")            # yes
   ELSE  (req_kde_major_vers GREATER kde_major_vers)               # no
      IF (req_kde_major_vers LESS kde_major_vers)                  # (6 < 3) ?
         SET( QT_VERSION_BIG_ENOUGH "YES" )                        # yes
      ELSE (req_kde_major_vers LESS kde_major_vers)                # ( 6==3) ?
         IF (req_kde_minor_vers GREATER kde_minor_vers)            # (5>2) ?
            MESSAGE(  FATAL_ERROR "KDE minor version not matched (required: ${KDE_MIN_VERSION}, found: ${KDE_VERSION})")      # yes
         ELSE (req_kde_minor_vers GREATER kde_minor_vers)          # no
            IF (req_kde_minor_vers LESS kde_minor_vers)            # (5<2) ?
               SET( QT_VERSION_BIG_ENOUGH "YES" )                  # yes
            ELSE (req_kde_minor_vers LESS kde_minor_vers)          # (5==2)
               IF (req_kde_patch_vers GREATER kde_vers_release)    # (4>1) ?
                  MESSAGE( FATAL_ERROR "KDE release version not matched (required: ${KDE_MIN_VERSION}, found: ${KDE_VERSION})")  # yes
               ELSE (req_kde_patch_vers GREATER kde_patch_vers)    # (4>1) ?
                  SET( QT_VERSION_BIG_ENOUGH "YES" )               # yes
               ENDIF (req_kde_patch_vers GREATER kde_vers_release) # (4>1) ?
            ENDIF (req_kde_minor_vers LESS kde_minor_vers)
         ENDIF (req_kde_minor_vers GREATER kde_minor_vers)
      ENDIF (req_kde_major_vers LESS kde_major_vers)
   ENDIF (req_kde_major_vers GREATER kde_major_vers)
   MESSAGE(STATUS "KDE version ok (required: ${KDE_MIN_VERSION}, found: ${KDE_VERSION})")
ENDIF (KDE_MIN_VERSION)

#now search for the dcop utilities
FIND_PROGRAM(KDE_DCOPIDL NAME dcopidl PATHS 
  $ENV{KDEDIR}/bin
  /opt/kde/bin
  /opt/kde3/bin)

IF(NOT KDE_DCOPIDL)
   MESSAGE(FATAL_ERROR "Didn't find the dcopidl preprocessor")
ENDIF(NOT KDE_DCOPIDL)

FIND_PROGRAM(KDE_DCOPIDL2CPP NAME dcopidl2cpp PATHS 
  $ENV{KDEDIR}/bin
  /opt/kde/bin
  /opt/kde3/bin)

IF(NOT KDE_DCOPIDL2CPP)
   MESSAGE(FATAL_ERROR "Didn't find the dcopidl2cpp preprocessor")
ENDIF(NOT KDE_DCOPIDL2CPP)

MACRO(DCOP_WRAP SKEL_SOURCES)
   FOREACH(dcop_source ${ARGN})

       GET_FILENAME_COMPONENT(dcop_source_basename ${dcop_source} NAME_WE)

       SET(idl_file ${CMAKE_BINARY_DIR}/${dcop_source_basename}.kidl)
       SET(skel_file ${CMAKE_BINARY_DIR}/${dcop_source_basename}_skel.cpp)

       ADD_CUSTOM_COMMAND(
         OUTPUT ${idl_file}
         COMMAND ${KDE_DCOPIDL}
         ARGS  ${CMAKE_SOURCE_DIR}/${dcop_source} > ${idl_file}
         DEPENDS  ${CMAKE_SOURCE_DIR}/${dcop_source})

      ADD_CUSTOM_COMMAND(
        OUTPUT ${skel_file}
        COMMAND ${KDE_DCOPIDL2CPP}  
        ARGS --c++-suffix cpp --no-signals --no-stub ${idl_file}
        DEPENDS ${idl_file})

      SET( SKEL_SOURCES ${${SKEL_SOURCES}}
        ${skel_file})
   ENDFOREACH(dcop_source)
ENDMACRO (DCOP_WRAP)

MARK_AS_ADVANCED(
  KDE_INCLUDE_DIR
  KDE_LIB_DIR
  ) 
