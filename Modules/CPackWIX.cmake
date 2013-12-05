#.rst:
# CPackWIX
# --------
#
# CPack WiX generator specific options
#
# Variables specific to CPack WiX generator
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#
# The following variables are specific to the installers built on
# Windows using WiX.
#
# .. variable:: CPACK_WIX_UPGRADE_GUID
#
#  Upgrade GUID (``Product/@UpgradeCode``)
#
#  Will be automatically generated unless explicitly provided.
#
#  It should be explicitly set to a constant generated gloabally unique
#  identifier (GUID) to allow your installers to replace existing
#  installations that use the same GUID.
#
#  You may for example explicitly set this variable in your
#  CMakeLists.txt to the value that has been generated per default.  You
#  should not use GUIDs that you did not generate yourself or which may
#  belong to other projects.
#
#  A GUID shall have the following fixed length syntax::
#
#   XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
#
#  (each X represents an uppercase hexadecimal digit)
#
# .. variable:: CPACK_WIX_PRODUCT_GUID
#
#  Product GUID (``Product/@Id``)
#
#  Will be automatically generated unless explicitly provided.
#
#  If explicitly provided this will set the Product Id of your installer.
#
#  The installer will abort if it detects a pre-existing installation that
#  uses the same GUID.
#
#  The GUID shall use the syntax described for CPACK_WIX_UPGRADE_GUID.
#
# .. variable:: CPACK_WIX_LICENSE_RTF
#
#  RTF License File
#
#  If CPACK_RESOURCE_FILE_LICENSE has an .rtf extension it is used as-is.
#
#  If CPACK_RESOURCE_FILE_LICENSE has an .txt extension it is implicitly
#  converted to RTF by the WiX Generator.
#
#  With CPACK_WIX_LICENSE_RTF you can override the license file used by the
#  WiX Generator in case CPACK_RESOURCE_FILE_LICENSE is in an unsupported
#  format or the .txt -> .rtf conversion does not work as expected.
#
# .. variable:: CPACK_WIX_PRODUCT_ICON
#
#  The Icon shown next to the program name in Add/Remove programs.
#
#  If set, this icon is used in place of the default icon.
#
# .. variable:: CPACK_WIX_UI_REF
#
#  This variable allows you to override the Id of the ``<UIRef>`` element
#  in the WiX template.
#
#  The default is ``WixUI_InstallDir`` in case no CPack components have
#  been defined and ``WixUI_FeatureTree`` otherwise.
#
# .. variable:: CPACK_WIX_UI_BANNER
#
#  The bitmap will appear at the top of all installer pages other than the
#  welcome and completion dialogs.
#
#  If set, this image will replace the default banner image.
#
#  This image must be 493 by 58 pixels.
#
# .. variable:: CPACK_WIX_UI_DIALOG
#
#  Background bitmap used on the welcome and completion dialogs.
#
#  If this variable is set, the installer will replace the default dialog
#  image.
#
#  This image must be 493 by 312 pixels.
#
# .. variable:: CPACK_WIX_PROGRAM_MENU_FOLDER
#
#  Start menu folder name for launcher.
#
#  If this variable is not set, it will be initialized with CPACK_PACKAGE_NAME
#
# .. variable:: CPACK_WIX_CULTURES
#
#  Language(s) of the installer
#
#  Languages are compiled into the WixUI extension library.  To use them,
#  simply provide the name of the culture.  If you specify more than one
#  culture identifier in a comma or semicolon delimited list, the first one
#  that is found will be used.  You can find a list of supported languages at:
#  http://wix.sourceforge.net/manual-wix3/WixUI_localization.htm
#
# .. variable:: CPACK_WIX_TEMPLATE
#
#  Template file for WiX generation
#
#  If this variable is set, the specified template will be used to generate
#  the WiX wxs file.  This should be used if further customization of the
#  output is required.
#
#  If this variable is not set, the default MSI template included with CMake
#  will be used.
#
# .. variable:: CPACK_WIX_EXTRA_SOURCES
#
#  Extra WiX source files
#
#  This variable provides an optional list of extra WiX source files (.wxs)
#  that should be compiled and linked.  The full path to source files is
#  required.
#
# .. variable:: CPACK_WIX_EXTRA_OBJECTS
#
#  Extra WiX object files or libraries
#
#  This variable provides an optional list of extra WiX object (.wixobj)
#  and/or WiX library (.wixlib) files.  The full path to objects and libraries
#  is required.
#
# .. variable:: CPACK_WIX_EXTENSIONS
#
#  This variable provides a list of additional extensions for the WiX
#  tools light and candle.
#
# .. variable:: CPACK_WIX_<TOOL>_EXTENSIONS
#
#  This is the tool specific version of CPACK_WIX_EXTENSIONS.
#  ``<TOOL>`` can be either LIGHT or CANDLE.
#
# .. variable:: CPACK_WIX_<TOOL>_EXTRA_FLAGS
#
#  This list variable allows you to pass additional
#  flags to the WiX tool ``<TOOL>``.
#
#  Use it at your own risk.
#  Future versions of CPack may generate flags which may be in conflict
#  with your own flags.
#
#  ``<TOOL>`` can be either LIGHT or CANDLE.
#

#=============================================================================
# Copyright 2013 Kitware, Inc.
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

if(NOT CPACK_WIX_ROOT)
  file(TO_CMAKE_PATH "$ENV{WIX}" CPACK_WIX_ROOT)
endif()

find_program(CPACK_WIX_CANDLE_EXECUTABLE candle
  PATHS "${CPACK_WIX_ROOT}/bin")

if(NOT CPACK_WIX_CANDLE_EXECUTABLE)
  message(FATAL_ERROR "Could not find the WiX candle executable.")
endif()

find_program(CPACK_WIX_LIGHT_EXECUTABLE light
  PATHS "${CPACK_WIX_ROOT}/bin")

if(NOT CPACK_WIX_LIGHT_EXECUTABLE)
  message(FATAL_ERROR "Could not find the WiX light executable.")
endif()
