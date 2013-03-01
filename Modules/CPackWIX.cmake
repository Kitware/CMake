##section Variables specific to CPack WiX generator
##end
##module
# - CPack WiX generator specific options
#
# The following variables are specific to the installers built
# on Windows using WiX.
##end
##variable
#  CPACK_WIX_UPGRADE_GUID - Upgrade GUID (Product/@UpgradeCode)
#
# Will be automatically generated unless explicitly provided.
#
# It should be explicitly set to a constant generated
# gloabally unique identifier (GUID) to allow your installers
# to replace existing installations that use the same GUID.
#
# You may for example explicitly set this variable in
# your CMakeLists.txt to the value that has been generated per default.
# You should not use GUIDs that you did not generate yourself or which may
# belong to other projects.
#
# A GUID shall have the following fixed length syntax:
# XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
#  (each X represents an uppercase hexadecimal digit)
##end
##variable
#  CPACK_WIX_PRODUCT_GUID - Product GUID (Product/@Id)
#
# Will be automatically generated unless explicitly provided.
#
# If explicitly provided this will set the Product Id of your installer.
#
# The installer will abort if it detects a pre-existing installation that uses
# the same GUID.
#
# The GUID shall use the syntax described for CPACK_WIX_UPGRADE_GUID.
##end
##variable
#  CPACK_WIX_LICENSE_RTF - RTF License File
#
# If CPACK_RESOURCE_FILE_LICENSE has an .rtf extension
# it is used as-is.
#
# If CPACK_RESOURCE_FILE_LICENSE has an .txt extension
# it is implicitly converted to RTF by the WiX Generator.
#
# With CPACK_WIX_LICENSE_RTF you can override the license file used
# by the WiX Generator in case CPACK_RESOURCE_FILE_LICENSE
# is in an unsupported format or the .txt -> .rtf
# conversion does not work as expected.
#
##end
#
##variable
# CPACK_WIX_PRODUCT_ICON - The Icon shown next to the program name in Add/Remove programs.
#
# If set, this icon is used in place of the default icon.
#
##end
#
##variable
# CPACK_WIX_UI_BANNER - The bitmap will appear at the top of all installer pages other than the welcome and completion dialogs.
#
# If set, this image will replace the default banner image.
#
# This image must be 493 by 58 pixels.
#
##end
#
##variable
# CPACK_WIX_UI_DIALOG - Background bitmap used on the welcome and completion dialogs.
#
# If this variable is set, the installer will replace the default dialog image.
#
# This image must be 493 by 312 pixels.
#
##end
#
##variable
# CPACK_WIX_PROGRAM_MENU_FOLDER - Start menu folder name for launcher.
#
# If this variable is not set, it will be initialized with CPACK_PACKAGE_NAME
##end

#=============================================================================
# Copyright 2012 Kitware, Inc.
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
