# searches for the installed Qt-Version and desides which FindQtX.cmake to include
#
# This module uses
# QT_MAX_VERSION                ,maximum Qt version set by YOU (e.g. "3.9.9")
#                                       if it is not set, we assume (for compatibility) the project is old and uses Qt < 4
#                                       Qt > 3 projects have to (!) set QT_MAX_VERSION (maximum by Trolltech is "15.7.99")
# QT_MIN_VERSION                ,minimum Qt version set by YOU (e.g. "2.2.3")
#
# This modules sets
# QT_INST_MAJOR_VERSION         ,major number of the installed Qt (e.g. Qt-Version = 4.2.6 => 4)
# QT_INST_MINOR_VERSION         ,minor number of the installed Qt (e.g. Qt-Version = 4.2.6 => 2)
# QT_INST_PATCH_VERSION         ,patch number of the installed Qt (e.g. Qt-Version = 4.2.6 => 6)
#
# QT_FOUND                      ,gets set to YES or NO depending on QT_MAX_VERSION and QT_MIN_VERSION

##########################################
#
#     init some internal variables 
#
##########################################
IF(NOT QT_FOUND)
  SET(QT_FOUND "NO")
ENDIF(NOT QT_FOUND)
# if a project needs Qt it has to call FoundQt.cmake and NOT FoundQt3.cmake, FoundQt4.cmake etc. This gets checked in FoundQt4.cmake etc.
SET(FOUNDQT_CALLED "YES")

# compatibility to CMakeList.txt files for Qt3 projects
IF(NOT QT_MAX_VERSION)
  SET(QT_MAX_VERSION "3.9.9")
ENDIF(NOT QT_MAX_VERSION)

IF(NOT QT_MIN_VERSION)
  SET(QT_MIN_VERSION "0.0.1")
ENDIF(NOT QT_MIN_VERSION)

#######################################
#
#       search for qglobal.h and sets
#       QT_GLOBAL_H_FILE
#
#######################################
IF( NOT QT_QGLOBAL_H_FILE)
  EXEC_PROGRAM( qmake
    ARGS "-query QT_INSTALL_HEADERS" 
    OUTPUT_VARIABLE qt_headers )
  
  # Qt4-like search paths
  FIND_FILE( QT4_QGLOBAL_H_FILE qglobal.h
    ${qt_headers}/Qt
    $ENV{QTDIR}/include/Qt
    /usr/local/qt/include/Qt
    /usr/local/include/Qt
    /usr/lib/qt/include/Qt
    /usr/include/Qt
    /usr/share/qt4/include/Qt
    C:/Progra~1/qt/include/Qt )
  
  IF(QT4_QGLOBAL_H_FILE)
    STRING(REGEX REPLACE "/include/Qt/qglobal.h" "" QT_DIR "${QT4_QGLOBAL_H_FILE}")
    SET(QT_QGLOBAL_H_FILE ${QT4_QGLOBAL_H_FILE})
  ELSE(QT4_QGLOBAL_H_FILE)
    #Qt3-like search paths
    FIND_FILE( QT3_QGLOBAL_H_FILE qglobal.h
      $ENV{QTDIR}/include
      /usr/include/qt3/Qt
      /usr/local/qt/include
      /usr/local/include
      /usr/lib/qt/include
      /usr/include
      /usr/share/qt3/include
      C:/Progra~1/qt/include
      /usr/include/qt3 )
    IF(QT3_QGLOBAL_H_FILE)
      STRING(REGEX REPLACE "/include/qglobal.h" "" QT_DIR "${QT3_QGLOBAL_H_FILE}")
      SET(QT_QGLOBAL_H_FILE ${QT3_QGLOBAL_H_FILE})
    ENDIF(QT3_QGLOBAL_H_FILE)
  ENDIF(QT4_QGLOBAL_H_FILE)
  
  #check if qglobal.h is just a file with includes qglobal.h from elsewhere
  IF(QT_QGLOBAL_H_FILE)
    FILE(READ "${QT_QGLOBAL_H_FILE}" QT_QGLOBAL_H)      
    #BUG!!! the regexpression is not correct. 
    #If "#include" and "qglobal.h" are in different lines it would be seen as ok
    STRING(REGEX MATCH "#include.*qglobal.h" QT_QGLOBAL_H_LINK "${QT_QGLOBAL_H}")
    
    IF( QT_QGLOBAL_H_LINK )
      STRING(REGEX REPLACE ".*\"" "" QT_QGLOBAL_H_LINK "${QT_QGLOBAL_H_LINK}")
      STRING(REGEX REPLACE "\\.\\./\\.\\." "${QT_DIR}" QT_QGLOBAL_H_FILE "${QT_QGLOBAL_H_LINK}")
    ENDIF(QT_QGLOBAL_H_LINK)
  ENDIF(QT_QGLOBAL_H_FILE)
ENDIF(NOT QT_QGLOBAL_H_FILE)

###############################################################
#
#       read the version number of the installed of Qt and sets
#       QT_INST_MAJOR_VERSION
#       QT_INST_MINOR_VERSION
#       QT_INST_PATCH_VERSION
#
###############################################################
IF( QT_QGLOBAL_H_FILE)
  IF(NOT QT_INST_MAJOR_VERSION OR NOT QT_INST_MINOR_VERSION OR NOT QT_INST_PATCH_VERSION)
    #extract the version string from qglobal.h
    FILE(READ ${QT_QGLOBAL_H_FILE} QT_QGLOBAL_H)
    
    # BUG !!! ... THE NEXT LINE DOES NOT WORK FOR Qt10 (even though that´s in far future) 
    # Trolltech defines in src/corelib/global/qglobal.h: QT_VERSION is (major << 16) + (minor << 8) + patch.
    
    STRING(REGEX MATCH "#define[\t ]+QT_VERSION_STR[\t ]+\"([0-9]+.[0-9]+.[0-9]+)" QT_QGLOBAL_H "${QT_QGLOBAL_H}")
    STRING(REGEX REPLACE ".*\"([0-9]+.[0-9]+.[0-9]+).*" "\\1" QT_QGLOBAL_H "${QT_QGLOBAL_H}")
    
    # Under windows the qt library (MSVC) has the format qt-mtXYZ where XYZ is the
    # version X.Y.Z, so we need to remove the dots from version (at least in Qt3)
    STRING(REGEX REPLACE "\\." "" qt_version_str "${QT_QGLOBAL_H}")
    
    IF(NOT QT_INST_MAJOR_VERSION)
      STRING(REGEX REPLACE "([0-9]+)[0-9]+[0-9]+" "\\1" QT_INST_MAJOR_VERSION "${qt_version_str}")
    ENDIF(NOT QT_INST_MAJOR_VERSION)
    IF(NOT QT_INST_MINOR_VERSION)
      STRING(REGEX REPLACE "[0-9]+([0-9]+)[0-9]+" "\\1" QT_INST_MINOR_VERSION "${qt_version_str}")
    ENDIF(NOT QT_INST_MINOR_VERSION)
    IF(NOT QT_INST_PATCH_VERSION)
      STRING(REGEX REPLACE "[0-9]+[0-9]+([0-9]+)" "\\1" QT_INST_PATCH_VERSION "${qt_version_str}")
    ENDIF(NOT QT_INST_PATCH_VERSION)
  ENDIF(NOT QT_INST_MAJOR_VERSION OR NOT QT_INST_MINOR_VERSION OR NOT QT_INST_PATCH_VERSION)
  # set qt_version_str back to x.y.z to make message outputs readable
  SET(qt_version_str "${QT_INST_MAJOR_VERSION}.${QT_INST_MINOR_VERSION}.${QT_INST_PATCH_VERSION}")
ELSE( QT_QGLOBAL_H_FILE )
  IF( NOT Qt_FIND_QUIETLY AND Qt_FIND_REQUIRED) 
    MESSAGE( SEND_ERROR "Couldn´t find qglobal.h => Qt not installed.")
  ENDIF( NOT Qt_FIND_QUIETLY AND Qt_FIND_REQUIRED) 
  SET( QT_FOUND "NO" )
ENDIF( QT_QGLOBAL_H_FILE )

IF( NOT Qt_FIND_QUIETLY) 
  MESSAGE(STATUS "Checking Qt-Version ${QT_INST_MAJOR_VERSION}.${QT_INST_MINOR_VERSION}.${QT_INST_PATCH_VERSION}")
ENDIF( NOT Qt_FIND_QUIETLY) 

#############################################################
#
#       check the minimum requirments set in QT_MIN_VERSION
#
#############################################################
IF (QT_MIN_VERSION)
  #now parse the parts of the user given version string into variables 
  STRING(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+$" min_qt_major_vers "${QT_MIN_VERSION}")
  IF (NOT min_qt_major_vers)
    IF( NOT Qt_FIND_QUIETLY AND Qt_FIND_REQUIRED) 
      MESSAGE( SEND_ERROR "Invalid Qt version string given: \"${QT_MIN_VERSION}\", expected e.g. \"3.1.5\"")
    ENDIF( NOT Qt_FIND_QUIETLY AND Qt_FIND_REQUIRED) 
    SET( QT_FOUND "NO" )
  ELSE(NOT min_qt_major_vers)
    STRING(REGEX REPLACE "([0-9]+)\\.[0-9]+\\.[0-9]+" "\\1" min_qt_major_vers "${QT_MIN_VERSION}")
    STRING(REGEX REPLACE "[0-9]+\\.([0-9])+\\.[0-9]+" "\\1" min_qt_minor_vers "${QT_MIN_VERSION}")
    STRING(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1" min_qt_patch_vers "${QT_MIN_VERSION}")
    # min = "6.5.4", qt = "3.2.1"
    
    # check major number
    IF (min_qt_major_vers GREATER QT_INST_MAJOR_VERSION)                  # (6 > 3) ?
      IF( NOT Qt_FIND_QUIETLY AND Qt_FIND_REQUIRED) 
        MESSAGE(  SEND_ERROR "Qt major version too small (minimum: ${QT_MIN_VERSION}, found: ${qt_version_str})")            # yes
      ENDIF( NOT Qt_FIND_QUIETLY AND Qt_FIND_REQUIRED) 
      SET( QT_FOUND "NO" )
    ELSE  (min_qt_major_vers GREATER QT_INST_MAJOR_VERSION)               # no
      IF (min_qt_major_vers LESS QT_INST_MAJOR_VERSION)                  # (6 < 3) ?
        SET( QT_VERSION_BIG_ENOUGH "YES" )                      # yes
      ELSE (min_qt_major_vers LESS QT_INST_MAJOR_VERSION)                # ( 6==3) ?
        
        # check minor number
        IF (min_qt_minor_vers GREATER QT_INST_MINOR_VERSION)            # (5>2) ?
          IF( NOT Qt_FIND_QUIETLY AND Qt_FIND_REQUIRED) 
            MESSAGE(  SEND_ERROR "Qt minor version too small (minimum: ${QT_MIN_VERSION}, found: ${qt_version_str})")      # yes
          ENDIF( NOT Qt_FIND_QUIETLY AND Qt_FIND_REQUIRED) 
          SET( QT_FOUND "NO" )
        ELSE (min_qt_minor_vers GREATER QT_INST_MINOR_VERSION)          # no
          IF (min_qt_minor_vers LESS QT_INST_MINOR_VERSION)            # (5<2) ?
            SET( QT_VERSION_BIG_ENOUGH "YES" )                # yes
          ELSE (min_qt_minor_vers LESS QT_INST_MINOR_VERSION)          # (5==2)
            
            # check patch number
            IF (min_qt_patch_vers GREATER QT_INST_PATCH_VERSION)      # (4>1) ?
              IF( NOT Qt_FIND_QUIETLY AND Qt_FIND_REQUIRED) 
                MESSAGE( SEND_ERROR "Qt patch level too small (minimum: ${QT_MIN_VERSION}, found: ${qt_version_str})")  # yes
              ENDIF( NOT Qt_FIND_QUIETLY AND Qt_FIND_REQUIRED) 
              SET( QT_FOUND "NO" )
            ELSE (min_qt_patch_vers GREATER QT_INST_PATCH_VERSION)    # (4>1) ?
              SET( QT_VERSION_BIG_ENOUGH "YES" )             # yes
            ENDIF (min_qt_patch_vers GREATER QT_INST_PATCH_VERSION)   # (4>1) ?
          ENDIF (min_qt_minor_vers LESS QT_INST_MINOR_VERSION)
          
        ENDIF (min_qt_minor_vers GREATER QT_INST_MINOR_VERSION)
      ENDIF (min_qt_major_vers LESS QT_INST_MAJOR_VERSION)
    ENDIF (min_qt_major_vers GREATER QT_INST_MAJOR_VERSION)
  ENDIF (NOT min_qt_major_vers)

ENDIF (QT_MIN_VERSION)


#############################################################
#
#       check the maximum requirments set in QT_MAX_VERSION
#
#############################################################
IF(QT_MAX_VERSION) 
  #now parse the parts of the user given version string into variables 
  STRING(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+$" max_qt_major_vers "${QT_MAX_VERSION}")
  IF (NOT max_qt_major_vers)
    IF( NOT Qt_FIND_QUIETLY AND Qt_FIND_REQUIRED) 
      MESSAGE( SEND_ERROR "Invalid Qt version string given: \"${QT_MAX_VERSION}\", expected e.g. \"4.1.5\"")
    ENDIF( NOT Qt_FIND_QUIETLY AND Qt_FIND_REQUIRED) 
    SET( QT_FOUND "NO" )
  ELSE(NOT max_qt_major_vers)
    
    STRING(REGEX REPLACE "([0-9]+)\\.[0-9]+\\.[0-9]+" "\\1" max_qt_major_vers "${QT_MAX_VERSION}")
    STRING(REGEX REPLACE "[0-9]+\\.([0-9])+\\.[0-9]+" "\\1" max_qt_minor_vers "${QT_MAX_VERSION}")
    STRING(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1" max_qt_patch_vers "${QT_MAX_VERSION}")
    # max = "6.5.4", qt = "7.2.1"
    
    # check major number
    IF (max_qt_major_vers LESS QT_INST_MAJOR_VERSION)                  # (6 < 3) ?
      IF( NOT Qt_FIND_QUIETLY AND Qt_FIND_REQUIRED) 
        MESSAGE(  SEND_ERROR "Qt major version too big (maximum: ${QT_MAX_VERSION}, found: ${qt_version_str})") # yes
      ENDIF( NOT Qt_FIND_QUIETLY AND Qt_FIND_REQUIRED) 
      SET( QT_FOUND "NO" )
    ELSE  (max_qt_major_vers LESS QT_INST_MAJOR_VERSION)               # no
      IF (max_qt_major_vers GREATER QT_INST_MAJOR_VERSION)                  # (6 < 3) ?
        SET( QT_VERSION_SMALL_ENOUGH "YES" )                      # yes
      ELSE (max_qt_major_vers GREATER QT_INST_MAJOR_VERSION)                # ( 6==3) ?
        
        # check minor number
        IF (max_qt_minor_vers LESS QT_INST_MINOR_VERSION)            # (5 < 2) ?
          IF( NOT Qt_FIND_QUIETLY AND Qt_FIND_REQUIRED) 
            MESSAGE(  SEND_ERROR "Qt minor version too big (maximum: ${QT_MAX_VERSION}, found: ${qt_version_str})") # yes
          ENDIF( NOT Qt_FIND_QUIETLY AND Qt_FIND_REQUIRED) 
          SET( QT_FOUND "NO" )
        ELSE (max_qt_minor_vers LESS QT_INST_MINOR_VERSION)          # no
          IF (max_qt_minor_vers GREATER QT_INST_MINOR_VERSION)            # (5 < 2) ?
            SET( QT_VERSION_SMALL_ENOUGH "YES" )                # yes
          ELSE (max_qt_minor_vers GREATER QT_INST_MINOR_VERSION)          # (5==2)
            
            # check patch number
            IF (max_qt_patch_vers LESS QT_INST_PATCH_VERSION)      # (4 < 1) ?
              IF( NOT Qt_FIND_QUIETLY AND Qt_FIND_REQUIRED) 
                MESSAGE( SEND_ERROR "Qt patch level too big (maximum: ${QT_MAX_VERSION}, found: ${qt_version_str})")  # yes
              ENDIF( NOT Qt_FIND_QUIETLY AND Qt_FIND_REQUIRED) 
              SET( QT_FOUND "NO" )
            ELSE (max_qt_patch_vers LESS QT_INST_PATCH_VERSION)    # (4 < 1) ?
              SET( QT_VERSION_SMALL_ENOUGH "YES" )             # yes
            ENDIF (max_qt_patch_vers LESS QT_INST_PATCH_VERSION)   # (4 > 1) ?
            
          ENDIF (max_qt_minor_vers GREATER QT_INST_MINOR_VERSION)
        ENDIF (max_qt_minor_vers LESS QT_INST_MINOR_VERSION)
      ENDIF (max_qt_major_vers GREATER QT_INST_MAJOR_VERSION)
    ENDIF (max_qt_major_vers LESS QT_INST_MAJOR_VERSION)
  ENDIF (NOT max_qt_major_vers)
ENDIF(QT_MAX_VERSION)


###############################################################
#
#       include the correct FindQtX.cmake file if Qt was found
#       (we only distinguage between major versions
#
###############################################################
IF(QT_VERSION_SMALL_ENOUGH AND QT_VERSION_BIG_ENOUGH)
  # Call package for Qt3
  IF( QT_INST_MAJOR_VERSION LESS 4)
    FIND_PACKAGE(Qt3)
  ENDIF( QT_INST_MAJOR_VERSION LESS 4)
  # Call package for Qt4
  IF( QT_INST_MAJOR_VERSION GREATER 3 )
    IF( QT_INST_MAJOR_VERSION LESS 5)
      FIND_PACKAGE(Qt4)
    ELSE( QT_INST_MAJOR_VERSION LESS 5)
      IF( NOT Qt_FIND_QUIETLY AND Qt_FIND_REQUIRED) 
        MESSAGE(SEND_ERROR "Right now CMake supports only Qt-Versions less than 5.0.0")
      ENDIF( NOT Qt_FIND_QUIETLY AND Qt_FIND_REQUIRED) 
      SET(QT_FOUND "NO")
    ENDIF( QT_INST_MAJOR_VERSION LESS 5)
  ENDIF( QT_INST_MAJOR_VERSION GREATER 3 )
ENDIF(QT_VERSION_SMALL_ENOUGH AND QT_VERSION_BIG_ENOUGH)





