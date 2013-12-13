#.rst:
# CMakeFindDependencyMacro
# -------------------------
#
# ::
#
#     find_dependency(<dep> [<version>])
#
#
# ``find_dependency()`` wraps a :command:`find_package` call for a package
# dependency. It is designed to be used in a <package>Config.cmake file, and it
# forwards the correct parameters for EXACT, QUIET and REQUIRED which were
# passed to the original :command:`find_package` call.  It also sets an
# informative diagnostic message if the dependency could not be found.
#

#=============================================================================
# Copyright 2013 Stephen Kelly <steveire@gmail.com>
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

macro(find_dependency dep)
  if (NOT ${dep}_FOUND)
    if (${ARGV1})
      set(version ${ARGV1})
    endif()
    set(exact_arg)
    if(${CMAKE_FIND_PACKAGE_NAME}_FIND_VERSION_EXACT)
      set(exact_arg EXACT)
    endif()
    set(quiet_arg)
    if(${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY)
      set(quiet_arg QUIET)
    endif()
    set(required_arg)
    if(${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
      set(required_arg REQUIRED)
    endif()

    find_package(${dep} ${version} ${exact_arg} ${quiet_arg} ${required_arg})
    if (NOT ${dep}_FOUND)
      set(${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE "${CMAKE_FIND_PACKAGE_NAME} could not be found because dependency ${dep} could not be found.")
      set(${CMAKE_FIND_PACKAGE_NAME}_FOUND False)
      return()
    endif()
    set(required_arg)
    set(quiet_arg)
    set(exact_arg)
  endif()
endmacro()
