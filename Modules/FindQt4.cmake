# - Find Qt 4
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
#       Note that if using IMPORTED targets, the qtmain.lib static library is
#       automatically linked on Windows. To disable that globally, set the
#       QT4_NO_LINK_QTMAIN variable before finding Qt4. To disable that for a
#       particular executable, set the QT4_NO_LINK_QTMAIN target property to
#       True on the executable.
#
#  QT_INCLUDE_DIRS_NO_SYSTEM
#        If this variable is set to TRUE, the Qt include directories
#        in the QT_USE_FILE will NOT have the SYSTEM keyword set.
#
# There are also some files that need processing by some Qt tools such as moc
# and uic.  Listed below are macros that may be used to process those files.
#
#  macro QT4_WRAP_CPP(outfiles inputfile ... OPTIONS ...)
#        create moc code from a list of files containing Qt class with
#        the Q_OBJECT declaration.  Per-directory preprocessor definitions
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
#        You should have a look on the AUTOMOC property for targets to achieve the same results.
#
#  macro QT4_ADD_DBUS_INTERFACE(outfiles interface basename)
#        Create a the interface header and implementation files with the
#        given basename from the given interface xml file and add it to
#        the list of sources.
#
#        You can pass additional parameters to the qdbusxml2cpp call by setting
#        properties on the input file:
#
#        INCLUDE the given file will be included in the generate interface header
#
#        CLASSNAME the generated class is named accordingly
#
#        NO_NAMESPACE the generated class is not wrapped in a namespace
#
#  macro QT4_ADD_DBUS_INTERFACES(outfiles inputfile ... )
#        Create the interface header and implementation files
#        for all listed interface xml files.
#        The basename will be automatically determined from the name of the xml file.
#
#        The source file properties described for QT4_ADD_DBUS_INTERFACE also apply here.
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
# function QT4_USE_MODULES( target [link_type] modules...)
#        Make <target> use the <modules> from Qt. Using a Qt module means
#        to link to the library, add the relevant include directories for the module,
#        and add the relevant compiler defines for using the module.
#        Modules are roughly equivalent to components of Qt4, so usage would be
#        something like:
#         qt4_use_modules(myexe Core Gui Declarative)
#        to use QtCore, QtGui and QtDeclarative. The optional <link_type> argument can
#        be specified as either LINK_PUBLIC or LINK_PRIVATE to specify the same argument
#        to the target_link_libraries call.
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

# Use find_package( Qt4 COMPONENTS ... ) to enable modules
if( Qt4_FIND_COMPONENTS )
  foreach( component ${Qt4_FIND_COMPONENTS} )
    string( TOUPPER ${component} _COMPONENT )
    set( QT_USE_${_COMPONENT} 1 )
  endforeach()

  # To make sure we don't use QtCore or QtGui when not in COMPONENTS
  if(NOT QT_USE_QTCORE)
    set( QT_DONT_USE_QTCORE 1 )
  endif()

  if(NOT QT_USE_QTGUI)
    set( QT_DONT_USE_QTGUI 1 )
  endif()

endif()

# If Qt3 has already been found, fail.
if(QT_QT_LIBRARY)
  if(Qt4_FIND_REQUIRED)
    message( FATAL_ERROR "Qt3 and Qt4 cannot be used together in one project.  If switching to Qt4, the CMakeCache.txt needs to be cleaned.")
  else()
    if(NOT Qt4_FIND_QUIETLY)
      message( STATUS    "Qt3 and Qt4 cannot be used together in one project.  If switching to Qt4, the CMakeCache.txt needs to be cleaned.")
    endif()
    return()
  endif()
endif()


include(${CMAKE_CURRENT_LIST_DIR}/CheckCXXSymbolExists.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/MacroAddFileDependencies.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)

set(QT_USE_FILE ${CMAKE_ROOT}/Modules/UseQt4.cmake)

set( QT_DEFINITIONS "")

# convenience macro for dealing with debug/release library names
macro (_QT4_ADJUST_LIB_VARS _camelCaseBasename)

  string(TOUPPER "${_camelCaseBasename}" basename)

  # The name of the imported targets, i.e. the prefix "Qt4::" must not change,
  # since it is stored in EXPORT-files as name of a required library. If the name would change
  # here, this would lead to the imported Qt4-library targets not being resolved by cmake anymore.
  if (QT_${basename}_LIBRARY_RELEASE OR QT_${basename}_LIBRARY_DEBUG)

    if(NOT TARGET Qt4::${_camelCaseBasename})
      add_library(Qt4::${_camelCaseBasename} UNKNOWN IMPORTED )

      if (QT_${basename}_LIBRARY_RELEASE)
        set_property(TARGET Qt4::${_camelCaseBasename} APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
        if(QT_USE_FRAMEWORKS)
          set_property(TARGET Qt4::${_camelCaseBasename}        PROPERTY IMPORTED_LOCATION_RELEASE "${QT_${basename}_LIBRARY_RELEASE}/${_camelCaseBasename}" )
        else()
          set_property(TARGET Qt4::${_camelCaseBasename}        PROPERTY IMPORTED_LOCATION_RELEASE "${QT_${basename}_LIBRARY_RELEASE}" )
        endif()
      endif ()

      if (QT_${basename}_LIBRARY_DEBUG)
        set_property(TARGET Qt4::${_camelCaseBasename} APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
        if(QT_USE_FRAMEWORKS)
          set_property(TARGET Qt4::${_camelCaseBasename}        PROPERTY IMPORTED_LOCATION_DEBUG "${QT_${basename}_LIBRARY_DEBUG}/${_camelCaseBasename}" )
        else()
          set_property(TARGET Qt4::${_camelCaseBasename}        PROPERTY IMPORTED_LOCATION_DEBUG "${QT_${basename}_LIBRARY_DEBUG}" )
        endif()
      endif ()
      set_property(TARGET Qt4::${_camelCaseBasename} PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES
          "${QT_${basename}_INCLUDE_DIR}"
      )
      string(REGEX REPLACE "^QT" "" _stemname ${basename})
      set_property(TARGET Qt4::${_camelCaseBasename} PROPERTY
        INTERFACE_COMPILE_DEFINITIONS
          "QT_${_stemname}_LIB"
      )
    endif()

    # If QT_USE_IMPORTED_TARGETS is enabled, the QT_QTFOO_LIBRARY variables are set to point at these
    # imported targets. This works better in general, and is also in almost all cases fully
    # backward compatible. The only issue is when a project A which had this enabled then exports its
    # libraries via export or export_library_dependencies(). In this case the libraries from project
    # A will depend on the imported Qt targets, and the names of these imported targets will be stored
    # in the dependency files on disk. This means when a project B then uses project A, these imported
    # targets must be created again, otherwise e.g. "Qt4__QtCore" will be interpreted as name of a
    # library file on disk, and not as a target, and linking will fail:
    if(QT_USE_IMPORTED_TARGETS)
        set(QT_${basename}_LIBRARY       Qt4::${_camelCaseBasename} )
        set(QT_${basename}_LIBRARIES     Qt4::${_camelCaseBasename} )
    else()

      # if the release- as well as the debug-version of the library have been found:
      if (QT_${basename}_LIBRARY_DEBUG AND QT_${basename}_LIBRARY_RELEASE)
        # if the generator supports configuration types then set
        # optimized and debug libraries, or if the CMAKE_BUILD_TYPE has a value
        if (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
          set(QT_${basename}_LIBRARY       optimized ${QT_${basename}_LIBRARY_RELEASE} debug ${QT_${basename}_LIBRARY_DEBUG})
        else()
          # if there are no configuration types and CMAKE_BUILD_TYPE has no value
          # then just use the release libraries
          set(QT_${basename}_LIBRARY       ${QT_${basename}_LIBRARY_RELEASE} )
        endif()
        set(QT_${basename}_LIBRARIES       optimized ${QT_${basename}_LIBRARY_RELEASE} debug ${QT_${basename}_LIBRARY_DEBUG})
      endif ()

      # if only the release version was found, set the debug variable also to the release version
      if (QT_${basename}_LIBRARY_RELEASE AND NOT QT_${basename}_LIBRARY_DEBUG)
        set(QT_${basename}_LIBRARY_DEBUG ${QT_${basename}_LIBRARY_RELEASE})
        set(QT_${basename}_LIBRARY       ${QT_${basename}_LIBRARY_RELEASE})
        set(QT_${basename}_LIBRARIES     ${QT_${basename}_LIBRARY_RELEASE})
      endif ()

      # if only the debug version was found, set the release variable also to the debug version
      if (QT_${basename}_LIBRARY_DEBUG AND NOT QT_${basename}_LIBRARY_RELEASE)
        set(QT_${basename}_LIBRARY_RELEASE ${QT_${basename}_LIBRARY_DEBUG})
        set(QT_${basename}_LIBRARY         ${QT_${basename}_LIBRARY_DEBUG})
        set(QT_${basename}_LIBRARIES       ${QT_${basename}_LIBRARY_DEBUG})
      endif ()

      # put the value in the cache:
      set(QT_${basename}_LIBRARY ${QT_${basename}_LIBRARY} CACHE STRING "The Qt ${basename} library" FORCE)

    endif()

    set(QT_${basename}_FOUND 1)

  else ()

    set(QT_${basename}_LIBRARY "" CACHE STRING "The Qt ${basename} library" FORCE)

  endif ()

  if (QT_${basename}_INCLUDE_DIR)
    #add the include directory to QT_INCLUDES
    set(QT_INCLUDES "${QT_${basename}_INCLUDE_DIR}" ${QT_INCLUDES})
  endif ()

  # Make variables changeble to the advanced user
  mark_as_advanced(QT_${basename}_LIBRARY QT_${basename}_LIBRARY_RELEASE QT_${basename}_LIBRARY_DEBUG QT_${basename}_INCLUDE_DIR)
endmacro ()

function(_QT4_QUERY_QMAKE VAR RESULT)
  execute_process(COMMAND "${QT_QMAKE_EXECUTABLE}" -query ${VAR}
    RESULT_VARIABLE return_code
    OUTPUT_VARIABLE output
    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE)
  if(NOT return_code)
    file(TO_CMAKE_PATH "${output}" output)
    set(${RESULT} ${output} PARENT_SCOPE)
  endif()
endfunction()


set(QT4_INSTALLED_VERSION_TOO_OLD FALSE)

get_filename_component(qt_install_version "[HKEY_CURRENT_USER\\Software\\trolltech\\Versions;DefaultQtVersion]" NAME)
# check for qmake
# Debian uses qmake-qt4
# macports' Qt uses qmake-mac
find_program(QT_QMAKE_EXECUTABLE NAMES qmake qmake4 qmake-qt4 qmake-mac
  PATHS
    ENV QTDIR
    "[HKEY_CURRENT_USER\\Software\\Trolltech\\Versions\\${qt_install_version};InstallDir]"
  PATH_SUFFIXES bin
  DOC "The qmake executable for the Qt installation to use"
)

# double check that it was a Qt4 qmake, if not, re-find with different names
if (QT_QMAKE_EXECUTABLE)

  if(QT_QMAKE_EXECUTABLE_LAST)
    string(COMPARE NOTEQUAL "${QT_QMAKE_EXECUTABLE_LAST}" "${QT_QMAKE_EXECUTABLE}" QT_QMAKE_CHANGED)
  endif()

  set(QT_QMAKE_EXECUTABLE_LAST "${QT_QMAKE_EXECUTABLE}" CACHE INTERNAL "" FORCE)

  _qt4_query_qmake(QT_VERSION QTVERSION)

  # check for qt3 qmake and then try and find qmake4 or qmake-qt4 in the path
  if(NOT QTVERSION)
    set(QT_QMAKE_EXECUTABLE NOTFOUND CACHE FILEPATH "" FORCE)
    find_program(QT_QMAKE_EXECUTABLE NAMES qmake4 qmake-qt4 PATHS
      "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\4.0.0;InstallDir]/bin"
      "[HKEY_CURRENT_USER\\Software\\Trolltech\\Versions\\4.0.0;InstallDir]/bin"
      $ENV{QTDIR}/bin
      DOC "The qmake executable for the Qt installation to use"
      )
    if(QT_QMAKE_EXECUTABLE)
      _qt4_query_qmake(QT_VERSION QTVERSION)
    endif()
  endif()

endif ()

if (QT_QMAKE_EXECUTABLE AND QTVERSION)

  # set version variables
  string(REGEX REPLACE "^([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1" QT_VERSION_MAJOR "${QTVERSION}")
  string(REGEX REPLACE "^[0-9]+\\.([0-9]+)\\.[0-9]+.*" "\\1" QT_VERSION_MINOR "${QTVERSION}")
  string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" QT_VERSION_PATCH "${QTVERSION}")

  # ask qmake for the mkspecs directory
  # we do this first because QT_LIBINFIX might be set
  if (NOT QT_MKSPECS_DIR  OR  QT_QMAKE_CHANGED)
    _qt4_query_qmake(QMAKE_MKSPECS qt_mkspecs_dirs)
    # do not replace : on windows as it might be a drive letter
    # and windows should already use ; as a separator
    if(NOT WIN32)
      string(REPLACE ":" ";" qt_mkspecs_dirs "${qt_mkspecs_dirs}")
    endif()
    set(qt_cross_paths)
    foreach(qt_cross_path ${CMAKE_FIND_ROOT_PATH})
      set(qt_cross_paths ${qt_cross_paths} "${qt_cross_path}/mkspecs")
    endforeach()
    set(QT_MKSPECS_DIR NOTFOUND)
    find_path(QT_MKSPECS_DIR NAMES qconfig.pri
      HINTS ${qt_cross_paths} ${qt_mkspecs_dirs}
      DOC "The location of the Qt mkspecs containing qconfig.pri")
  endif()

  if(EXISTS "${QT_MKSPECS_DIR}/qconfig.pri")
    file(READ ${QT_MKSPECS_DIR}/qconfig.pri _qconfig_FILE_contents)
    string(REGEX MATCH "QT_CONFIG[^\n]+" QT_QCONFIG "${_qconfig_FILE_contents}")
    string(REGEX MATCH "CONFIG[^\n]+" QT_CONFIG "${_qconfig_FILE_contents}")
    string(REGEX MATCH "EDITION[^\n]+" QT_EDITION "${_qconfig_FILE_contents}")
    string(REGEX MATCH "QT_LIBINFIX[^\n]+" _qconfig_qt_libinfix "${_qconfig_FILE_contents}")
    string(REGEX REPLACE "QT_LIBINFIX *= *([^\n]*)" "\\1" QT_LIBINFIX "${_qconfig_qt_libinfix}")
  endif()
  if("${QT_EDITION}" MATCHES "DesktopLight")
    set(QT_EDITION_DESKTOPLIGHT 1)
  endif()

  # ask qmake for the library dir as a hint, then search for QtCore library and use that as a reference for finding the
  # others and for setting QT_LIBRARY_DIR
  if (NOT (QT_QTCORE_LIBRARY_RELEASE OR QT_QTCORE_LIBRARY_DEBUG)  OR QT_QMAKE_CHANGED)
    _qt4_query_qmake(QT_INSTALL_LIBS QT_LIBRARY_DIR_TMP)
    set(QT_QTCORE_LIBRARY_RELEASE NOTFOUND)
    set(QT_QTCORE_LIBRARY_DEBUG NOTFOUND)
    find_library(QT_QTCORE_LIBRARY_RELEASE
                 NAMES QtCore${QT_LIBINFIX} QtCore${QT_LIBINFIX}4
                 HINTS ${QT_LIBRARY_DIR_TMP}
                 NO_DEFAULT_PATH
        )
    find_library(QT_QTCORE_LIBRARY_DEBUG
                 NAMES QtCore${QT_LIBINFIX}_debug QtCore${QT_LIBINFIX}d QtCore${QT_LIBINFIX}d4
                 HINTS ${QT_LIBRARY_DIR_TMP}
                 NO_DEFAULT_PATH
        )

    if(NOT QT_QTCORE_LIBRARY_RELEASE AND NOT QT_QTCORE_LIBRARY_DEBUG)
      find_library(QT_QTCORE_LIBRARY_RELEASE
                   NAMES QtCore${QT_LIBINFIX} QtCore${QT_LIBINFIX}4
                   HINTS ${QT_LIBRARY_DIR_TMP}
          )
      find_library(QT_QTCORE_LIBRARY_DEBUG
                   NAMES QtCore${QT_LIBINFIX}_debug QtCore${QT_LIBINFIX}d QtCore${QT_LIBINFIX}d4
                   HINTS ${QT_LIBRARY_DIR_TMP}
          )
    endif()

    # try dropping a hint if trying to use Visual Studio with Qt built by mingw
    if(NOT QT_QTCORE_LIBRARY_RELEASE AND MSVC)
      if(EXISTS ${QT_LIBRARY_DIR_TMP}/libqtmain.a)
        message( FATAL_ERROR "It appears you're trying to use Visual Studio with Qt built by mingw.  Those compilers do not produce code compatible with each other.")
      endif()
    endif()

  endif ()

  # set QT_LIBRARY_DIR based on location of QtCore found.
  if(QT_QTCORE_LIBRARY_RELEASE)
    get_filename_component(QT_LIBRARY_DIR_TMP "${QT_QTCORE_LIBRARY_RELEASE}" PATH)
    set(QT_LIBRARY_DIR ${QT_LIBRARY_DIR_TMP} CACHE INTERNAL "Qt library dir" FORCE)
    set(QT_QTCORE_FOUND 1)
  elseif(QT_QTCORE_LIBRARY_DEBUG)
    get_filename_component(QT_LIBRARY_DIR_TMP "${QT_QTCORE_LIBRARY_DEBUG}" PATH)
    set(QT_LIBRARY_DIR ${QT_LIBRARY_DIR_TMP} CACHE INTERNAL "Qt library dir" FORCE)
    set(QT_QTCORE_FOUND 1)
  else()
    message(WARNING "${QT_QMAKE_EXECUTABLE} reported QT_INSTALL_LIBS as \"${QT_LIBRARY_DIR_TMP}\" "
                    "but QtCore could not be found there.  "
                    "Qt is NOT installed correctly for the target build environment.")
    if(Qt4_FIND_REQUIRED)
      message( FATAL_ERROR "Could NOT find QtCore. Check ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log for more details.")
    endif()
  endif()

  # ask qmake for the binary dir
  if (NOT QT_BINARY_DIR  OR  QT_QMAKE_CHANGED)
    _qt4_query_qmake(QT_INSTALL_BINS qt_bins)
    set(QT_BINARY_DIR ${qt_bins} CACHE INTERNAL "" FORCE)
  endif ()

  if (APPLE)
    set(CMAKE_FIND_FRAMEWORK_OLD ${CMAKE_FIND_FRAMEWORK})
    if (EXISTS ${QT_LIBRARY_DIR}/QtCore.framework)
      set(QT_USE_FRAMEWORKS ON CACHE INTERNAL "" FORCE)
      set(CMAKE_FIND_FRAMEWORK FIRST)
    else ()
      set(QT_USE_FRAMEWORKS OFF CACHE INTERNAL "" FORCE)
      set(CMAKE_FIND_FRAMEWORK LAST)
    endif ()
  endif ()

  # ask qmake for the include dir
  if (QT_LIBRARY_DIR AND (NOT QT_QTCORE_INCLUDE_DIR OR NOT QT_HEADERS_DIR OR  QT_QMAKE_CHANGED))
      _qt4_query_qmake(QT_INSTALL_HEADERS qt_headers)
      set(QT_QTCORE_INCLUDE_DIR NOTFOUND)
      find_path(QT_QTCORE_INCLUDE_DIR QtCore
                HINTS ${qt_headers} ${QT_LIBRARY_DIR}
                PATH_SUFFIXES QtCore qt4/QtCore
        )

      # Set QT_HEADERS_DIR based on finding QtCore header
      if(QT_QTCORE_INCLUDE_DIR)
        if(QT_USE_FRAMEWORKS)
          set(QT_HEADERS_DIR "${qt_headers}" CACHE INTERNAL "" FORCE)
        else()
          get_filename_component(qt_headers "${QT_QTCORE_INCLUDE_DIR}/../" ABSOLUTE)
          set(QT_HEADERS_DIR "${qt_headers}" CACHE INTERNAL "" FORCE)
        endif()
      elseif()
        message("Warning: QT_QMAKE_EXECUTABLE reported QT_INSTALL_HEADERS as ${qt_headers}")
        message("Warning: But QtCore couldn't be found.  Qt must NOT be installed correctly.")
      endif()
  endif()

  if(APPLE)
    set(CMAKE_FIND_FRAMEWORK ${CMAKE_FIND_FRAMEWORK_OLD})
  endif()

  # Set QT_INCLUDE_DIR based on QT_HEADERS_DIR
  if(QT_HEADERS_DIR)
    if(QT_USE_FRAMEWORKS)
      # Qt/Mac frameworks has two include dirs.
      # One is the framework include for which CMake will add a -F flag
      # and the other is an include dir for non-framework Qt modules
      set(QT_INCLUDE_DIR ${QT_HEADERS_DIR} ${QT_QTCORE_LIBRARY_RELEASE} )
    else()
      set(QT_INCLUDE_DIR ${QT_HEADERS_DIR})
    endif()
  endif()

  # Set QT_INCLUDES
  set( QT_INCLUDES ${QT_MKSPECS_DIR}/default ${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR})


  # ask qmake for the documentation directory
  if (QT_LIBRARY_DIR AND NOT QT_DOC_DIR  OR  QT_QMAKE_CHANGED)
    _qt4_query_qmake(QT_INSTALL_DOCS qt_doc_dir)
    set(QT_DOC_DIR ${qt_doc_dir} CACHE PATH "The location of the Qt docs" FORCE)
  endif ()


  # ask qmake for the plugins directory
  if (QT_LIBRARY_DIR AND NOT QT_PLUGINS_DIR  OR  QT_QMAKE_CHANGED)
    _qt4_query_qmake(QT_INSTALL_PLUGINS qt_plugins_dir)
    set(QT_PLUGINS_DIR NOTFOUND)
    foreach(qt_cross_path ${CMAKE_FIND_ROOT_PATH})
      set(qt_cross_paths ${qt_cross_paths} "${qt_cross_path}/plugins")
    endforeach()
    find_path(QT_PLUGINS_DIR NAMES accessible imageformats sqldrivers codecs designer
      HINTS ${qt_cross_paths} ${qt_plugins_dir}
      DOC "The location of the Qt plugins")
  endif ()

  # ask qmake for the translations directory
  if (QT_LIBRARY_DIR AND NOT QT_TRANSLATIONS_DIR  OR  QT_QMAKE_CHANGED)
    _qt4_query_qmake(QT_INSTALL_TRANSLATIONS qt_translations_dir)
    set(QT_TRANSLATIONS_DIR ${qt_translations_dir} CACHE PATH "The location of the Qt translations" FORCE)
  endif ()

  # ask qmake for the imports directory
  if (QT_LIBRARY_DIR AND NOT QT_IMPORTS_DIR OR QT_QMAKE_CHANGED)
    _qt4_query_qmake(QT_INSTALL_IMPORTS qt_imports_dir)
    if(qt_imports_dir)
      set(QT_IMPORTS_DIR NOTFOUND)
      foreach(qt_cross_path ${CMAKE_FIND_ROOT_PATH})
        set(qt_cross_paths ${qt_cross_paths} "${qt_cross_path}/imports")
      endforeach()
      find_path(QT_IMPORTS_DIR NAMES Qt
        HINTS ${qt_cross_paths} ${qt_imports_dir}
        DOC "The location of the Qt imports"
        NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_SYSTEM_ENVIRONMENT_PATH
        NO_CMAKE_SYSTEM_PATH)
      mark_as_advanced(QT_IMPORTS_DIR)
    endif()
  endif ()

  # Make variables changeble to the advanced user
  mark_as_advanced( QT_LIBRARY_DIR QT_DOC_DIR QT_MKSPECS_DIR
                    QT_PLUGINS_DIR QT_TRANSLATIONS_DIR)




  #############################################
  #
  # Find out what window system we're using
  #
  #############################################
  # Save required variable
  set(CMAKE_REQUIRED_INCLUDES_SAVE ${CMAKE_REQUIRED_INCLUDES})
  set(CMAKE_REQUIRED_FLAGS_SAVE    ${CMAKE_REQUIRED_FLAGS})
  # Add QT_INCLUDE_DIR to CMAKE_REQUIRED_INCLUDES
  set(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES};${QT_INCLUDE_DIR}")
  # Check for Window system symbols (note: only one should end up being set)
  CHECK_CXX_SYMBOL_EXISTS(Q_WS_X11 "QtCore/qglobal.h" Q_WS_X11)
  CHECK_CXX_SYMBOL_EXISTS(Q_WS_WIN "QtCore/qglobal.h" Q_WS_WIN)
  CHECK_CXX_SYMBOL_EXISTS(Q_WS_QWS "QtCore/qglobal.h" Q_WS_QWS)
  CHECK_CXX_SYMBOL_EXISTS(Q_WS_MAC "QtCore/qglobal.h" Q_WS_MAC)
  if(Q_WS_MAC)
    if(QT_QMAKE_CHANGED)
      unset(QT_MAC_USE_COCOA CACHE)
    endif()
    CHECK_CXX_SYMBOL_EXISTS(QT_MAC_USE_COCOA "QtCore/qconfig.h" QT_MAC_USE_COCOA)
  endif()

  if (QT_QTCOPY_REQUIRED)
     CHECK_CXX_SYMBOL_EXISTS(QT_IS_QTCOPY "QtCore/qglobal.h" QT_KDE_QT_COPY)
     if (NOT QT_IS_QTCOPY)
        message(FATAL_ERROR "qt-copy is required, but hasn't been found")
     endif ()
  endif ()

  # Restore CMAKE_REQUIRED_INCLUDES and CMAKE_REQUIRED_FLAGS variables
  set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES_SAVE})
  set(CMAKE_REQUIRED_FLAGS    ${CMAKE_REQUIRED_FLAGS_SAVE})
  #
  #############################################



  ########################################
  #
  #       Setting the INCLUDE-Variables
  #
  ########################################

  set(QT_MODULES QtGui Qt3Support QtSvg QtScript QtTest QtUiTools
                 QtHelp QtWebKit QtXmlPatterns phonon QtNetwork QtMultimedia
                 QtNsPlugin QtOpenGL QtSql QtXml QtDesigner QtDBus QtScriptTools
                 QtDeclarative)

  if(Q_WS_X11)
    set(QT_MODULES ${QT_MODULES} QtMotif)
  endif()

  if(QT_QMAKE_CHANGED)
    foreach(QT_MODULE ${QT_MODULES})
      string(TOUPPER ${QT_MODULE} _upper_qt_module)
      set(QT_${_upper_qt_module}_INCLUDE_DIR NOTFOUND)
      set(QT_${_upper_qt_module}_LIBRARY_RELEASE NOTFOUND)
      set(QT_${_upper_qt_module}_LIBRARY_DEBUG NOTFOUND)
    endforeach()
    set(QT_QTDESIGNERCOMPONENTS_INCLUDE_DIR NOTFOUND)
    set(QT_QTDESIGNERCOMPONENTS_LIBRARY_RELEASE NOTFOUND)
    set(QT_QTDESIGNERCOMPONENTS_LIBRARY_DEBUG NOTFOUND)
    set(QT_QTASSISTANTCLIENT_INCLUDE_DIR NOTFOUND)
    set(QT_QTASSISTANTCLIENT_LIBRARY_RELEASE NOTFOUND)
    set(QT_QTASSISTANTCLIENT_LIBRARY_DEBUG NOTFOUND)
    set(QT_QTASSISTANT_INCLUDE_DIR NOTFOUND)
    set(QT_QTASSISTANT_LIBRARY_RELEASE NOTFOUND)
    set(QT_QTASSISTANT_LIBRARY_DEBUG NOTFOUND)
    set(QT_QTCLUCENE_LIBRARY_RELEASE NOTFOUND)
    set(QT_QTCLUCENE_LIBRARY_DEBUG NOTFOUND)
    set(QT_QAXCONTAINER_INCLUDE_DIR NOTFOUND)
    set(QT_QAXCONTAINER_LIBRARY_RELEASE NOTFOUND)
    set(QT_QAXCONTAINER_LIBRARY_DEBUG NOTFOUND)
    set(QT_QAXSERVER_INCLUDE_DIR NOTFOUND)
    set(QT_QAXSERVER_LIBRARY_RELEASE NOTFOUND)
    set(QT_QAXSERVER_LIBRARY_DEBUG NOTFOUND)
    if(Q_WS_WIN)
      set(QT_QTMAIN_LIBRARY_DEBUG NOTFOUND)
      set(QT_QTMAIN_LIBRARY_RELEASE NOTFOUND)
    endif()
  endif()

  foreach(QT_MODULE ${QT_MODULES})
    string(TOUPPER ${QT_MODULE} _upper_qt_module)
    find_path(QT_${_upper_qt_module}_INCLUDE_DIR ${QT_MODULE}
              PATHS
              ${QT_HEADERS_DIR}/${QT_MODULE}
              ${QT_LIBRARY_DIR}/${QT_MODULE}.framework/Headers
              NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
      )
    # phonon doesn't seem consistent, let's try phonondefs.h for some
    # installations
    if(${QT_MODULE} STREQUAL "phonon")
      find_path(QT_${_upper_qt_module}_INCLUDE_DIR phonondefs.h
                PATHS
                ${QT_HEADERS_DIR}/${QT_MODULE}
                ${QT_LIBRARY_DIR}/${QT_MODULE}.framework/Headers
                NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
        )
    endif()
  endforeach()

  if(Q_WS_WIN)
    set(QT_MODULES ${QT_MODULES} QAxContainer QAxServer)
    # Set QT_AXCONTAINER_INCLUDE_DIR and QT_AXSERVER_INCLUDE_DIR
    find_path(QT_QAXCONTAINER_INCLUDE_DIR ActiveQt
      PATHS ${QT_HEADERS_DIR}/ActiveQt
      NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
      )
    find_path(QT_QAXSERVER_INCLUDE_DIR ActiveQt
      PATHS ${QT_HEADERS_DIR}/ActiveQt
      NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
      )
  endif()

  # Set QT_QTDESIGNERCOMPONENTS_INCLUDE_DIR
  find_path(QT_QTDESIGNERCOMPONENTS_INCLUDE_DIR QDesignerComponents
    PATHS
    ${QT_HEADERS_DIR}/QtDesigner
    ${QT_LIBRARY_DIR}/QtDesigner.framework/Headers
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  # Set QT_QTASSISTANT_INCLUDE_DIR
  find_path(QT_QTASSISTANT_INCLUDE_DIR QtAssistant
    PATHS
    ${QT_HEADERS_DIR}/QtAssistant
    ${QT_LIBRARY_DIR}/QtAssistant.framework/Headers
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  # Set QT_QTASSISTANTCLIENT_INCLUDE_DIR
  find_path(QT_QTASSISTANTCLIENT_INCLUDE_DIR QAssistantClient
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
  foreach(QT_MODULE ${QT_MODULES})
    string(TOUPPER ${QT_MODULE} _upper_qt_module)
    find_library(QT_${_upper_qt_module}_LIBRARY_RELEASE
                 NAMES ${QT_MODULE}${QT_LIBINFIX} ${QT_MODULE}${QT_LIBINFIX}4
                 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
        )
    find_library(QT_${_upper_qt_module}_LIBRARY_DEBUG
                 NAMES ${QT_MODULE}${QT_LIBINFIX}_debug ${QT_MODULE}${QT_LIBINFIX}d ${QT_MODULE}${QT_LIBINFIX}d4
                 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
        )
    if(QT_${_upper_qt_module}_LIBRARY_RELEASE MATCHES "/${QT_MODULE}\\.framework$")
      if(NOT EXISTS "${QT_${_upper_qt_module}_LIBRARY_RELEASE}/${QT_MODULE}")
        # Release framework library file does not exist... Force to NOTFOUND:
        set(QT_${_upper_qt_module}_LIBRARY_RELEASE "QT_${_upper_qt_module}_LIBRARY_RELEASE-NOTFOUND" CACHE FILEPATH "Path to a library." FORCE)
      endif()
    endif()
    if(QT_${_upper_qt_module}_LIBRARY_DEBUG MATCHES "/${QT_MODULE}\\.framework$")
      if(NOT EXISTS "${QT_${_upper_qt_module}_LIBRARY_DEBUG}/${QT_MODULE}")
        # Debug framework library file does not exist... Force to NOTFOUND:
        set(QT_${_upper_qt_module}_LIBRARY_DEBUG "QT_${_upper_qt_module}_LIBRARY_DEBUG-NOTFOUND" CACHE FILEPATH "Path to a library." FORCE)
      endif()
    endif()
  endforeach()

  # QtUiTools is sometimes not in the same directory as the other found libraries
  # e.g. on Mac, its never a framework like the others are
  if(QT_QTCORE_LIBRARY_RELEASE AND NOT QT_QTUITOOLS_LIBRARY_RELEASE)
    find_library(QT_QTUITOOLS_LIBRARY_RELEASE NAMES QtUiTools${QT_LIBINFIX} PATHS ${QT_LIBRARY_DIR})
  endif()

  # Set QT_QTDESIGNERCOMPONENTS_LIBRARY
  find_library(QT_QTDESIGNERCOMPONENTS_LIBRARY_RELEASE NAMES QtDesignerComponents${QT_LIBINFIX} QtDesignerComponents${QT_LIBINFIX}4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  find_library(QT_QTDESIGNERCOMPONENTS_LIBRARY_DEBUG   NAMES QtDesignerComponents${QT_LIBINFIX}_debug QtDesignerComponents${QT_LIBINFIX}d QtDesignerComponents${QT_LIBINFIX}d4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)

  # Set QT_QTMAIN_LIBRARY
  if(Q_WS_WIN)
    find_library(QT_QTMAIN_LIBRARY_RELEASE NAMES qtmain${QT_LIBINFIX} PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    find_library(QT_QTMAIN_LIBRARY_DEBUG NAMES qtmain${QT_LIBINFIX}d PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  endif()

  # Set QT_QTASSISTANTCLIENT_LIBRARY
  find_library(QT_QTASSISTANTCLIENT_LIBRARY_RELEASE NAMES QtAssistantClient${QT_LIBINFIX} QtAssistantClient${QT_LIBINFIX}4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  find_library(QT_QTASSISTANTCLIENT_LIBRARY_DEBUG   NAMES QtAssistantClient${QT_LIBINFIX}_debug QtAssistantClient${QT_LIBINFIX}d QtAssistantClient${QT_LIBINFIX}d4 PATHS ${QT_LIBRARY_DIR}  NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)

  # Set QT_QTASSISTANT_LIBRARY
  find_library(QT_QTASSISTANT_LIBRARY_RELEASE NAMES QtAssistantClient${QT_LIBINFIX} QtAssistantClient${QT_LIBINFIX}4 QtAssistant${QT_LIBINFIX} QtAssistant${QT_LIBINFIX}4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  find_library(QT_QTASSISTANT_LIBRARY_DEBUG   NAMES QtAssistantClient${QT_LIBINFIX}_debug QtAssistantClient${QT_LIBINFIX}d QtAssistantClient${QT_LIBINFIX}d4 QtAssistant${QT_LIBINFIX}_debug QtAssistant${QT_LIBINFIX}d4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)

  # Set QT_QTHELP_LIBRARY
  find_library(QT_QTCLUCENE_LIBRARY_RELEASE NAMES QtCLucene${QT_LIBINFIX} QtCLucene${QT_LIBINFIX}4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  find_library(QT_QTCLUCENE_LIBRARY_DEBUG   NAMES QtCLucene${QT_LIBINFIX}_debug QtCLucene${QT_LIBINFIX}d QtCLucene${QT_LIBINFIX}d4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  if(Q_WS_MAC AND QT_QTCORE_LIBRARY_RELEASE AND NOT QT_QTCLUCENE_LIBRARY_RELEASE)
    find_library(QT_QTCLUCENE_LIBRARY_RELEASE NAMES QtCLucene${QT_LIBINFIX} PATHS ${QT_LIBRARY_DIR})
  endif()


  ############################################
  #
  # Check the existence of the libraries.
  #
  ############################################


  macro(_qt4_add_target_depends_internal _QT_MODULE _PROPERTY)
    if (TARGET Qt4::${_QT_MODULE})
      foreach(_DEPEND ${ARGN})
        set(_VALID_DEPENDS)
        if (TARGET Qt4::Qt${_DEPEND})
          list(APPEND _VALID_DEPENDS Qt4::Qt${_DEPEND})
        endif()
        if (_VALID_DEPENDS)
          set_property(TARGET Qt4::${_QT_MODULE} APPEND PROPERTY
            ${_PROPERTY}
            "${_VALID_DEPENDS}"
          )
        endif()
        set(_VALID_DEPENDS)
      endforeach()
    endif()
  endmacro()

  macro(_qt4_add_target_depends _QT_MODULE)
    get_target_property(_configs Qt4::${_QT_MODULE} IMPORTED_CONFIGURATIONS)
    foreach(_config ${_configs})
      _qt4_add_target_depends_internal(${_QT_MODULE} IMPORTED_LINK_INTERFACE_LIBRARIES_${_config} ${ARGN})
    endforeach()
    set(_configs)
  endmacro()

  macro(_qt4_add_target_private_depends _QT_MODULE)
    get_target_property(_configs ${_QT_MODULE} IMPORTED_CONFIGURATIONS)
    foreach(_config ${_configs})
      _qt4_add_target_depends_internal(${_QT_MODULE} IMPORTED_LINK_DEPENDENT_LIBRARIES_${_config} ${ARGN})
    endforeach()
    set(_configs)
  endmacro()


  # Set QT_xyz_LIBRARY variable and add
  # library include path to QT_INCLUDES
  _QT4_ADJUST_LIB_VARS(QtCore)
  set_property(TARGET Qt4::QtCore APPEND PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES
      "${QT_MKSPECS_DIR}/default"
      ${QT_INCLUDE_DIR}
  )

  foreach(QT_MODULE ${QT_MODULES})
    _QT4_ADJUST_LIB_VARS(${QT_MODULE})
    _qt4_add_target_depends(${QT_MODULE} Core)
  endforeach()

  _QT4_ADJUST_LIB_VARS(QtAssistant)
  _QT4_ADJUST_LIB_VARS(QtAssistantClient)
  _QT4_ADJUST_LIB_VARS(QtCLucene)
  _QT4_ADJUST_LIB_VARS(QtDesignerComponents)

  # platform dependent libraries
  if(Q_WS_WIN)
    _QT4_ADJUST_LIB_VARS(qtmain)

    _QT4_ADJUST_LIB_VARS(QAxServer)
    set_property(TARGET Qt4::QAxServer PROPERTY
      INTERFACE_QT4_NO_LINK_QTMAIN ON
    )
    set_property(TARGET Qt4::QAxServer APPEND PROPERTY
      COMPATIBLE_INTERFACE_BOOL QT4_NO_LINK_QTMAIN)

    _QT4_ADJUST_LIB_VARS(QAxContainer)
  endif()

  # Only public dependencies are listed here.
  # Eg, QtDBus links to QtXml, but users of QtDBus do not need to
  # link to QtXml because QtDBus only uses it internally, not in public
  # headers.
  # Everything depends on QtCore, but that is covered above already
  _qt4_add_target_depends(Qt3Support Sql Gui Network)
  if (TARGET Qt4::Qt3Support)
    # An additional define is required for QT3_SUPPORT
    set_property(TARGET Qt4::Qt3Support APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS QT3_SUPPORT)
  endif()
  _qt4_add_target_depends(QtDeclarative Script Gui)
  _qt4_add_target_depends(QtDesigner Gui)
  _qt4_add_target_depends(QtHelp Gui)
  _qt4_add_target_depends(QtMultimedia Gui)
  _qt4_add_target_depends(QtOpenGL Gui)
  _qt4_add_target_depends(QtSvg Gui)
  _qt4_add_target_depends(QtWebKit Gui Network)

  _qt4_add_target_private_depends(Qt3Support Xml)
  _qt4_add_target_private_depends(QtSvg Xml)
  _qt4_add_target_private_depends(QtDBus Xml)
  _qt4_add_target_private_depends(QtUiTools Xml Gui)
  _qt4_add_target_private_depends(QtHelp Sql Xml Network)
  _qt4_add_target_private_depends(QtXmlPatterns Network)
  _qt4_add_target_private_depends(QtScriptTools Gui)
  _qt4_add_target_private_depends(QtWebKit XmlPatterns)
  _qt4_add_target_private_depends(QtDeclarative XmlPatterns Svg Sql Gui)
  _qt4_add_target_private_depends(QtMultimedia Gui)
  _qt4_add_target_private_depends(QtOpenGL Gui)
  _qt4_add_target_private_depends(QAxServer Gui)
  _qt4_add_target_private_depends(QAxContainer Gui)
  _qt4_add_target_private_depends(phonon Gui)
  if(QT_QTDBUS_FOUND)
    _qt4_add_target_private_depends(phonon DBus)
  endif()

  if (WIN32 AND NOT QT4_NO_LINK_QTMAIN)
    set(_isExe $<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>)
    set(_isWin32 $<BOOL:$<TARGET_PROPERTY:WIN32_EXECUTABLE>>)
    set(_isNotExcluded $<NOT:$<BOOL:$<TARGET_PROPERTY:QT4_NO_LINK_QTMAIN>>>)
    set(_isPolicyNEW $<TARGET_POLICY:CMP0020>)
    get_target_property(_configs Qt4::QtCore IMPORTED_CONFIGURATIONS)
    foreach(_config ${_configs})
      set_property(TARGET Qt4::QtCore APPEND PROPERTY
        IMPORTED_LINK_INTERFACE_LIBRARIES_${_config}
          $<$<AND:${_isExe},${_isWin32},${_isNotExcluded},${_isPolicyNEW}>:Qt4::qtmain>
      )
    endforeach()
    unset(_configs)
    unset(_isExe)
    unset(_isWin32)
    unset(_isNotExcluded)
    unset(_isPolicyNEW)
  endif()

  #######################################
  #
  #       Check the executables of Qt
  #          ( moc, uic, rcc )
  #
  #######################################


  if(QT_QMAKE_CHANGED)
    set(QT_UIC_EXECUTABLE NOTFOUND)
    set(QT_MOC_EXECUTABLE NOTFOUND)
    set(QT_UIC3_EXECUTABLE NOTFOUND)
    set(QT_RCC_EXECUTABLE NOTFOUND)
    set(QT_DBUSCPP2XML_EXECUTABLE NOTFOUND)
    set(QT_DBUSXML2CPP_EXECUTABLE NOTFOUND)
    set(QT_LUPDATE_EXECUTABLE NOTFOUND)
    set(QT_LRELEASE_EXECUTABLE NOTFOUND)
    set(QT_QCOLLECTIONGENERATOR_EXECUTABLE NOTFOUND)
    set(QT_DESIGNER_EXECUTABLE NOTFOUND)
    set(QT_LINGUIST_EXECUTABLE NOTFOUND)
  endif()

  find_program(QT_MOC_EXECUTABLE
    NAMES moc-qt4 moc moc4
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  find_program(QT_UIC_EXECUTABLE
    NAMES uic-qt4 uic uic4
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  find_program(QT_UIC3_EXECUTABLE
    NAMES uic3
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  find_program(QT_RCC_EXECUTABLE
    NAMES rcc
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  find_program(QT_DBUSCPP2XML_EXECUTABLE
    NAMES qdbuscpp2xml
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  find_program(QT_DBUSXML2CPP_EXECUTABLE
    NAMES qdbusxml2cpp
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  find_program(QT_LUPDATE_EXECUTABLE
    NAMES lupdate-qt4 lupdate lupdate4
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  find_program(QT_LRELEASE_EXECUTABLE
    NAMES lrelease-qt4 lrelease lrelease4
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  find_program(QT_QCOLLECTIONGENERATOR_EXECUTABLE
    NAMES qcollectiongenerator-qt4 qcollectiongenerator
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  find_program(QT_DESIGNER_EXECUTABLE
    NAMES designer-qt4 designer designer4
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  find_program(QT_LINGUIST_EXECUTABLE
    NAMES linguist-qt4 linguist linguist4
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
    )

  if (QT_MOC_EXECUTABLE)
     set(QT_WRAP_CPP "YES")
  endif ()

  if (QT_UIC_EXECUTABLE)
     set(QT_WRAP_UI "YES")
  endif ()



  mark_as_advanced( QT_UIC_EXECUTABLE QT_UIC3_EXECUTABLE QT_MOC_EXECUTABLE
    QT_RCC_EXECUTABLE QT_DBUSXML2CPP_EXECUTABLE QT_DBUSCPP2XML_EXECUTABLE
    QT_LUPDATE_EXECUTABLE QT_LRELEASE_EXECUTABLE QT_QCOLLECTIONGENERATOR_EXECUTABLE
    QT_DESIGNER_EXECUTABLE QT_LINGUIST_EXECUTABLE)


  # get the directory of the current file, used later on in the file
  get_filename_component( _qt4_current_dir  "${CMAKE_CURRENT_LIST_FILE}" PATH)


  ###############################################
  #
  #       configuration/system dependent settings
  #
  ###############################################

  include("${_qt4_current_dir}/Qt4ConfigDependentSettings.cmake")


  #######################################
  #
  #       Check the plugins of Qt
  #
  #######################################

  set( QT_PLUGIN_TYPES accessible bearer codecs decorations designer gfxdrivers graphicssystems iconengines imageformats inputmethods mousedrivers phonon_backend script sqldrivers )

  set( QT_ACCESSIBLE_PLUGINS qtaccessiblecompatwidgets qtaccessiblewidgets )
  set( QT_BEARER_PLUGINS qcorewlanbearer qgenericbearer )
  set( QT_CODECS_PLUGINS qcncodecs qjpcodecs qkrcodecs qtwcodecs )
  set( QT_DECORATIONS_PLUGINS qdecorationdefault qdecorationwindows )
  set( QT_DESIGNER_PLUGINS arthurplugin containerextension customwidgetplugin phononwidgets qdeclarativeview qt3supportwidgets qwebview taskmenuextension worldtimeclockplugin )
  set( QT_GRAPHICSDRIVERS_PLUGINS qgfxtransformed qgfxvnc qscreenvfb )
  set( QT_GRAPHICSSYSTEMS_PLUGINS qglgraphicssystem qtracegraphicssystem )
  set( QT_ICONENGINES_PLUGINS qsvgicon )
  set( QT_IMAGEFORMATS_PLUGINS qgif qjpeg qmng qico qsvg qtiff  )
  set( QT_INPUTMETHODS_PLUGINS qimsw_multi )
  set( QT_MOUSEDRIVERS_PLUGINS qwstslibmousehandler )
  if(APPLE)
    set( QT_PHONON_BACKEND_PLUGINS phonon_qt7 )
  elseif(WIN32)
    set( QT_PHONON_BACKEND_PLUGINS phonon_ds9 )
  endif()
  set( QT_SCRIPT_PLUGINS qtscriptdbus )
  set( QT_SQLDRIVERS_PLUGINS qsqldb2 qsqlibase qsqlite qsqlite2 qsqlmysql qsqloci qsqlodbc qsqlpsql qsqltds )

  set( QT_PHONON_PLUGINS ${QT_PHONON_BACKEND_PLUGINS} )
  set( QT_QT3SUPPORT_PLUGINS qtaccessiblecompatwidgets )
  set( QT_QTCORE_PLUGINS ${QT_BEARER_PLUGINS} ${QT_CODECS_PLUGINS} )
  set( QT_QTGUI_PLUGINS qtaccessiblewidgets qgif qjpeg qmng qico qtiff ${QT_DECORATIONS_PLUGINS} ${QT_GRAPHICSDRIVERS_PLUGINS} ${QT_GRAPHICSSYSTEMS_PLUGINS} ${QT_INPUTMETHODS_PLUGINS} ${QT_MOUSEDRIVERS_PLUGINS} )
  set( QT_QTSCRIPT_PLUGINS ${QT_SCRIPT_PLUGINS} )
  set( QT_QTSQL_PLUGINS ${QT_SQLDRIVERS_PLUGINS} )
  set( QT_QTSVG_PLUGINS qsvg qsvgicon )

  if(QT_QMAKE_CHANGED)
    foreach(QT_PLUGIN_TYPE ${QT_PLUGIN_TYPES})
      string(TOUPPER ${QT_PLUGIN_TYPE} _upper_qt_plugin_type)
      set(QT_${_upper_qt_plugin_type}_PLUGINS_DIR ${QT_PLUGINS_DIR}/${QT_PLUGIN_TYPE})
      foreach(QT_PLUGIN ${QT_${_upper_qt_plugin_type}_PLUGINS})
        string(TOUPPER ${QT_PLUGIN} _upper_qt_plugin)
        unset(QT_${_upper_qt_plugin}_LIBRARY_RELEASE CACHE)
        unset(QT_${_upper_qt_plugin}_LIBRARY_DEBUG CACHE)
        unset(QT_${_upper_qt_plugin}_LIBRARY CACHE)
        unset(QT_${_upper_qt_plugin}_PLUGIN_RELEASE CACHE)
        unset(QT_${_upper_qt_plugin}_PLUGIN_DEBUG CACHE)
        unset(QT_${_upper_qt_plugin}_PLUGIN CACHE)
      endforeach()
    endforeach()
  endif()

  # find_library works better than find_file but we need to set prefixes to only match plugins
  foreach(QT_PLUGIN_TYPE ${QT_PLUGIN_TYPES})
    string(TOUPPER ${QT_PLUGIN_TYPE} _upper_qt_plugin_type)
    set(QT_${_upper_qt_plugin_type}_PLUGINS_DIR ${QT_PLUGINS_DIR}/${QT_PLUGIN_TYPE})
    foreach(QT_PLUGIN ${QT_${_upper_qt_plugin_type}_PLUGINS})
      string(TOUPPER ${QT_PLUGIN} _upper_qt_plugin)
      if(QT_IS_STATIC)
        find_library(QT_${_upper_qt_plugin}_LIBRARY_RELEASE
                     NAMES ${QT_PLUGIN}${QT_LIBINFIX} ${QT_PLUGIN}${QT_LIBINFIX}4
                     PATHS ${QT_${_upper_qt_plugin_type}_PLUGINS_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
            )
        find_library(QT_${_upper_qt_plugin}_LIBRARY_DEBUG
                     NAMES ${QT_PLUGIN}${QT_LIBINFIX}_debug ${QT_PLUGIN}${QT_LIBINFIX}d ${QT_PLUGIN}${QT_LIBINFIX}d4
                     PATHS ${QT_${_upper_qt_plugin_type}_PLUGINS_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
            )
        _QT4_ADJUST_LIB_VARS(${QT_PLUGIN})
      else()
        # find_library works easier/better than find_file but we need to set suffixes to only match plugins
        set(CMAKE_FIND_LIBRARY_SUFFIXES_DEFAULT ${CMAKE_FIND_LIBRARY_SUFFIXES})
        set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_SHARED_MODULE_SUFFIX} ${CMAKE_SHARED_LIBRARY_SUFFIX})
        find_library(QT_${_upper_qt_plugin}_PLUGIN_RELEASE
                     NAMES ${QT_PLUGIN}${QT_LIBINFIX} ${QT_PLUGIN}${QT_LIBINFIX}4
                     PATHS ${QT_${_upper_qt_plugin_type}_PLUGINS_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
            )
        find_library(QT_${_upper_qt_plugin}_PLUGIN_DEBUG
                     NAMES ${QT_PLUGIN}${QT_LIBINFIX}_debug ${QT_PLUGIN}${QT_LIBINFIX}d ${QT_PLUGIN}${QT_LIBINFIX}d4
                     PATHS ${QT_${_upper_qt_plugin_type}_PLUGINS_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
            )
        mark_as_advanced(QT_${_upper_qt_plugin}_PLUGIN_RELEASE QT_${_upper_qt_plugin}_PLUGIN_DEBUG)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_DEFAULT})
      endif()
    endforeach()
  endforeach()


  ######################################
  #
  #       Macros for building Qt files
  #
  ######################################

  include("${_qt4_current_dir}/Qt4Macros.cmake")

endif()

#support old QT_MIN_VERSION if set, but not if version is supplied by find_package()
if(NOT Qt4_FIND_VERSION AND QT_MIN_VERSION)
  set(Qt4_FIND_VERSION ${QT_MIN_VERSION})
endif()

if( Qt4_FIND_COMPONENTS )

  # if components specified in find_package(), make sure each of those pieces were found
  set(_QT4_FOUND_REQUIRED_VARS QT_QMAKE_EXECUTABLE QT_MOC_EXECUTABLE QT_RCC_EXECUTABLE QT_INCLUDE_DIR QT_LIBRARY_DIR)
  foreach( component ${Qt4_FIND_COMPONENTS} )
    string( TOUPPER ${component} _COMPONENT )
    if(${_COMPONENT} STREQUAL "QTMAIN")
      if(Q_WS_WIN)
        set(_QT4_FOUND_REQUIRED_VARS ${_QT4_FOUND_REQUIRED_VARS} QT_${_COMPONENT}_LIBRARY)
      endif()
    else()
      set(_QT4_FOUND_REQUIRED_VARS ${_QT4_FOUND_REQUIRED_VARS} QT_${_COMPONENT}_INCLUDE_DIR QT_${_COMPONENT}_LIBRARY)
    endif()
  endforeach()

  if(Qt4_FIND_COMPONENTS MATCHES QtGui)
    set(_QT4_FOUND_REQUIRED_VARS ${_QT4_FOUND_REQUIRED_VARS} QT_UIC_EXECUTABLE)
  endif()

else()

  # if no components specified, we'll make a default set of required variables to say Qt is found
  set(_QT4_FOUND_REQUIRED_VARS QT_QMAKE_EXECUTABLE QT_MOC_EXECUTABLE QT_RCC_EXECUTABLE QT_UIC_EXECUTABLE QT_INCLUDE_DIR
    QT_LIBRARY_DIR QT_QTCORE_LIBRARY)

endif()

if (QT_VERSION_MAJOR GREATER 4)
    set(VERSION_MSG "Found unsuitable Qt version \"${QTVERSION}\" from ${QT_QMAKE_EXECUTABLE}")
    set(QT4_FOUND FALSE)
    if(Qt4_FIND_REQUIRED)
       message( FATAL_ERROR "${VERSION_MSG}, this code requires Qt 4.x")
    else()
      if(NOT Qt4_FIND_QUIETLY)
         message( STATUS    "${VERSION_MSG}")
      endif()
    endif()
else()
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(Qt4
    REQUIRED_VARS ${_QT4_FOUND_REQUIRED_VARS}
    VERSION_VAR QTVERSION
    )
endif()

#######################################
#
#       compatibility settings
#
#######################################
# Backwards compatibility for CMake1.4 and 1.2
set (QT_MOC_EXE ${QT_MOC_EXECUTABLE} )
set (QT_UIC_EXE ${QT_UIC_EXECUTABLE} )
set( QT_QT_LIBRARY "")
set(QT_FOUND ${QT4_FOUND})

