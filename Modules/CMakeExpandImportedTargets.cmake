# CMAKE_EXPAND_IMPORTED_TARGETS(<var> LIBRARIES lib1 lib2...libN)
#
# CMAKE_EXPAND_IMPORTED_TARGETS() takes a list of libraries and replaces
# all imported targets contained in this list with their actual file paths
# of the referenced libraries on disk, including the libraries from their
# link interfaces.
# This macro is used by all Check*.cmake files which use
# TRY_COMPILE() or TRY_RUN() and support CMAKE_REQUIRED_LIBRARIES , so that
# these checks support imported targets in CMAKE_REQUIRED_LIBRARIES:
#    cmake_expand_imported_targets(expandedLibs LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} )


#=============================================================================
# Copyright 2012 Kitware, Inc.
# Copyright 2009-2012 Alexander Neundorf <neundorf@kde.org>
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


function(CMAKE_EXPAND_IMPORTED_TARGETS _RESULT _LIB)

   if(NOT "${_LIB}" STREQUAL "LIBRARIES")
      message(FATAL_ERROR "cmake_expand_imported_targets() called with bad second argument \"${_LIB}\", expected keyword \"LIBRARIES\".")
   endif()

   # handle imported library targets

   set(_CCSR_REQ_LIBS ${ARGN})

   set(_CHECK_FOR_IMPORTED_TARGETS TRUE)
   set(_CCSR_LOOP_COUNTER 0)
   while(_CHECK_FOR_IMPORTED_TARGETS)
      math(EXPR _CCSR_LOOP_COUNTER "${_CCSR_LOOP_COUNTER} + 1 ")
      set(_CCSR_NEW_REQ_LIBS )
      set(_CHECK_FOR_IMPORTED_TARGETS FALSE)
      foreach(_CURRENT_LIB ${_CCSR_REQ_LIBS})
         get_target_property(_importedConfigs "${_CURRENT_LIB}" IMPORTED_CONFIGURATIONS)
         if (_importedConfigs)
#            message(STATUS "Detected imported target ${_CURRENT_LIB}")
            # Ok, so this is an imported target.
            # First we get the imported configurations.
            # Then we get the location of the actual library on disk of the first configuration.
            # then we'll get its link interface libraries property,
            # iterate through it and replace all imported targets we find there
            # with there actual location.

            # guard against infinite loop: abort after 100 iterations ( 100 is arbitrary chosen)
            if ("${_CCSR_LOOP_COUNTER}" LESS 100)
               set(_CHECK_FOR_IMPORTED_TARGETS TRUE)
#                else ("${_CCSR_LOOP_COUNTER}" LESS 1)
#                   message(STATUS "********* aborting loop, counter : ${_CCSR_LOOP_COUNTER}")
            endif ("${_CCSR_LOOP_COUNTER}" LESS 100)

            # if one of the imported configurations equals ${CMAKE_TRY_COMPILE_CONFIGURATION},
            # use it, otherwise simply use the first one:
            list(FIND _importedConfigs "${CMAKE_TRY_COMPILE_CONFIGURATION}" _configIndexToUse)
            if("${_configIndexToUse}" EQUAL -1)
              set(_configIndexToUse 0)
            endif("${_configIndexToUse}" EQUAL -1)
            list(GET _importedConfigs ${_configIndexToUse} _importedConfigToUse)

            get_target_property(_importedLocation "${_CURRENT_LIB}" IMPORTED_LOCATION_${_importedConfigToUse})
            get_target_property(_linkInterfaceLibs "${_CURRENT_LIB}" IMPORTED_LINK_INTERFACE_LIBRARIES_${_importedConfigToUse} )

            list(APPEND _CCSR_NEW_REQ_LIBS  "${_importedLocation}")
#            message(STATUS "Appending lib ${_CURRENT_LIB} as ${_importedLocation}")
            if(_linkInterfaceLibs)
               foreach(_currentLinkInterfaceLib ${_linkInterfaceLibs})
#                  message(STATUS "Appending link interface lib ${_currentLinkInterfaceLib}")
                  if(_currentLinkInterfaceLib)
                     list(APPEND _CCSR_NEW_REQ_LIBS "${_currentLinkInterfaceLib}" )
                  endif(_currentLinkInterfaceLib)
               endforeach(_currentLinkInterfaceLib "${_linkInterfaceLibs}")
            endif(_linkInterfaceLibs)
         else(_importedConfigs)
            # "Normal" libraries are just used as they are.
            list(APPEND _CCSR_NEW_REQ_LIBS "${_CURRENT_LIB}" )
#            message(STATUS "Appending lib directly: ${_CURRENT_LIB}")
         endif(_importedConfigs)
      endforeach(_CURRENT_LIB ${_CCSR_REQ_LIBS})

      set(_CCSR_REQ_LIBS ${_CCSR_NEW_REQ_LIBS} )
   endwhile(_CHECK_FOR_IMPORTED_TARGETS)

   # Finally we iterate once more over all libraries. This loop only removes
   # all remaining imported target names (there shouldn't be any left anyway).
   set(_CCSR_NEW_REQ_LIBS )
   foreach(_CURRENT_LIB ${_CCSR_REQ_LIBS})
      get_target_property(_importedConfigs "${_CURRENT_LIB}" IMPORTED_CONFIGURATIONS)
      if (NOT _importedConfigs)
         list(APPEND _CCSR_NEW_REQ_LIBS "${_CURRENT_LIB}" )
#         message(STATUS "final: appending ${_CURRENT_LIB}")
      else (NOT _importedConfigs)
#             message(STATUS "final: skipping ${_CURRENT_LIB}")
      endif (NOT _importedConfigs)
   endforeach(_CURRENT_LIB ${_CCSR_REQ_LIBS})
#   message(STATUS "setting -${_RESULT}- to -${_CCSR_NEW_REQ_LIBS}-")
   set(${_RESULT} "${_CCSR_NEW_REQ_LIBS}" PARENT_SCOPE)

endfunction()
