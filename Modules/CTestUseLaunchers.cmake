#.rst:
# CTestUseLaunchers
# -----------------
#
# Set the RULE_LAUNCH_* global properties when CTEST_USE_LAUNCHERS is on.
#
# CTestUseLaunchers is automatically included when you include(CTest).
# However, it is split out into its own module file so projects can use
# the CTEST_USE_LAUNCHERS functionality independently.
#
# To use launchers, set CTEST_USE_LAUNCHERS to ON in a ctest -S
# dashboard script, and then also set it in the cache of the configured
# project.  Both cmake and ctest need to know the value of it for the
# launchers to work properly.  CMake needs to know in order to generate
# proper build rules, and ctest, in order to produce the proper error
# and warning analysis.
#
# For convenience, you may set the ENV variable
# CTEST_USE_LAUNCHERS_DEFAULT in your ctest -S script, too.  Then, as
# long as your CMakeLists uses include(CTest) or
# include(CTestUseLaunchers), it will use the value of the ENV variable
# to initialize a CTEST_USE_LAUNCHERS cache variable.  This cache
# variable initialization only occurs if CTEST_USE_LAUNCHERS is not
# already defined.

#=============================================================================
# Copyright 2008-2012 Kitware, Inc.
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

if(NOT DEFINED CTEST_USE_LAUNCHERS AND DEFINED ENV{CTEST_USE_LAUNCHERS_DEFAULT})
  set(CTEST_USE_LAUNCHERS "$ENV{CTEST_USE_LAUNCHERS_DEFAULT}"
    CACHE INTERNAL "CTEST_USE_LAUNCHERS initial value from ENV")
endif()

if(NOT "${CMAKE_GENERATOR}" MATCHES "Make|Ninja")
  set(CTEST_USE_LAUNCHERS 0)
endif()

if(CTEST_USE_LAUNCHERS)
  set(CTEST_LAUNCH_COMPILE "\"${CMAKE_CTEST_COMMAND}\" --launch --target-name <TARGET_NAME> --build-dir <CMAKE_CURRENT_BINARY_DIR> --output <OBJECT> --source <SOURCE> --language <LANGUAGE> --")
  set(CTEST_LAUNCH_LINK    "\"${CMAKE_CTEST_COMMAND}\" --launch --target-name <TARGET_NAME> --build-dir <CMAKE_CURRENT_BINARY_DIR> --output <TARGET> --target-type <TARGET_TYPE> --language <LANGUAGE> --")
  set(CTEST_LAUNCH_CUSTOM  "\"${CMAKE_CTEST_COMMAND}\" --launch --target-name <TARGET_NAME> --build-dir <CMAKE_CURRENT_BINARY_DIR> --output <OUTPUT> --")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CTEST_LAUNCH_COMPILE}")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CTEST_LAUNCH_LINK}")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_CUSTOM "${CTEST_LAUNCH_CUSTOM}")
endif()
