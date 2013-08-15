#include "cmCPackDocumentVariables.h"
#include "cmake.h"

void cmCPackDocumentVariables::DefineVariables(cmake* cm)
{
  // Subsection: variables defined/used by cpack,
  // which are common to all CPack generators

  cm->DefineProperty
      ("CPACK_PACKAGING_INSTALL_PREFIX", cmProperty::VARIABLE,
       "The prefix used in the built package.",
       "Each CPack generator has a default value (like /usr)."
       " This default value may"
       " be overwritten from the CMakeLists.txt or the cpack command line"
       " by setting an alternative value.\n"
       "e.g. "
       " set(CPACK_PACKAGING_INSTALL_PREFIX \"/opt\")\n"
       "This is not the same purpose as CMAKE_INSTALL_PREFIX which"
       " is used when installing from the build tree without building"
       " a package."
       "", false,
       "Variables common to all CPack generators");

  cm->DefineProperty
        ("CPACK_INCLUDE_TOPLEVEL_DIRECTORY", cmProperty::VARIABLE,
         "Boolean toggle to include/exclude top level directory.",
         "When preparing a package CPack installs the item under"
         " the so-called top level directory. The purpose of"
         " is to include (set to 1 or ON or TRUE) the top level directory"
         " in the package or not (set to 0 or OFF or FALSE).\n"
         "Each CPack generator has a built-in default value for this"
         " variable. E.g. Archive generators (ZIP, TGZ, ...) includes"
         " the top level whereas RPM or DEB don't. The user may override"
         " the default value by setting this variable.\n"
         "There is a similar variable "
         "CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY "
         "which may be used to override the behavior for the component"
         " packaging case which may have different default value for"
         " historical (now backward compatibility) reason.", false,
         "Variables common to all CPack generators");

  cm->DefineProperty
          ("CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY", cmProperty::VARIABLE,
            "Boolean toggle to include/exclude top level directory "
             "(component case).",
            "Similar usage as CPACK_INCLUDE_TOPLEVEL_DIRECTORY"
            " but for the component case. "
            "See CPACK_INCLUDE_TOPLEVEL_DIRECTORY documentation for"
            " the detail.", false,
            "Variables common to all CPack generators");

  cm->DefineProperty
          ("CPACK_SET_DESTDIR", cmProperty::VARIABLE,
           "Boolean toggle to make CPack use DESTDIR mechanism when"
           " packaging.", "DESTDIR means DESTination DIRectory."
           " It is commonly used by makefile "
           "users in order to install software at non-default location. It "
           "is a basic relocation mechanism that should not be used on"
           " Windows (see CMAKE_INSTALL_PREFIX documentation). "
           "It is usually invoked like this:\n"
           " make DESTDIR=/home/john install\n"
           "which will install the concerned software using the"
           " installation prefix, e.g. \"/usr/local\" prepended with "
           "the DESTDIR value which finally gives \"/home/john/usr/local\"."
           " When preparing a package, CPack first installs the items to be "
           "packaged in a local (to the build tree) directory by using the "
           "same DESTDIR mechanism. Nevertheless, if "
           "CPACK_SET_DESTDIR is set then CPack will set DESTDIR before"
           " doing the local install. The most noticeable difference is"
           " that without CPACK_SET_DESTDIR, CPack uses "
           "CPACK_PACKAGING_INSTALL_PREFIX as a prefix whereas with "
           "CPACK_SET_DESTDIR set, CPack will use CMAKE_INSTALL_PREFIX as"
           " a prefix.\n"
           "Manually setting CPACK_SET_DESTDIR may help (or simply be"
           " necessary) if some install rules uses absolute "
           "DESTINATION (see CMake INSTALL command)."
           " However, starting with"
           " CPack/CMake 2.8.3 RPM and DEB installers tries to handle DESTDIR"
           " automatically so that it is seldom necessary for the user to set"
           " it.", false,
           "Variables common to all CPack generators");

  cm->DefineProperty
        ("CPACK_INSTALL_SCRIPT", cmProperty::VARIABLE,
         "Extra CMake script provided by the user.",
         "If set this CMake script will be executed by CPack "
         "during its local [CPack-private] installation "
         "which is done right before packaging the files."
         " The script is not called by e.g.: make install.", false,
         "Variables common to all CPack generators");

  cm->DefineProperty
        ("CPACK_ABSOLUTE_DESTINATION_FILES", cmProperty::VARIABLE,
         "List of files which have been installed using "
         " an ABSOLUTE DESTINATION path.",
         "This variable is a Read-Only variable which is set internally"
         " by CPack during installation and before packaging using"
         " CMAKE_ABSOLUTE_DESTINATION_FILES defined in cmake_install.cmake "
         "scripts. The value can be used within CPack project configuration"
         " file and/or CPack<GEN>.cmake file of <GEN> generator.", false,
         "Variables common to all CPack generators");

  cm->DefineProperty
        ("CPACK_WARN_ON_ABSOLUTE_INSTALL_DESTINATION", cmProperty::VARIABLE,
         "Ask CPack to warn each time a file with absolute INSTALL"
         " DESTINATION is encountered.",
         "This variable triggers the definition of "
         "CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION when CPack runs"
         " cmake_install.cmake scripts.", false,
         "Variables common to all CPack generators");

  cm->DefineProperty
        ("CPACK_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION", cmProperty::VARIABLE,
         "Ask CPack to error out as soon as a file with absolute INSTALL"
         " DESTINATION is encountered.",
         "The fatal error is emitted before the installation of "
         "the offending file takes place. Some CPack generators, like NSIS,"
         "enforce this internally. "
         "This variable triggers the definition of"
         "CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION when CPack runs"
         "Variables common to all CPack generators");
}
