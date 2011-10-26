# - Find upx
# This module looks for some executable packers (i.e. softwares that
# compress executables or shared libs into on-the-fly self-extracting
# executables or shared libs.
# Examples:
#  UPX: http://wildsau.idv.uni-linz.ac.at/mfx/upx.html

#=============================================================================
# Copyright 2001-2009 Kitware, Inc.
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

include(FindCygwin)

find_program(SELF_PACKER_FOR_EXECUTABLE
  upx
  ${CYGWIN_INSTALL_PATH}/bin
  /bin
  /usr/bin
  /usr/local/bin
  /sbin
)

find_program(SELF_PACKER_FOR_SHARED_LIB
  upx
  ${CYGWIN_INSTALL_PATH}/bin
  /bin
  /usr/bin
  /usr/local/bin
  /sbin
)

mark_as_advanced(
  SELF_PACKER_FOR_EXECUTABLE
  SELF_PACKER_FOR_SHARED_LIB
)

#
# Set flags
#
if(SELF_PACKER_FOR_EXECUTABLE MATCHES "upx")
  set(SELF_PACKER_FOR_EXECUTABLE_FLAGS "-q" CACHE STRING
       "Flags for the executable self-packer.")
else(SELF_PACKER_FOR_EXECUTABLE MATCHES "upx")
  set(SELF_PACKER_FOR_EXECUTABLE_FLAGS "" CACHE STRING
       "Flags for the executable self-packer.")
endif(SELF_PACKER_FOR_EXECUTABLE MATCHES "upx")

if(SELF_PACKER_FOR_SHARED_LIB MATCHES "upx")
  set(SELF_PACKER_FOR_SHARED_LIB_FLAGS "-q" CACHE STRING
       "Flags for the shared lib self-packer.")
else(SELF_PACKER_FOR_SHARED_LIB MATCHES "upx")
  set(SELF_PACKER_FOR_SHARED_LIB_FLAGS "" CACHE STRING
       "Flags for the shared lib self-packer.")
endif(SELF_PACKER_FOR_SHARED_LIB MATCHES "upx")

mark_as_advanced(
  SELF_PACKER_FOR_EXECUTABLE_FLAGS
  SELF_PACKER_FOR_SHARED_LIB_FLAGS
)
