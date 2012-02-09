# - Find QT 4
# This module can be used to find Qt4.
# The most important issue is that the Qt4 qmake is available via the system path.
# This qmake is then used to detect basically everything else.
# This module defines a number of key variables and macros. 
# The variable QT_USE_FILE is set which is the path to a CMake file that can be included 
# to compile Qt 4 applications and libraries.  It sets up the compilation
# environment for include directories, preprocessor defines and populates a
# QT_LIBRARIES variable.
#
# Typical usage could be something like:
#   find_package(Qt4 4.4.3 REQUIRED QtCore QtGui QtXml)
#   include(${QT_USE_FILE})
#   add_executable(myexe main.cpp)
#   target_link_libraries(myexe ${QT_LIBRARIES})
#
# The minimum required version can be specified using the standard find_package()-syntax
# (see example above). 
# For compatibility with older versions of FindQt4.cmake it is also possible to
# set the variable QT_MIN_VERSION to the minimum required version of Qt4 before the 
# find_package(Qt4) command. 
# If both are used, the version used in the find_package() command overrides the
# one from QT_MIN_VERSION.
#
# When using the components argument, QT_USE_QT* variables are automatically set
# for the QT_USE_FILE to pick up.  If one wishes to manually set them, the
# available ones to set include:
#                    QT_DONT_USE_QTCORE
#                    QT_DONT_USE_QTGUI
#                    QT_USE_QT3SUPPORT
#                    QT_USE_QTASSISTANT
#                    QT_USE_QAXCONTAINER
#                    QT_USE_QAXSERVER
#                    QT_USE_QTDESIGNER
#                    QT_USE_QTMOTIF
#                    QT_USE_QTMAIN
#                    QT_USE_QTMULTIMEDIA
#                    QT_USE_QTNETWORK
#                    QT_USE_QTNSPLUGIN
#                    QT_USE_QTOPENGL
#                    QT_USE_QTSQL
#                    QT_USE_QTXML
#                    QT_USE_QTSVG
#                    QT_USE_QTTEST
#                    QT_USE_QTUITOOLS
#                    QT_USE_QTDBUS
#                    QT_USE_QTSCRIPT
#                    QT_USE_QTASSISTANTCLIENT
#                    QT_USE_QTHELP
#                    QT_USE_QTWEBKIT
#                    QT_USE_QTXMLPATTERNS
#                    QT_USE_PHONON
#                    QT_USE_QTSCRIPTTOOLS
#                    QT_USE_QTDECLARATIVE
#
#  QT_USE_IMPORTED_TARGETS 
#        If this variable is set to TRUE, FindQt4.cmake will create imported
#        library targets for the various Qt libraries and set the 
#        library variables like QT_QTCORE_LIBRARY to point at these imported
#        targets instead of the library file on disk. This provides much better 
#        handling of the release and debug versions of the Qt libraries and is 
#       also always backwards compatible, except for the case that dependencies
#       of libraries are exported, these will then also list the names of the 
#       imported targets as dependency and not the file location on disk. This
#       is much more flexible, but requires that FindQt4.cmake is executed before
#       such an exported dependency file is processed.
#
# There are also some files that need processing by some Qt tools such as moc
# and uic.  Listed below are macros that may be used to process those files.
#  
#  macro QT4_WRAP_CPP(outfiles inputfile ... OPTIONS ...)
#        create moc code from a list of files containing Qt class with
#        the Q_OBJECT declaration.  Per-direcotry preprocessor definitions 
#        are also added.  Options may be given to moc, such as those found
#        when executing "moc -help".  
#
#  macro QT4_WRAP_UI(outfiles inputfile ... OPTIONS ...)
#        create code from a list of Qt designer ui files.
#        Options may be given to uic, such as those found
#        when executing "uic -help"
#
#  macro QT4_ADD_RESOURCES(outfiles inputfile ... OPTIONS ...)
#        create code from a list of Qt resource files.
#        Options may be given to rcc, such as those found
#        when executing "rcc -help"
#
#  macro QT4_GENERATE_MOC(inputfile outputfile )
#        creates a rule to run moc on infile and create outfile.
#        Use this if for some reason QT4_WRAP_CPP() isn't appropriate, e.g.
#        because you need a custom filename for the moc file or something similar.
#
#  macro QT4_AUTOMOC(sourcefile1 sourcefile2 ... )
#        This macro is still experimental.
#        It can be used to have moc automatically handled.
#        So if you have the files foo.h and foo.cpp, and in foo.h a 
#        a class uses the Q_OBJECT macro, moc has to run on it. If you don't
#        want to use QT4_WRAP_CPP() (which is reliable and mature), you can insert
#        #include "foo.moc"
#        in foo.cpp and then give foo.cpp as argument to QT4_AUTOMOC(). This will the
#        scan all listed files at cmake-time for such included moc files and if it finds
#        them cause a rule to be generated to run moc at build time on the 
#        accompanying header file foo.h.
#        If a source file has the SKIP_AUTOMOC property set it will be ignored by this macro.
#
#  macro QT4_ADD_DBUS_INTERFACE(outfiles interface basename)
#        create a the interface header and implementation files with the 
#        given basename from the given interface xml file and add it to 
#        the list of sources
#
#  macro QT4_ADD_DBUS_INTERFACES(outfiles inputfile ... )
#        create the interface header and implementation files 
#        for all listed interface xml files
#        the name will be automatically determined from the name of the xml file
#
#  macro QT4_ADD_DBUS_ADAPTOR(outfiles xmlfile parentheader parentclassname [basename] [classname])
#        create a dbus adaptor (header and implementation file) from the xml file
#        describing the interface, and add it to the list of sources. The adaptor
#        forwards the calls to a parent class, defined in parentheader and named
#        parentclassname. The name of the generated files will be
#        <basename>adaptor.{cpp,h} where basename defaults to the basename of the xml file.
#        If <classname> is provided, then it will be used as the classname of the
#        adaptor itself.
#
#  macro QT4_GENERATE_DBUS_INTERFACE( header [interfacename] OPTIONS ...)
#        generate the xml interface file from the given header.
#        If the optional argument interfacename is omitted, the name of the 
#        interface file is constructed from the basename of the header with
#        the suffix .xml appended.
#        Options may be given to qdbuscpp2xml, such as those found when executing "qdbuscpp2xml --help"
#
#  macro QT4_CREATE_TRANSLATION( qm_files directories ... sources ... 
#                                ts_files ... OPTIONS ...)
#        out: qm_files
#        in:  directories sources ts_files
#        options: flags to pass to lupdate, such as -extensions to specify
#        extensions for a directory scan.
#        generates commands to create .ts (vie lupdate) and .qm
#        (via lrelease) - files from directories and/or sources. The ts files are 
#        created and/or updated in the source tree (unless given with full paths).
#        The qm files are generated in the build tree.
#        Updating the translations can be done by adding the qm_files
#        to the source list of your library/executable, so they are
#        always updated, or by adding a custom target to control when
#        they get updated/generated.
#
#  macro QT4_ADD_TRANSLATION( qm_files ts_files ... )
#        out: qm_files
#        in:  ts_files
#        generates commands to create .qm from .ts - files. The generated
#        filenames can be found in qm_files. The ts_files
#        must exists and are not updated in any way.
#
#
#  Below is a detailed list of variables that FindQt4.cmake sets.
#  QT_FOUND         If false, don't try to use Qt.
#  QT4_FOUND        If false, don't try to use Qt 4.
#
#  QT_VERSION_MAJOR The major version of Qt found.
#  QT_VERSION_MINOR The minor version of Qt found.
#  QT_VERSION_PATCH The patch version of Qt found.
#
#  QT_EDITION               Set to the edition of Qt (i.e. DesktopLight)
#  QT_EDITION_DESKTOPLIGHT  True if QT_EDITION == DesktopLight
#  QT_QTCORE_FOUND          True if QtCore was found.
#  QT_QTGUI_FOUND           True if QtGui was found.
#  QT_QT3SUPPORT_FOUND      True if Qt3Support was found.
#  QT_QTASSISTANT_FOUND     True if QtAssistant was found.
#  QT_QTASSISTANTCLIENT_FOUND  True if QtAssistantClient was found.
#  QT_QAXCONTAINER_FOUND    True if QAxContainer was found (Windows only).
#  QT_QAXSERVER_FOUND       True if QAxServer was found (Windows only).
#  QT_QTDBUS_FOUND          True if QtDBus was found.
#  QT_QTDESIGNER_FOUND      True if QtDesigner was found.
#  QT_QTDESIGNERCOMPONENTS  True if QtDesignerComponents was found.
#  QT_QTHELP_FOUND          True if QtHelp was found.
#  QT_QTMOTIF_FOUND         True if QtMotif was found.
#  QT_QTMULTIMEDIA_FOUND    True if QtMultimedia was found (since Qt 4.6.0).
#  QT_QTNETWORK_FOUND       True if QtNetwork was found.
#  QT_QTNSPLUGIN_FOUND      True if QtNsPlugin was found.
#  QT_QTOPENGL_FOUND        True if QtOpenGL was found.
#  QT_QTSQL_FOUND           True if QtSql was found.
#  QT_QTSVG_FOUND           True if QtSvg was found.
#  QT_QTSCRIPT_FOUND        True if QtScript was found.
#  QT_QTSCRIPTTOOLS_FOUND   True if QtScriptTools was found.
#  QT_QTTEST_FOUND          True if QtTest was found.
#  QT_QTUITOOLS_FOUND       True if QtUiTools was found.
#  QT_QTWEBKIT_FOUND        True if QtWebKit was found.
#  QT_QTXML_FOUND           True if QtXml was found.
#  QT_QTXMLPATTERNS_FOUND   True if QtXmlPatterns was found.
#  QT_PHONON_FOUND          True if phonon was found.
#  QT_QTDECLARATIVE_FOUND   True if QtDeclarative was found.
#
#  QT_MAC_USE_COCOA    For Mac OS X, its whether Cocoa or Carbon is used.
#                      In general, this should not be used, but its useful
#                      when having platform specific code.
#
#  QT_DEFINITIONS   Definitions to use when compiling code that uses Qt.
#                   You do not need to use this if you include QT_USE_FILE.
#                   The QT_USE_FILE will also define QT_DEBUG and QT_NO_DEBUG
#                   to fit your current build type.  Those are not contained
#                   in QT_DEFINITIONS.
#                  
#  QT_INCLUDES      List of paths to all include directories of 
#                   Qt4 QT_INCLUDE_DIR and QT_QTCORE_INCLUDE_DIR are
#                   always in this variable even if NOTFOUND,
#                   all other INCLUDE_DIRS are
#                   only added if they are found.
#                   You do not need to use this if you include QT_USE_FILE.
#   
#
#  Include directories for the Qt modules are listed here.
#  You do not need to use these variables if you include QT_USE_FILE.
#
#  QT_INCLUDE_DIR              Path to "include" of Qt4
#  QT_QT3SUPPORT_INCLUDE_DIR   Path to "include/Qt3Support" 
#  QT_QTASSISTANT_INCLUDE_DIR  Path to "include/QtAssistant" 
#  QT_QTASSISTANTCLIENT_INCLUDE_DIR       Path to "include/QtAssistant"
#  QT_QAXCONTAINER_INCLUDE_DIR Path to "include/ActiveQt" (Windows only)
#  QT_QAXSERVER_INCLUDE_DIR    Path to "include/ActiveQt" (Windows only)
#  QT_QTCORE_INCLUDE_DIR       Path to "include/QtCore"         
#  QT_QTDBUS_INCLUDE_DIR       Path to "include/QtDBus" 
#  QT_QTDESIGNER_INCLUDE_DIR   Path to "include/QtDesigner" 
#  QT_QTDESIGNERCOMPONENTS_INCLUDE_DIR   Path to "include/QtDesigner"
#  QT_QTGUI_INCLUDE_DIR        Path to "include/QtGui" 
#  QT_QTHELP_INCLUDE_DIR       Path to "include/QtHelp"
#  QT_QTMOTIF_INCLUDE_DIR      Path to "include/QtMotif" 
#  QT_QTMULTIMEDIA_INCLUDE_DIR Path to "include/QtMultimedia" 
#  QT_QTNETWORK_INCLUDE_DIR    Path to "include/QtNetwork" 
#  QT_QTNSPLUGIN_INCLUDE_DIR   Path to "include/QtNsPlugin" 
#  QT_QTOPENGL_INCLUDE_DIR     Path to "include/QtOpenGL" 
#  QT_QTSCRIPT_INCLUDE_DIR     Path to "include/QtScript"
#  QT_QTSQL_INCLUDE_DIR        Path to "include/QtSql" 
#  QT_QTSVG_INCLUDE_DIR        Path to "include/QtSvg"
#  QT_QTTEST_INCLUDE_DIR       Path to "include/QtTest"
#  QT_QTWEBKIT_INCLUDE_DIR     Path to "include/QtWebKit"
#  QT_QTXML_INCLUDE_DIR        Path to "include/QtXml" 
#  QT_QTXMLPATTERNS_INCLUDE_DIR  Path to "include/QtXmlPatterns"
#  QT_PHONON_INCLUDE_DIR       Path to "include/phonon"
#  QT_QTSCRIPTTOOLS_INCLUDE_DIR       Path to "include/QtScriptTools"
#  QT_QTDECLARATIVE_INCLUDE_DIR       Path to "include/QtDeclarative"
#
#  QT_BINARY_DIR               Path to "bin" of Qt4
#  QT_LIBRARY_DIR              Path to "lib" of Qt4
#  QT_PLUGINS_DIR              Path to "plugins" for Qt4
#  QT_TRANSLATIONS_DIR         Path to "translations" of Qt4
#  QT_IMPORTS_DIR              Path to "imports" of Qt4
#  QT_DOC_DIR                  Path to "doc" of Qt4
#  QT_MKSPECS_DIR              Path to "mkspecs" of Qt4
#
#
# The Qt toolkit may contain both debug and release libraries.
# In that case, the following library variables will contain both.
# You do not need to use these variables if you include QT_USE_FILE,
# and use QT_LIBRARIES.
#
#  QT_QT3SUPPORT_LIBRARY            The Qt3Support library
#  QT_QTASSISTANT_LIBRARY           The QtAssistant library
#  QT_QTASSISTANTCLIENT_LIBRARY     The QtAssistantClient library
#  QT_QAXCONTAINER_LIBRARY           The QAxContainer library (Windows only)
#  QT_QAXSERVER_LIBRARY                The QAxServer library (Windows only)
#  QT_QTCORE_LIBRARY                The QtCore library
#  QT_QTDBUS_LIBRARY                The QtDBus library
#  QT_QTDESIGNER_LIBRARY            The QtDesigner library
#  QT_QTDESIGNERCOMPONENTS_LIBRARY  The QtDesignerComponents library
#  QT_QTGUI_LIBRARY                 The QtGui library
#  QT_QTHELP_LIBRARY                The QtHelp library
#  QT_QTMOTIF_LIBRARY               The QtMotif library
#  QT_QTMULTIMEDIA_LIBRARY          The QtMultimedia library
#  QT_QTNETWORK_LIBRARY             The QtNetwork library
#  QT_QTNSPLUGIN_LIBRARY            The QtNsPLugin library
#  QT_QTOPENGL_LIBRARY              The QtOpenGL library
#  QT_QTSCRIPT_LIBRARY              The QtScript library
#  QT_QTSQL_LIBRARY                 The QtSql library
#  QT_QTSVG_LIBRARY                 The QtSvg library
#  QT_QTTEST_LIBRARY                The QtTest library
#  QT_QTUITOOLS_LIBRARY             The QtUiTools library
#  QT_QTWEBKIT_LIBRARY              The QtWebKit library
#  QT_QTXML_LIBRARY                 The QtXml library
#  QT_QTXMLPATTERNS_LIBRARY         The QtXmlPatterns library
#  QT_QTMAIN_LIBRARY                The qtmain library for Windows
#  QT_PHONON_LIBRARY                The phonon library
#  QT_QTSCRIPTTOOLS_LIBRARY         The QtScriptTools library
#  
# The QtDeclarative library:             QT_QTDECLARATIVE_LIBRARY
#
# also defined, but NOT for general use are
#  QT_MOC_EXECUTABLE                   Where to find the moc tool.
#  QT_UIC_EXECUTABLE                   Where to find the uic tool.
#  QT_UIC3_EXECUTABLE                  Where to find the uic3 tool.
#  QT_RCC_EXECUTABLE                   Where to find the rcc tool
#  QT_DBUSCPP2XML_EXECUTABLE           Where to find the qdbuscpp2xml tool.
#  QT_DBUSXML2CPP_EXECUTABLE           Where to find the qdbusxml2cpp tool.
#  QT_LUPDATE_EXECUTABLE               Where to find the lupdate tool.
#  QT_LRELEASE_EXECUTABLE              Where to find the lrelease tool.
#  QT_QCOLLECTIONGENERATOR_EXECUTABLE  Where to find the qcollectiongenerator tool.
#  QT_DESIGNER_EXECUTABLE              Where to find the Qt designer tool.
#  QT_LINGUIST_EXECUTABLE              Where to find the Qt linguist tool.
#  
#
# These are around for backwards compatibility 
# they will be set
#  QT_WRAP_CPP  Set true if QT_MOC_EXECUTABLE is found
#  QT_WRAP_UI   Set true if QT_UIC_EXECUTABLE is found
#  
# These variables do _NOT_ have any effect anymore (compared to FindQt.cmake)
#  QT_MT_REQUIRED         Qt4 is now always multithreaded
#  
# These variables are set to "" Because Qt structure changed 
# (They make no sense in Qt4)
#  QT_QT_LIBRARY        Qt-Library is now split

#=============================================================================
# Copyright 2005-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# Use FIND_PACKAGE( Qt4 COMPONENTS ... ) to enable modules
IF( Qt4_FIND_COMPONENTS )
  FOREACH( component ${Qt4_FIND_COMPONENTS} )
    STRING( TOUPPER ${component} _COMPONENT )
    SET( QT_USE_${_COMPONENT} 1 )
  ENDFOREACH( component )
  
  # To make sure we don't use QtCore or QtGui when not in COMPONENTS
  IF(NOT QT_USE_QTCORE)
    SET( QT_DONT_USE_QTCORE 1 )
  ENDIF(NOT QT_USE_QTCORE)
  
  IF(NOT QT_USE_QTGUI)
    SET( QT_DONT_USE_QTGUI 1 )
  ENDIF(NOT QT_USE_QTGUI)

ENDIF( Qt4_FIND_COMPONENTS )

# If Qt3 has already been found, fail.
IF(QT_QT_LIBRARY)
  IF(Qt4_FIND_REQUIRED)
    MESSAGE( FATAL_ERROR "Qt3 and Qt4 cannot be used together in one project.  If switching to Qt4, the CMakeCache.txt needs to be cleaned.")
  ELSE(Qt4_FIND_REQUIRED)
    IF(NOT Qt4_FIND_QUIETLY)
      MESSAGE( STATUS    "Qt3 and Qt4 cannot be used together in one project.  If switching to Qt4, the CMakeCache.txt needs to be cleaned.")
    ENDIF(NOT Qt4_FIND_QUIETLY)
    RETURN()
  ENDIF(Qt4_FIND_REQUIRED)
ENDIF(QT_QT_LIBRARY)


INCLUDE(CheckCXXSymbolExists)
INCLUDE(MacroAddFileDependencies)
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)

SET(QT_USE_FILE ${CMAKE_ROOT}/Modules/UseQt4.cmake)

SET( QT_DEFINITIONS "")

# convenience macro for dealing with debug/release library names
MACRO (_QT4_ADJUST_LIB_VARS _camelCaseBasename)

  STRING(TOUPPER "${_camelCaseBasename}" basename)

  # The name of the imported targets, i.e. the prefix "Qt4::" must not change,
  # since it is stored in EXPORT-files as name of a required library. If the name would change
  # here, this would lead to the imported Qt4-library targets not being resolved by cmake anymore.
  IF (QT_${basename}_LIBRARY_RELEASE OR QT_${basename}_LIBRARY_DEBUG)

    IF(NOT TARGET Qt4::${_camelCaseBasename})
      ADD_LIBRARY(Qt4::${_camelCaseBasename} UNKNOWN IMPORTED )

      IF (QT_${basename}_LIBRARY_RELEASE)
        SET_PROPERTY(TARGET Qt4::${_camelCaseBasename} APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
        if(QT_USE_FRAMEWORKS)
          SET_PROPERTY(TARGET Qt4::${_camelCaseBasename}        PROPERTY IMPORTED_LOCATION_RELEASE "${QT_${basename}_LIBRARY_RELEASE}/${_camelCaseBasename}" )
        else()
          SET_PROPERTY(TARGET Qt4::${_camelCaseBasename}        PROPERTY IMPORTED_LOCATION_RELEASE "${QT_${basename}_LIBRARY_RELEASE}" )
        endif()
      ENDIF (QT_${basename}_LIBRARY_RELEASE)

      IF (QT_${basename}_LIBRARY_DEBUG)
        SET_PROPERTY(TARGET Qt4::${_camelCaseBasename} APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
        if(QT_USE_FRAMEWORKS)
          SET_PROPERTY(TARGET Qt4::${_camelCaseBasename}        PROPERTY IMPORTED_LOCATION_DEBUG "${QT_${basename}_LIBRARY_DEBUG}/${_camelCaseBasename}" )
        else()
          SET_PROPERTY(TARGET Qt4::${_camelCaseBasename}        PROPERTY IMPORTED_LOCATION_DEBUG "${QT_${basename}_LIBRARY_DEBUG}" )
        endif()
      ENDIF (QT_${basename}_LIBRARY_DEBUG)
    ENDIF(NOT TARGET Qt4::${_camelCaseBasename})

    # If QT_USE_IMPORTED_TARGETS is enabled, the QT_QTFOO_LIBRARY variables are set to point at these
    # imported targets. This works better in general, and is also in almost all cases fully
    # backward compatible. The only issue is when a project A which had this enabled then exports its
    # libraries via export or EXPORT_LIBRARY_DEPENDENCIES(). In this case the libraries from project
    # A will depend on the imported Qt targets, and the names of these imported targets will be stored
    # in the dependency files on disk. This means when a project B then uses project A, these imported
    # targets must be created again, otherwise e.g. "Qt4__QtCore" will be interpreted as name of a
    # library file on disk, and not as a target, and linking will fail:
    IF(QT_USE_IMPORTED_TARGETS)
        SET(QT_${basename}_LIBRARY       Qt4::${_camelCaseBasename} )
        SET(QT_${basename}_LIBRARIES     Qt4::${_camelCaseBasename} )
    ELSE(QT_USE_IMPORTED_TARGETS)

      # if the release- as well as the debug-version of the library have been found:
      IF (QT_${basename}_LIBRARY_DEBUG AND QT_${basename}_LIBRARY_RELEASE)
        # if the generator supports configuration types then set
        # optimized and debug libraries, or if the CMAKE_BUILD_TYPE has a value
        IF (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
          SET(QT_${basename}_LIBRARY       optimized ${QT_${basename}_LIBRARY_RELEASE} debug ${QT_${basename}_LIBRARY_DEBUG})
        ELSE(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
          # if there are no configuration types and CMAKE_BUILD_TYPE has no value
          # then just use the release libraries
          SET(QT_${basename}_LIBRARY       ${QT_${basename}_LIBRARY_RELEASE} )
        ENDIF(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
        SET(QT_${basename}_LIBRARIES       optimized ${QT_${basename}_LIBRARY_RELEASE} debug ${QT_${basename}_LIBRARY_DEBUG})
      ENDIF (QT_${basename}_LIBRARY_DEBUG AND QT_${basename}_LIBRARY_RELEASE)

      # if only the release version was found, set the debug variable also to the release version
      IF (QT_${basename}_LIBRARY_RELEASE AND NOT QT_${basename}_LIBRARY_DEBUG)
        SET(QT_${basename}_LIBRARY_DEBUG ${QT_${basename}_LIBRARY_RELEASE})
        SET(QT_${basename}_LIBRARY       ${QT_${basename}_LIBRARY_RELEASE})
        SET(QT_${basename}_LIBRARIES     ${QT_${basename}_LIBRARY_RELEASE})
      ENDIF (QT_${basename}_LIBRARY_RELEASE AND NOT QT_${basename}_LIBRARY_DEBUG)

      # if only the debug version was found, set the release variable also to the debug version
      IF (QT_${basename}_LIBRARY_DEBUG AND NOT QT_${basename}_LIBRARY_RELEASE)
        SET(QT_${basename}_LIBRARY_RELEASE ${QT_${basename}_LIBRARY_DEBUG})
        SET(QT_${basename}_LIBRARY         ${QT_${basename}_LIBRARY_DEBUG})
        SET(QT_${basename}_LIBRARIES       ${QT_${basename}_LIBRARY_DEBUG})
      ENDIF (QT_${basename}_LIBRARY_DEBUG AND NOT QT_${basename}_LIBRARY_RELEASE)

      # put the value in the cache:
      SET(QT_${basename}_LIBRARY ${QT_${basename}_LIBRARY} CACHE STRING "The Qt ${basename} library" FORCE)

    ENDIF(QT_USE_IMPORTED_TARGETS)

    SET(QT_${basename}_FOUND 1)

  ELSE (QT_${basename}_LIBRARY_RELEASE OR QT_${basename}_LIBRARY_DEBUG)

    SET(QT_${basename}_LIBRARY "" CACHE STRING "The Qt ${basename} library" FORCE)

  ENDIF (QT_${basename}_LIBRARY_RELEASE OR QT_${basename}_LIBRARY_DEBUG)

  IF (QT_${basename}_INCLUDE_DIR)
    #add the include directory to QT_INCLUDES
    SET(QT_INCLUDES "${QT_${basename}_INCLUDE_DIR}" ${QT_INCLUDES})
  ENDIF (QT_${basename}_INCLUDE_DIR)

  # Make variables changeble to the advanced user
  MARK_AS_ADVANCED(QT_${basename}_LIBRARY QT_${basename}_LIBRARY_RELEASE QT_${basename}_LIBRARY_DEBUG QT_${basename}_INCLUDE_DIR)
ENDMACRO (_QT4_ADJUST_LIB_VARS)

function(_QT4_QUERY_QMAKE VAR RESULT)
  execute_process(COMMAND "${QT_QMAKE_EXECUTABLE}" -query ${VAR}
    RESULT_VARIABLE return_code
    OUTPUT_VARIABLE output ERROR_VARIABLE output
    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE)
  if(NOT return_code)
    file(TO_CMAKE_PATH "${output}" output)
    set(${RESULT} ${output} PARENT_SCOPE)
  endif(NOT return_code)
endfunction(_QT4_QUERY_QMAKE)


SET(QT4_INSTALLED_VERSION_TOO_OLD FALSE)

GET_FILENAME_COMPONENT(qt_install_version "[HKEY_CURRENT_USER\\Software\\trolltech\\Versions;DefaultQtVersion]" NAME)
# check for qmake
# Debian uses qmake-qt4
# macports' Qt uses qmake-mac
FIND_PROGRAM(QT_QMAKE_EXECUTABLE NAMES qmake qmake4 qmake-qt4 qmake-mac PATHS
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\4.0.0;InstallDir]/bin"
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Versions\\4.0.0;InstallDir]/bin"
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Versions\\${qt_install_version};InstallDir]/bin"
  $ENV{QTDIR}/bin
  DOC "The qmake executable for the Qt installation to use"
)

# double check that it was a Qt4 qmake, if not, re-find with different names
IF (QT_QMAKE_EXECUTABLE)

  IF(QT_QMAKE_EXECUTABLE_LAST)
    STRING(COMPARE NOTEQUAL "${QT_QMAKE_EXECUTABLE_LAST}" "${QT_QMAKE_EXECUTABLE}" QT_QMAKE_CHANGED)
  ENDIF(QT_QMAKE_EXECUTABLE_LAST)

  SET(QT_QMAKE_EXECUTABLE_LAST "${QT_QMAKE_EXECUTABLE}" CACHE INTERNAL "" FORCE)

  _qt4_query_qmake(QT_VERSION QTVERSION)

  # check for qt3 qmake and then try and find qmake4 or qmake-qt4 in the path
  IF(NOT QTVERSION)
    SET(QT_QMAKE_EXECUTABLE NOTFOUND CACHE FILEPATH "" FORCE)
    FIND_PROGRAM(QT_QMAKE_EXECUTABLE NAMES qmake4 qmake-qt4 PATHS
      "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\4.0.0;InstallDir]/bin"
      "[HKEY_CURRENT_USER\\Software\\Trolltech\\Versions\\4.0.0;InstallDir]/bin"
      $ENV{QTDIR}/bin
      DOC "The qmake executable for the Qt installation to use"
      )
    IF(QT_QMAKE_EXECUTABLE)
      _qt4_query_qmake(QT_VERSION QTVERSION)
    ENDIF(QT_QMAKE_EXECUTABLE)
  ENDIF(NOT QTVERSION)

ENDIF (QT_QMAKE_EXECUTABLE)

IF (QT_QMAKE_EXECUTABLE AND QTVERSION)

  # ask qmake for the mkspecs directory
  # we do this first because QT_LIBINFIX might be set
  IF (NOT QT_MKSPECS_DIR  OR  QT_QMAKE_CHANGED)
    _qt4_query_qmake(QMAKE_MKSPECS qt_mkspecs_dirs)
    # do not replace : on windows as it might be a drive letter
    # and windows should already use ; as a separator
    IF(NOT WIN32)
      STRING(REPLACE ":" ";" qt_mkspecs_dirs "${qt_mkspecs_dirs}")
    ENDIF(NOT WIN32)
    set(qt_cross_paths)
    foreach(qt_cross_path ${CMAKE_FIND_ROOT_PATH})
      set(qt_cross_paths ${qt_cross_paths} "${qt_cross_path}/mkspecs")
    endforeach(qt_cross_path)
    SET(QT_MKSPECS_DIR NOTFOUND)
    FIND_PATH(QT_MKSPECS_DIR NAMES qconfig.pri
      HINTS ${qt_cross_paths} ${qt_mkspecs_dirs}
      DOC "The location of the Qt mkspecs containing qconfig.pri")
  ENDIF()

  IF(EXISTS "${QT_MKSPECS_DIR}/qconfig.pri")
    FILE(READ ${QT_MKSPECS_DIR}/qconfig.pri _qconfig_FILE_contents)
    STRING(REGEX MATCH "QT_CONFIG[^\n]+" QT_QCONFIG "${_qconfig_FILE_contents}")
    STRING(REGEX MATCH "CONFIG[^\n]+" QT_CONFIG "${_qconfig_FILE_contents}")
    STRING(REGEX MATCH "EDITION[^\n]+" QT_EDITION "${_qconfig_FILE_contents}")
    STRING(REGEX MATCH "QT_LIBINFIX[^\n]+" _qconfig_qt_libinfix "${_qconfig_FILE_contents}")
    STRING(REGEX REPLACE "QT_LIBINFIX *= *([^\n]*)" "\\1" QT_LIBINFIX "${_qconfig_qt_libinfix}")
  ENDIF(EXISTS "${QT_MKSPECS_DIR}/qconfig.pri")
  IF("${QT_EDITION}" MATCHES "DesktopLight")
    SET(QT_EDITION_DESKTOPLIGHT 1)
  ENDIF("${QT_EDITION}" MATCHES "DesktopLight")

  # ask qmake for the library dir as a hint, then search for QtCore library and use that as a reference for finding the
  # others and for setting QT_LIBRARY_DIR
  IF (NOT (QT_QTCORE_LIBRARY_RELEASE OR QT_QTCORE_LIBRARY_DEBUG)  OR QT_QMAKE_CHANGED)
    _qt4_query_qmake(QT_INSTALL_LIBS QT_LIBRARY_DIR_TMP)
    SET(QT_QTCORE_LIBRARY_RELEASE NOTFOUND)
    SET(QT_QTCORE_LIBRARY_DEBUG NOTFOUND)
    FIND_LIBRARY(QT_QTCORE_LIBRARY_RELEASE
                 NAMES QtCore${QT_LIBINFIX} QtCore${QT_LIBINFIX}4
                 HINTS ${QT_LIBRARY_DIR_TMP}
                 NO_DEFAULT_PATH
        )
    FIND_LIBRARY(QT_QTCORE_LIBRARY_DEBUG
                 NAMES QtCore${QT_LIBINFIX}_debug QtCore${QT_LIBINFIX}d QtCore${QT_LIBINFIX}d4
                 HINTS ${QT_LIBRARY_DIR_TMP}
                 NO_DEFAULT_PATH
        )

    IF(NOT QT_QTCORE_LIBRARY_RELEASE AND NOT QT_QTCORE_LIBRARY_DEBUG)
      FIND_LIBRARY(QT_QTCORE_LIBRARY_RELEASE
                   NAMES QtCore${QT_LIBINFIX} QtCore${QT_LIBINFIX}4
                   HINTS ${QT_LIBRARY_DIR_TMP}
          )
      FIND_LIBRARY(QT_QTCORE_LIBRARY_DEBUG
                   NAMES QtCore${QT_LIBINFIX}_debug QtCore${QT_LIBINFIX}d QtCore${QT_LIBINFIX}d4
                   HINTS ${QT_LIBRARY_DIR_TMP}
          )
    ENDIF(NOT QT_QTCORE_LIBRARY_RELEASE AND NOT QT_QTCORE_LIBRARY_DEBUG)

    # try dropping a hint if trying to use Visual Studio with Qt built by mingw
    IF(NOT QT_QTCORE_LIBRARY_RELEASE AND MSVC)
      IF(EXISTS ${QT_LIBRARY_DIR_TMP}/libqtmain.a)
        MESSAGE( FATAL_ERROR "It appears you're trying to use Visual Studio with Qt built by mingw.  Those compilers do not produce code compatible with each other.")
      ENDIF(EXISTS ${QT_LIBRARY_DIR_TMP}/libqtmain.a)
    ENDIF(NOT QT_QTCORE_LIBRARY_RELEASE AND MSVC)

  ENDIF ()

  # set QT_LIBRARY_DIR based on location of QtCore found.
  IF(QT_QTCORE_LIBRARY_RELEASE)
    GET_FILENAME_COMPONENT(QT_LIBRARY_DIR_TMP "${QT_QTCORE_LIBRARY_RELEASE}" PATH)
    SET(QT_LIBRARY_DIR ${QT_LIBRARY_DIR_TMP} CACHE INTERNAL "Qt library dir" FORCE)
    SET(QT_QTCORE_FOUND 1)
  ELSEIF(QT_QTCORE_LIBRARY_DEBUG)
    GET_FILENAME_COMPONENT(QT_LIBRARY_DIR_TMP "${QT_QTCORE_LIBRARY_DEBUG}" PATH)
    SET(QT_LIBRARY_DIR ${QT_LIBRARY_DIR_TMP} CACHE INTERNAL "Qt library dir" FORCE)
    SET(QT_QTCORE_FOUND 1)
  ELSE()
    MESSAGE(WARNING "${QT_QMAKE_EXECUTABLE} reported QT_INSTALL_LIBS as \"${QT_LIBRARY_DIR_TMP}\" "
                    "but QtCore could not be found there.  "
                    "Qt is NOT installed correctly for the target build environment.")
    IF(Qt4_FIND_REQUIRED)
      MESSAGE( FATAL_ERROR "Could NOT find QtCore. Check ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log for more details.")
    ENDIF(Qt4_FIND_REQUIRED)
  ENDIF()

  # ask qmake for the binary dir
  IF (NOT QT_BINARY_DIR  OR  QT_QMAKE_CHANGED)
    _qt4_query_qmake(QT_INSTALL_BINS qt_bins)
    SET(QT_BINARY_DIR ${qt_bins} CACHE INTERNAL "" FORCE)
  ENDIF (NOT QT_BINARY_DIR  OR  QT_QMAKE_CHANGED)

  IF (APPLE)
    SET(CMAKE_FIND_FRAMEWORK_OLD ${CMAKE_FIND_FRAMEWORK})
    IF (EXISTS ${QT_LIBRARY_DIR}/QtCore.framework)
      SET(QT_USE_FRAMEWORKS ON CACHE INTERNAL "" FORCE)
      SET(CMAKE_FIND_FRAMEWORK FIRST)
    ELSE (EXISTS ${QT_LIBRARY_DIR}/QtCore.framework)
      SET(QT_USE_FRAMEWORKS OFF CACHE INTERNAL "" FORCE)
      SET(CMAKE_FIND_FRAMEWORK LAST)
    ENDIF (EXISTS ${QT_LIBRARY_DIR}/QtCore.framework)
  ENDIF (APPLE)

  # ask qmake for the include dir
  IF (QT_LIBRARY_DIR AND (NOT QT_QTCORE_INCLUDE_DIR OR NOT QT_HEADERS_DIR OR  QT_QMAKE_CHANGED))
      _qt4_query_qmake(QT_INSTALL_HEADERS qt_headers)
      SET(QT_QTCORE_INCLUDE_DIR NOTFOUND)
      FIND_PATH(QT_QTCORE_INCLUDE_DIR QtCore
                HINTS ${qt_headers} ${QT_LIBRARY_DIR}
                PATH_SUFFIXES QtCore qt4/QtCore
        )

      # Set QT_HEADERS_DIR based on finding QtCore header
      IF(QT_QTCORE_INCLUDE_DIR)
        IF(QT_USE_FRAMEWORKS)
          SET(QT_HEADERS_DIR "${qt_headers}" CACHE INTERNAL "" FORCE)
        ELSE(QT_USE_FRAMEWORKS)
          GET_FILENAME_COMPONENT(qt_headers "${QT_QTCORE_INCLUDE_DIR}/../" ABSOLUTE)
          SET(QT_HEADERS_DIR "${qt_headers}" CACHE INTERNAL "" FORCE)
        ENDIF(QT_USE_FRAMEWORKS)
      ELSEIF()
        MESSAGE("Warning: QT_QMAKE_EXECUTABLE reported QT_INSTALL_HEADERS as ${qt_headers}")
        MESSAGE("Warning: But QtCore couldn't be found.  Qt must NOT be installed correctly.")
      ENDIF()
  ENDIF()

  IF(APPLE)
    SET(CMAKE_FIND_FRAMEWORK ${CMAKE_FIND_FRAMEWORK_OLD})
  ENDIF(APPLE)

  # Set QT_INCLUDE_DIR based on QT_HEADERS_DIR
  IF(QT_HEADERS_DIR)
    IF(QT_USE_FRAMEWORKS)
      # Qt/Mac frameworks has two include dirs.
      # One is the framework include for which CMake will add a -F flag
      # and the other is an include dir for non-framework Qt modules
      SET(QT_INCLUDE_DIR ${QT_HEADERS_DIR} ${QT_QTCORE_LIBRARY_RELEASE} )
    ELSE(QT_USE_FRAMEWORKS)
      SET(QT_INCLUDE_DIR ${QT_HEADERS_DIR})
    ENDIF(QT_USE_FRAMEWORKS)
  ENDIF(QT_HEADERS_DIR)

  # Set QT_INCLUDES
  SET( QT_INCLUDES ${QT_MKSPECS_DIR}/default ${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR})


  # ask qmake for the documentation directory
  IF (QT_LIBRARY_DIR AND NOT QT_DOC_DIR  OR  QT_QMAKE_CHANGED)
    _qt4_query_qmake(QT_INSTALL_DOCS qt_doc_dir)
    SET(QT_DOC_DIR ${qt_doc_dir} CACHE PATH "The location of the Qt docs" FORCE)
  ENDIF (QT_LIBRARY_DIR AND NOT QT_DOC_DIR  OR  QT_QMAKE_CHANGED)


  # ask qmake for the plugins directory
  IF (QT_LIBRARY_DIR AND NOT QT_PLUGINS_DIR  OR  QT_QMAKE_CHANGED)
    _qt4_query_qmake(QT_INSTALL_PLUGINS qt_plugins_dir)
    SET(QT_PLUGINS_DIR NOTFOUND)
    foreach(qt_cross_path ${CMAKE_FIND_ROOT_PATH})
      set(qt_cross_paths ${qt_cross_paths} "${qt_cross_path}/plugins")
    endforeach(qt_cross_path)
    FIND_PATH(QT_PLUGINS_DIR NAMES accessible imageformats sqldrivers codecs designer
      HINTS ${qt_cross_paths} ${qt_plugins_dir}
      DOC "The location of the Qt plugins")
  ENDIF (QT_LIBRARY_DIR AND NOT QT_PLUGINS_DIR  OR  QT_QMAKE_CHANGED)

  # ask qmake for the translations directory
  IF (QT_LIBRARY_DIR AND NOT QT_TRANSLATIONS_DIR  OR  QT_QMAKE_CHANGED)
    _qt4_query_qmake(QT_INSTALL_TRANSLATIONS qt_translations_dir)
    SET(QT_TRANSLATIONS_DIR ${qt_translations_dir} CACHE PATH "The location of the Qt translations" FORCE)
  ENDIF (QT_LIBRARY_DIR AND NOT QT_TRANSLATIONS_DIR  OR  QT_QMAKE_CHANGED)

  # ask qmake for the imports directory
  IF (QT_LIBRARY_DIR AND NOT QT_IMPORTS_DIR OR QT_QMAKE_CHANGED)
    _qt4_query_qmake(QT_INSTALL_IMPORTS qt_imports_dir)
    if(qt_imports_dir)
      SET(QT_IMPORTS_DIR NOTFOUND)
      foreach(qt_cross_path ${CMAKE_FIND_ROOT_PATH})
        set(qt_cross_paths ${qt_cross_paths} "${qt_cross_path}/imports")
      endforeach(qt_cross_path)
      FIND_PATH(QT_IMPORTS_DIR NAMES Qt
        HINTS ${qt_cross_paths} ${qt_imports_dir}
        DOC "The location of the Qt imports"
        NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_SYSTEM_ENVIRONMENT_PATH
        NO_CMAKE_SYSTEM_PATH)
      mark_as_advanced(QT_IMPORTS_DIR)
    endif(qt_imports_dir)
  ENDIF (QT_LIBRARY_DIR AND NOT QT_IMPORTS_DIR  OR  QT_QMAKE_CHANGED)

  # Make variables changeble to the advanced user
  MARK_AS_ADVANCED( QT_LIBRARY_DIR QT_DOC_DIR QT_MKSPECS_DIR
                    QT_PLUGINS_DIR QT_TRANSLATIONS_DIR)




  #############################################
  #
  # Find out what window system we're using
  #
  #############################################
  # Save required variable
  SET(CMAKE_REQUIRED_INCLUDES_SAVE ${CMAKE_REQUIRED_INCLUDES})
  SET(CMAKE_REQUIRED_FLAGS_SAVE    ${CMAKE_REQUIRED_FLAGS})
  # Add QT_INCLUDE_DIR to CMAKE_REQUIRED_INCLUDES
  SET(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES};${QT_INCLUDE_DIR}")
  # Check for Window system symbols (note: only one should end up being set)
  CHECK_CXX_SYMBOL_EXISTS(Q_WS_X11 "QtCore/qglobal.h" Q_WS_X11)
  CHECK_CXX_SYMBOL_EXISTS(Q_WS_WIN "QtCore/qglobal.h" Q_WS_WIN)
  CHECK_CXX_SYMBOL_EXISTS(Q_WS_QWS "QtCore/qglobal.h" Q_WS_QWS)
  CHECK_CXX_SYMBOL_EXISTS(Q_WS_MAC "QtCore/qglobal.h" Q_WS_MAC)
  IF(Q_WS_MAC)
    IF(QT_QMAKE_CHANGED)
      UNSET(QT_MAC_USE_COCOA CACHE)
    ENDIF(QT_QMAKE_CHANGED)
    CHECK_CXX_SYMBOL_EXISTS(QT_MAC_USE_COCOA "QtCore/qconfig.h" QT_MAC_USE_COCOA)
  ENDIF(Q_WS_MAC)

  IF (QT_QTCOPY_REQUIRED)
     CHECK_CXX_SYMBOL_EXISTS(QT_IS_QTCOPY "QtCore/qglobal.h" QT_KDE_QT_COPY)
     IF (NOT QT_IS_QTCOPY)
        MESSAGE(FATAL_ERROR "qt-copy is required, but hasn't been found")
     ENDIF (NOT QT_IS_QTCOPY)
  ENDIF (QT_QTCOPY_REQUIRED)

  # Restore CMAKE_REQUIRED_INCLUDES and CMAKE_REQUIRED_FLAGS variables
  SET(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES_SAVE})
  SET(CMAKE_REQUIRED_FLAGS    ${CMAKE_REQUIRED_FLAGS_SAVE})
  #
  #############################################



  ########################################
  #
  #       Setting the INCLUDE-Variables
  #
  ########################################

  SET(QT_MODULES QtGui Qt3Support QtSvg QtScript QtTest QtUiTools
                 QtHelp QtWebKit QtXmlPatterns phonon QtNetwork QtMultimedia
                 QtNsPlugin QtOpenGL QtSql QtXml QtDesigner QtDBus QtScriptTools
                 QtDeclarative)
  
  IF(Q_WS_X11)
    SET(QT_MODULES ${QT_MODULES} QtMotif)
  ENDIF(Q_WS_X11)

  IF(QT_QMAKE_CHANGED)
    FOREACH(QT_MODULE ${QT_MODULES})
      STRING(TOUPPER ${QT_MODULE} _upper_qt_module)
      SET(QT_${_upper_qt_module}_INCLUDE_DIR NOTFOUND)
      SET(QT_${_upper_qt_module}_LIBRARY_RELEASE NOTFOUND)
      SET(QT_${_upper_qt_module}_LIBRARY_DEBUG NOTFOUND)
    ENDFOREACH(QT_MODULE)
    SET(QT_QTDESIGNERCOMPONENTS_INCLUDE_DIR NOTFOUND)
    SET(QT_QTDESIGNERCOMPONENTS_LIBRARY_RELEASE NOTFOUND)
    SET(QT_QTDESIGNERCOMPONENTS_LIBRARY_DEBUG NOTFOUND)
    SET(QT_QTASSISTANTCLIENT_INCLUDE_DIR NOTFOUND)
    SET(QT_QTASSISTANTCLIENT_LIBRARY_RELEASE NOTFOUND)
    SET(QT_QTASSISTANTCLIENT_LIBRARY_DEBUG NOTFOUND)
    SET(QT_QTASSISTANT_INCLUDE_DIR NOTFOUND)
    SET(QT_QTASSISTANT_LIBRARY_RELEASE NOTFOUND)
    SET(QT_QTASSISTANT_LIBRARY_DEBUG NOTFOUND)
    SET(QT_QTCLUCENE_LIBRARY_RELEASE NOTFOUND)
    SET(QT_QTCLUCENE_LIBRARY_DEBUG NOTFOUND)
    SET(QT_QAXCONTAINER_INCLUDE_DIR NOTFOUND)
    SET(QT_QAXCONTAINER_LIBRARY_RELEASE NOTFOUND)
    SET(QT_QAXCONTAINER_LIBRARY_DEBUG NOTFOUND)
    SET(QT_QAXSERVER_INCLUDE_DIR NOTFOUND)
    SET(QT_QAXSERVER_LIBRARY_RELEASE NOTFOUND)
    SET(QT_QAXSERVER_LIBRARY_DEBUG NOTFOUND)
    IF(Q_WS_WIN)
      SET(QT_QTMAIN_LIBRARY_DEBUG NOTFOUND)
      SET(QT_QTMAIN_LIBRARY_RELEASE NOTFOUND)
    ENDIF(Q_WS_WIN)
  ENDIF(QT_QMAKE_CHANGED)

  FOREACH(QT_MODULE ${QT_MODULES})
    STRING(TOUPPER ${QT_MODULE} _upper_qt_module)
    FIND_PATH(QT_${_upper_qt_module}_INCLUDE_DIR ${QT_MODULE}
              PATHS
              ${QT_HEADERS_DIR}/${QT_MODULE}
              ${QT_LIBRARY_DIR}/${QT_MODULE}.framework/Headers
              NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
      )
    # phonon doesn't seem consistent, let's try phonondefs.h for some
    # installations
    IF(${QT_MODULE} STREQUAL "phonon")
      FIND_PATH(QT_${_upper_qt_module}_INCLUDE_DIR phonondefs.h
                PATHS
                ${QT_HEADERS_DIR}/${QT_MODULE}
                ${QT_LIBRARY_DIR}/${QT_MODULE}.framework/Headers
                NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
        )
    ENDIF(${QT_MODULE} STREQUAL "phonon")
  ENDFOREACH(QT_MODULE)

  IF(Q_WS_WIN)
    SET(QT_MODULES ${QT_MODULES} QAxContainer QAxServer)
    # Set QT_AXCONTAINER_INCLUDE_DIR and QT_AXSERVER_INCLUDE_DIR
    FIND_PATH(QT_QAXCONTAINER_INCLUDE_DIR ActiveQt
      PATHS ${QT_HEADERS_DIR}/ActiveQt
      NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
      )
    FIND_PATH(QT_QAXSERVER_INCLUDE_DIR ActiveQt
      PATHS ${QT_HEADERS_DIR}/ActiveQt
      NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
      )
  ENDIF(Q_WS_WIN)

  # Set QT_QTDESIGNERCOMPONENTS_INCLUDE_DIR
  FIND_PATH(QT_QTDESIGNERCOMPONENTS_INCLUDE_DIR QDesignerComponents
    PATHS
    ${QT_HEADERS_DIR}/QtDesigner
    ${QT_LIBRARY_DIR}/QtDesigner.framework/Headers
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )
  
  # Set QT_QTASSISTANT_INCLUDE_DIR
  FIND_PATH(QT_QTASSISTANT_INCLUDE_DIR QtAssistant
    PATHS
    ${QT_HEADERS_DIR}/QtAssistant
    ${QT_LIBRARY_DIR}/QtAssistant.framework/Headers
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )
  
  # Set QT_QTASSISTANTCLIENT_INCLUDE_DIR
  FIND_PATH(QT_QTASSISTANTCLIENT_INCLUDE_DIR QAssistantClient
    PATHS
    ${QT_HEADERS_DIR}/QtAssistant
    ${QT_LIBRARY_DIR}/QtAssistant.framework/Headers
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  ########################################
  #
  #       Setting the LIBRARY-Variables
  #
  ########################################

  # find the libraries
  FOREACH(QT_MODULE ${QT_MODULES})
    STRING(TOUPPER ${QT_MODULE} _upper_qt_module)
    FIND_LIBRARY(QT_${_upper_qt_module}_LIBRARY_RELEASE 
                 NAMES ${QT_MODULE}${QT_LIBINFIX} ${QT_MODULE}${QT_LIBINFIX}4
                 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
        )
    FIND_LIBRARY(QT_${_upper_qt_module}_LIBRARY_DEBUG 
                 NAMES ${QT_MODULE}${QT_LIBINFIX}_debug ${QT_MODULE}${QT_LIBINFIX}d ${QT_MODULE}${QT_LIBINFIX}d4
                 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
        )
  ENDFOREACH(QT_MODULE)

  # QtUiTools is sometimes not in the same directory as the other found libraries
  # e.g. on Mac, its never a framework like the others are
  IF(QT_QTCORE_LIBRARY_RELEASE AND NOT QT_QTUITOOLS_LIBRARY_RELEASE)
    FIND_LIBRARY(QT_QTUITOOLS_LIBRARY_RELEASE NAMES QtUiTools${QT_LIBINFIX} PATHS ${QT_LIBRARY_DIR})
  ENDIF(QT_QTCORE_LIBRARY_RELEASE AND NOT QT_QTUITOOLS_LIBRARY_RELEASE)

  # Set QT_QTDESIGNERCOMPONENTS_LIBRARY
  FIND_LIBRARY(QT_QTDESIGNERCOMPONENTS_LIBRARY_RELEASE NAMES QtDesignerComponents${QT_LIBINFIX} QtDesignerComponents${QT_LIBINFIX}4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  FIND_LIBRARY(QT_QTDESIGNERCOMPONENTS_LIBRARY_DEBUG   NAMES QtDesignerComponents${QT_LIBINFIX}_debug QtDesignerComponents${QT_LIBINFIX}d QtDesignerComponents${QT_LIBINFIX}d4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)

  # Set QT_QTMAIN_LIBRARY
  IF(Q_WS_WIN)
    FIND_LIBRARY(QT_QTMAIN_LIBRARY_RELEASE NAMES qtmain${QT_LIBINFIX} PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    FIND_LIBRARY(QT_QTMAIN_LIBRARY_DEBUG NAMES qtmain${QT_LIBINFIX}d PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  ENDIF(Q_WS_WIN)
  
  # Set QT_QTASSISTANTCLIENT_LIBRARY
  FIND_LIBRARY(QT_QTASSISTANTCLIENT_LIBRARY_RELEASE NAMES QtAssistantClient${QT_LIBINFIX} QtAssistantClient${QT_LIBINFIX}4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  FIND_LIBRARY(QT_QTASSISTANTCLIENT_LIBRARY_DEBUG   NAMES QtAssistantClient${QT_LIBINFIX}_debug QtAssistantClient${QT_LIBINFIX}d QtAssistantClient${QT_LIBINFIX}d4 PATHS ${QT_LIBRARY_DIR}  NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  
  # Set QT_QTASSISTANT_LIBRARY
  FIND_LIBRARY(QT_QTASSISTANT_LIBRARY_RELEASE NAMES QtAssistantClient${QT_LIBINFIX} QtAssistantClient${QT_LIBINFIX}4 QtAssistant${QT_LIBINFIX} QtAssistant${QT_LIBINFIX}4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  FIND_LIBRARY(QT_QTASSISTANT_LIBRARY_DEBUG   NAMES QtAssistantClient${QT_LIBINFIX}_debug QtAssistantClient${QT_LIBINFIX}d QtAssistantClient${QT_LIBINFIX}d4 QtAssistant${QT_LIBINFIX}_debug QtAssistant${QT_LIBINFIX}d4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)

  # Set QT_QTHELP_LIBRARY
  FIND_LIBRARY(QT_QTCLUCENE_LIBRARY_RELEASE NAMES QtCLucene${QT_LIBINFIX} QtCLucene${QT_LIBINFIX}4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  FIND_LIBRARY(QT_QTCLUCENE_LIBRARY_DEBUG   NAMES QtCLucene${QT_LIBINFIX}_debug QtCLucene${QT_LIBINFIX}d QtCLucene${QT_LIBINFIX}d4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  IF(Q_WS_MAC AND QT_QTCORE_LIBRARY_RELEASE AND NOT QT_QTCLUCENE_LIBRARY_RELEASE)
    FIND_LIBRARY(QT_QTCLUCENE_LIBRARY_RELEASE NAMES QtCLucene${QT_LIBINFIX} PATHS ${QT_LIBRARY_DIR})
  ENDIF(Q_WS_MAC AND QT_QTCORE_LIBRARY_RELEASE AND NOT QT_QTCLUCENE_LIBRARY_RELEASE)


  ############################################
  #
  # Check the existence of the libraries.
  #
  ############################################


  # Set QT_xyz_LIBRARY variable and add 
  # library include path to QT_INCLUDES
  _QT4_ADJUST_LIB_VARS(QtCore)

  FOREACH(QT_MODULE ${QT_MODULES})
    _QT4_ADJUST_LIB_VARS(${QT_MODULE})
  ENDFOREACH(QT_MODULE)

  _QT4_ADJUST_LIB_VARS(QtAssistant)
  _QT4_ADJUST_LIB_VARS(QtAssistantClient)
  _QT4_ADJUST_LIB_VARS(QtCLucene)
  _QT4_ADJUST_LIB_VARS(QtDesignerComponents)

  # platform dependent libraries
  IF(Q_WS_WIN)
    _QT4_ADJUST_LIB_VARS(qtmain)
    _QT4_ADJUST_LIB_VARS(QAxServer)
    _QT4_ADJUST_LIB_VARS(QAxContainer)
  ENDIF(Q_WS_WIN)


  #######################################
  #
  #       Check the executables of Qt 
  #          ( moc, uic, rcc )
  #
  #######################################


  IF(QT_QMAKE_CHANGED)
    SET(QT_UIC_EXECUTABLE NOTFOUND)
    SET(QT_MOC_EXECUTABLE NOTFOUND)
    SET(QT_UIC3_EXECUTABLE NOTFOUND)
    SET(QT_RCC_EXECUTABLE NOTFOUND)
    SET(QT_DBUSCPP2XML_EXECUTABLE NOTFOUND)
    SET(QT_DBUSXML2CPP_EXECUTABLE NOTFOUND)
    SET(QT_LUPDATE_EXECUTABLE NOTFOUND)
    SET(QT_LRELEASE_EXECUTABLE NOTFOUND)
    SET(QT_QCOLLECTIONGENERATOR_EXECUTABLE NOTFOUND)
    SET(QT_DESIGNER_EXECUTABLE NOTFOUND)
    SET(QT_LINGUIST_EXECUTABLE NOTFOUND)
  ENDIF(QT_QMAKE_CHANGED)
  
  FIND_PROGRAM(QT_MOC_EXECUTABLE
    NAMES moc-qt4 moc
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  FIND_PROGRAM(QT_UIC_EXECUTABLE
    NAMES uic-qt4 uic
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  FIND_PROGRAM(QT_UIC3_EXECUTABLE
    NAMES uic3
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  FIND_PROGRAM(QT_RCC_EXECUTABLE 
    NAMES rcc
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  FIND_PROGRAM(QT_DBUSCPP2XML_EXECUTABLE 
    NAMES qdbuscpp2xml
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  FIND_PROGRAM(QT_DBUSXML2CPP_EXECUTABLE 
    NAMES qdbusxml2cpp
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  FIND_PROGRAM(QT_LUPDATE_EXECUTABLE
    NAMES lupdate-qt4 lupdate
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  FIND_PROGRAM(QT_LRELEASE_EXECUTABLE
    NAMES lrelease-qt4 lrelease
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  FIND_PROGRAM(QT_QCOLLECTIONGENERATOR_EXECUTABLE
    NAMES qcollectiongenerator-qt4 qcollectiongenerator
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  FIND_PROGRAM(QT_DESIGNER_EXECUTABLE
    NAMES designer-qt4 designer
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  FIND_PROGRAM(QT_LINGUIST_EXECUTABLE
    NAMES linguist-qt4 linguist
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  IF (QT_MOC_EXECUTABLE)
     SET(QT_WRAP_CPP "YES")
  ENDIF (QT_MOC_EXECUTABLE)

  IF (QT_UIC_EXECUTABLE)
     SET(QT_WRAP_UI "YES")
  ENDIF (QT_UIC_EXECUTABLE)



  MARK_AS_ADVANCED( QT_UIC_EXECUTABLE QT_UIC3_EXECUTABLE QT_MOC_EXECUTABLE
    QT_RCC_EXECUTABLE QT_DBUSXML2CPP_EXECUTABLE QT_DBUSCPP2XML_EXECUTABLE
    QT_LUPDATE_EXECUTABLE QT_LRELEASE_EXECUTABLE QT_QCOLLECTIONGENERATOR_EXECUTABLE
    QT_DESIGNER_EXECUTABLE QT_LINGUIST_EXECUTABLE)


  # get the directory of the current file, used later on in the file
  GET_FILENAME_COMPONENT( _qt4_current_dir  "${CMAKE_CURRENT_LIST_FILE}" PATH)

  
  ###############################################
  #
  #       configuration/system dependent settings  
  #
  ###############################################

  INCLUDE("${_qt4_current_dir}/Qt4ConfigDependentSettings.cmake")


  #######################################
  #
  #       Check the plugins of Qt
  #
  #######################################

  SET( QT_PLUGIN_TYPES accessible bearer codecs decorations designer gfxdrivers graphicssystems iconengines imageformats inputmethods mousedrivers phonon_backend script sqldrivers )

  SET( QT_ACCESSIBLE_PLUGINS qtaccessiblecompatwidgets qtaccessiblewidgets )
  SET( QT_BEARER_PLUGINS qcorewlanbearer qgenericbearer )
  SET( QT_CODECS_PLUGINS qcncodecs qjpcodecs qkrcodecs qtwcodecs )
  SET( QT_DECORATIONS_PLUGINS qdecorationdefault qdecorationwindows )
  SET( QT_DESIGNER_PLUGINS arthurplugin containerextension customwidgetplugin phononwidgets qdeclarativeview qt3supportwidgets qwebview taskmenuextension worldtimeclockplugin )
  SET( QT_GRAPHICSDRIVERS_PLUGINS qgfxtransformed qgfxvnc qscreenvfb )
  SET( QT_GRAPHICSSYSTEMS_PLUGINS qglgraphicssystem qtracegraphicssystem )
  SET( QT_ICONENGINES_PLUGINS qsvgicon )
  SET( QT_IMAGEFORMATS_PLUGINS qgif qjpeg qmng qico qsvg qtiff  )
  SET( QT_INPUTMETHODS_PLUGINS qimsw_multi )
  SET( QT_MOUSEDRIVERS_PLUGINS qwstslibmousehandler )
  SET( QT_PHONON_BACKEND_PLUGINS phonon_qt7 )
  SET( QT_SCRIPT_PLUGINS qtscriptdbus )
  SET( QT_SQLDRIVERS_PLUGINS qsqldb2 qsqlibase qsqlite qsqlite2 qsqlmysql qsqloci qsqlodbc qsqlpsql qsqltds )

  SET( QT_PHONON_PLUGINS ${QT_PHONON_BACKEND_PLUGINS} )
  SET( QT_QT3SUPPORT_PLUGINS qtaccessiblecompatwidgets )
  SET( QT_QTCORE_PLUGINS ${QT_BEARER_PLUGINS} ${QT_CODECS_PLUGINS} )
  SET( QT_QTGUI_PLUGINS qtaccessiblewidgets qgif qjpeg qmng qico qtiff ${QT_DECORATIONS_PLUGINS} ${QT_GRAPHICSDRIVERS_PLUGINS} ${QT_GRAPHICSSYSTEMS_PLUGINS} ${QT_INPUTMETHODS_PLUGINS} ${QT_MOUSEDRIVERS_PLUGINS} )
  SET( QT_QTSCRIPT_PLUGINS ${QT_SCRIPT_PLUGINS} )
  SET( QT_QTSQL_PLUGINS ${QT_SQLDRIVERS_PLUGINS} )
  SET( QT_QTSVG_PLUGINS qsvg qsvgicon )

  IF(QT_QMAKE_CHANGED)
    FOREACH(QT_PLUGIN_TYPE ${QT_PLUGIN_TYPES})
      STRING(TOUPPER ${QT_PLUGIN_TYPE} _upper_qt_plugin_type)
      SET(QT_${_upper_qt_plugin_type}_PLUGINS_DIR ${QT_PLUGINS_DIR}/${QT_PLUGIN_TYPE})
      FOREACH(QT_PLUGIN ${QT_${_upper_qt_plugin_type}_PLUGINS})
        STRING(TOUPPER ${QT_PLUGIN} _upper_qt_plugin)
        UNSET(QT_${_upper_qt_plugin}_LIBRARY_RELEASE CACHE)
        UNSET(QT_${_upper_qt_plugin}_LIBRARY_DEBUG CACHE)
        UNSET(QT_${_upper_qt_plugin}_LIBRARY CACHE)
        UNSET(QT_${_upper_qt_plugin}_PLUGIN_RELEASE CACHE)
        UNSET(QT_${_upper_qt_plugin}_PLUGIN_DEBUG CACHE)
        UNSET(QT_${_upper_qt_plugin}_PLUGIN CACHE)
      ENDFOREACH(QT_PLUGIN)
    ENDFOREACH(QT_PLUGIN_TYPE)
  ENDIF(QT_QMAKE_CHANGED)

  # find_library works better than find_file but we need to set prefixes to only match plugins
  FOREACH(QT_PLUGIN_TYPE ${QT_PLUGIN_TYPES})
    STRING(TOUPPER ${QT_PLUGIN_TYPE} _upper_qt_plugin_type)
    SET(QT_${_upper_qt_plugin_type}_PLUGINS_DIR ${QT_PLUGINS_DIR}/${QT_PLUGIN_TYPE})
    FOREACH(QT_PLUGIN ${QT_${_upper_qt_plugin_type}_PLUGINS})
      STRING(TOUPPER ${QT_PLUGIN} _upper_qt_plugin)
      IF(QT_IS_STATIC)
        FIND_LIBRARY(QT_${_upper_qt_plugin}_LIBRARY_RELEASE
                     NAMES ${QT_PLUGIN}${QT_LIBINFIX} ${QT_PLUGIN}${QT_LIBINFIX}4
                     PATHS ${QT_${_upper_qt_plugin_type}_PLUGINS_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
            )
        FIND_LIBRARY(QT_${_upper_qt_plugin}_LIBRARY_DEBUG
                     NAMES ${QT_PLUGIN}${QT_LIBINFIX}_debug ${QT_PLUGIN}${QT_LIBINFIX}d ${QT_PLUGIN}${QT_LIBINFIX}d4
                     PATHS ${QT_${_upper_qt_plugin_type}_PLUGINS_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
            )
        _QT4_ADJUST_LIB_VARS(${QT_PLUGIN})
      ELSE(QT_IS_STATIC)
        # find_library works easier/better than find_file but we need to set suffixes to only match plugins
        SET(CMAKE_FIND_LIBRARY_SUFFIXES_DEFAULT ${CMAKE_FIND_LIBRARY_SUFFIXES})
        SET(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_SHARED_MODULE_SUFFIX} ${CMAKE_SHARED_LIBRARY_SUFFIX})
        FIND_LIBRARY(QT_${_upper_qt_plugin}_PLUGIN_RELEASE
                     NAMES ${QT_PLUGIN}${QT_LIBINFIX} ${QT_PLUGIN}${QT_LIBINFIX}4
                     PATHS ${QT_${_upper_qt_plugin_type}_PLUGINS_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
            )
        FIND_LIBRARY(QT_${_upper_qt_plugin}_PLUGIN_DEBUG
                     NAMES ${QT_PLUGIN}${QT_LIBINFIX}_debug ${QT_PLUGIN}${QT_LIBINFIX}d ${QT_PLUGIN}${QT_LIBINFIX}d4
                     PATHS ${QT_${_upper_qt_plugin_type}_PLUGINS_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
            )
        MARK_AS_ADVANCED(QT_${_upper_qt_plugin}_PLUGIN_RELEASE QT_${_upper_qt_plugin}_PLUGIN_DEBUG)
        SET(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_DEFAULT})
      ENDIF(QT_IS_STATIC)
    ENDFOREACH(QT_PLUGIN)
  ENDFOREACH(QT_PLUGIN_TYPE)


  ######################################
  #
  #       Macros for building Qt files
  #
  ######################################

  INCLUDE("${_qt4_current_dir}/Qt4Macros.cmake")

  # set version variables
  STRING(REGEX REPLACE "^([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1" QT_VERSION_MAJOR "${QTVERSION}")
  STRING(REGEX REPLACE "^[0-9]+\\.([0-9])+\\.[0-9]+.*" "\\1" QT_VERSION_MINOR "${QTVERSION}")
  STRING(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" QT_VERSION_PATCH "${QTVERSION}")

ENDIF(QT_QMAKE_EXECUTABLE AND QTVERSION)

#support old QT_MIN_VERSION if set, but not if version is supplied by find_package()
IF(NOT Qt4_FIND_VERSION AND QT_MIN_VERSION)
  SET(Qt4_FIND_VERSION ${QT_MIN_VERSION})
ENDIF(NOT Qt4_FIND_VERSION AND QT_MIN_VERSION)

IF( Qt4_FIND_COMPONENTS )

  # if components specified in find_package(), make sure each of those pieces were found
  SET(_QT4_FOUND_REQUIRED_VARS QT_QMAKE_EXECUTABLE QT_MOC_EXECUTABLE QT_RCC_EXECUTABLE QT_INCLUDE_DIR QT_LIBRARY_DIR)
  FOREACH( component ${Qt4_FIND_COMPONENTS} )
    STRING( TOUPPER ${component} _COMPONENT )
    if(${_COMPONENT} STREQUAL "QTMAIN")
      IF(Q_WS_WIN)
        SET(_QT4_FOUND_REQUIRED_VARS ${_QT4_FOUND_REQUIRED_VARS} QT_${_COMPONENT}_LIBRARY)
      ENDIF(Q_WS_WIN)
    else(${_COMPONENT} STREQUAL "QTMAIN")
      SET(_QT4_FOUND_REQUIRED_VARS ${_QT4_FOUND_REQUIRED_VARS} QT_${_COMPONENT}_INCLUDE_DIR QT_${_COMPONENT}_LIBRARY)
    endif(${_COMPONENT} STREQUAL "QTMAIN")
  ENDFOREACH( component )

  if(Qt4_FIND_COMPONENTS MATCHES QtGui)
    SET(_QT4_FOUND_REQUIRED_VARS ${_QT4_FOUND_REQUIRED_VARS} QT_UIC_EXECUTABLE)
  endif(Qt4_FIND_COMPONENTS MATCHES QtGui)

ELSE( Qt4_FIND_COMPONENTS )

  # if no components specified, we'll make a default set of required variables to say Qt is found
  SET(_QT4_FOUND_REQUIRED_VARS QT_QMAKE_EXECUTABLE QT_MOC_EXECUTABLE QT_RCC_EXECUTABLE QT_UIC_EXECUTABLE QT_INCLUDE_DIR
    QT_LIBRARY_DIR QT_QTCORE_LIBRARY)

ENDIF( Qt4_FIND_COMPONENTS )

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Qt4
  REQUIRED_VARS ${_QT4_FOUND_REQUIRED_VARS}
  VERSION_VAR QTVERSION
  )

#######################################
#
#       compatibility settings
#
#######################################
# Backwards compatibility for CMake1.4 and 1.2
SET (QT_MOC_EXE ${QT_MOC_EXECUTABLE} )
SET (QT_UIC_EXE ${QT_UIC_EXECUTABLE} )
SET( QT_QT_LIBRARY "")
SET(QT_FOUND ${QT4_FOUND})

