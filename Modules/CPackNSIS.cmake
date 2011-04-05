# - CPack NSIS generator specific options
#
# The following variables are specific to the graphical installers built
# on Windows using the Nullsoft Installation System.
#
#   CPACK_PACKAGE_INSTALL_REGISTRY_KEY - Registry key used when
#   installing this project.
#
#   CPACK_NSIS_INSTALL_ROOT - The default installation directory presented
#   to the end user by the NSIS installer is under this root dir. The full
#   directory presented to the end user is:
#   ${CPACK_NSIS_INSTALL_ROOT}/${CPACK_PACKAGE_INSTALL_DIRECTORY}
#
#   CPACK_NSIS_MUI_ICON - The icon file (.ico) for the generated
#   install program.
#
#   CPACK_NSIS_MUI_UNIICON - The icon file (.ico) for the generated
#   uninstall program.
#
#   CPACK_PACKAGE_ICON - A branding image that will be displayed inside
#   the installer.
#
#   CPACK_NSIS_EXTRA_INSTALL_COMMANDS - Extra NSIS commands that will
#   be added to the install Section.
#
#   CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS - Extra NSIS commands that will
#   be added to the uninstall Section.
#
#   CPACK_NSIS_COMPRESSOR - The arguments that will be passed to the
#   NSIS SetCompressor command.
#
#   CPACK_NSIS_MODIFY_PATH - If this is set to "ON", then an extra page
#   will appear in the installer that will allow the user to choose
#   whether the program directory should be added to the system PATH
#   variable.
#
#   CPACK_NSIS_DISPLAY_NAME - The display name string that appears in
#   the Windows Add/Remove Program control panel
#
#   CPACK_NSIS_PACKAGE_NAME - The title displayed at the top of the
#   installer.
#
#   CPACK_NSIS_INSTALLED_ICON_NAME - A path to the executable that
#   contains the installer icon.
#
#   CPACK_NSIS_HELP_LINK - URL to a web site providing assistance in
#   installing your application.
#
#   CPACK_NSIS_URL_INFO_ABOUT - URL to a web site providing more
#   information about your application.
#
#   CPACK_NSIS_CONTACT - Contact information for questions and comments
#   about the installation process.
#
#   CPACK_NSIS_CREATE_ICONS_EXTRA - Additional NSIS commands for
#   creating start menu shortcuts.
#
#   CPACK_NSIS_DELETE_ICONS_EXTRA -Additional NSIS commands to
#   uninstall start menu shortcuts.
#
#   CPACK_NSIS_EXECUTABLES_DIRECTORY - Creating NSIS start menu links
#   assumes that they are in 'bin' unless this variable is set.
#   For example, you would set this to 'exec' if your executables are
#   in an exec directory.
#
#   CPACK_NSIS_MUI_FINISHPAGE_RUN - Specify an executable to add an option
#   to run on the finish page of the NSIS installer.

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
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

#FIXME we should put NSIS specific code here
#FIXME but I'm not doing it because I'm not able to test it...
