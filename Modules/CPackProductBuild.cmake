#.rst:
# CPackProductBuild
# -----------------
#
# productbuild CPack generator (Mac OS X).
#
# Variables specific to CPack productbuild generator
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#
# The following variable is specific to installers built on Mac
# OS X using productbuild:
#
# .. variable:: CPACK_COMMAND_PRODUCTBUILD
#
#  Path to the productbuild(1) command used to generate a product archive for
#  the OS X Installer or Mac App Store.  This variable can be used to override
#  the automatically detected command (or specify its location if the
#  auto-detection fails to find it.)
#
# .. variable:: CPACK_COMMAND_PKGBUILD
#
#  Path to the pkgbuild(1) command used to generate an OS X component package
#  on OS X.  This variable can be used to override the automatically detected
#  command (or specify its location if the auto-detection fails to find it.)
#

#=============================================================================
# Copyright 2006-2012 Kitware, Inc.
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
