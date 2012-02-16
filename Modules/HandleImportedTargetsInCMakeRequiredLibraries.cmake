
# This is a helper function used by CheckCXXSourceRuns.cmake and
# CheckCXXSourceCompiles.cmake. Actually it should be used by all macros which
# use TRY_COMPILE() or TRY_RUN().
# It takes the CMAKE_REQUIRED_LIBRARY variable and searches it for imported
# (library) targets. Since the project created by TRY_COMPILE() (and TRY_RUN())
# does not know about these imported targets, this macro here replaces these
# imported targets with the actual library files on disk and it also
# adds the libraries from the link interface of these imported targets.
# E.g the imported target KDE4__kdeui is replaced on my system with /opt/kdelibs/lib/libkdeui.so
# and the link interface libraries, which includes e.g. /opt/kdelibs/lib/libkdecore.so.
# This way imported targets work also when used with CHECK_CXX_SOURCE_COMPILES/RUNS().

# Copyright (c) 2009, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

FUNCTION(HANDLE_IMPORTED_TARGETS_IN_CMAKE_REQUIRED_LIBRARIES _RESULT)
# handle imported library targets
   SET(_CCSR_IMP_TARGETS_MAP)
   SET(_CCSR_REQ_LIBS ${CMAKE_REQUIRED_LIBRARIES})
   SET(_CHECK_FOR_IMPORTED_TARGETS TRUE)
   SET(_CCSR_LOOP_COUNTER 0)
   WHILE(_CHECK_FOR_IMPORTED_TARGETS)
      MATH(EXPR _CCSR_LOOP_COUNTER "${_CCSR_LOOP_COUNTER} + 1 ")
      SET(_CCSR_NEW_REQ_LIBS )
      SET(_CHECK_FOR_IMPORTED_TARGETS FALSE)
      FOREACH(_CURRENT_LIB ${_CCSR_REQ_LIBS})
         GET_TARGET_PROPERTY(_importedConfigs "${_CURRENT_LIB}" IMPORTED_CONFIGURATIONS)
         IF (_importedConfigs)
#            MESSAGE(STATUS "Detected imported target ${_CURRENT_LIB}")
            # Ok, so this is an imported target.
            # First we get the imported configurations.
            # Then we get the location of the actual library on disk of the first configuration.
            # then we'll get its link interface libraries property,
            # iterate through it and replace all imported targets we find there
            # with there actual location.

            # guard against infinite loop: abort after 100 iterations ( 100 is arbitrary chosen)
            IF ("${_CCSR_LOOP_COUNTER}" LESS 100)
               SET(_CHECK_FOR_IMPORTED_TARGETS TRUE)
#                ELSE ("${_CCSR_LOOP_COUNTER}" LESS 1)
#                   MESSAGE(STATUS "********* aborting loop, counter : ${_CCSR_LOOP_COUNTER}")
            ENDIF ("${_CCSR_LOOP_COUNTER}" LESS 100)

            # if one of the imported configurations equals ${CMAKE_TRY_COMPILE_CONFIGURATION},
            # use it, otherwise simply use the first one:
            LIST(FIND _importedConfigs "${CMAKE_TRY_COMPILE_CONFIGURATION}" _configIndexToUse)
            IF("${_configIndexToUse}" EQUAL -1)
              SET(_configIndexToUse 0)
            ENDIF("${_configIndexToUse}" EQUAL -1)
            LIST(GET _importedConfigs ${_configIndexToUse} _importedConfigToUse)

            GET_TARGET_PROPERTY(_importedLocation "${_CURRENT_LIB}" IMPORTED_LOCATION_${_importedConfigToUse})
            GET_TARGET_PROPERTY(_linkInterfaceLibs "${_CURRENT_LIB}" IMPORTED_LINK_INTERFACE_LIBRARIES_${_importedConfigToUse} )

            LIST(APPEND _CCSR_NEW_REQ_LIBS  "${_importedLocation}")
#            MESSAGE(STATUS "Appending lib ${_CURRENT_LIB} as ${_importedLocation}")
            IF(_linkInterfaceLibs)
               FOREACH(_currentLinkInterfaceLib ${_linkInterfaceLibs})
#                  MESSAGE(STATUS "Appending link interface lib ${_currentLinkInterfaceLib}")
                  IF(_currentLinkInterfaceLib)
                     LIST(APPEND _CCSR_NEW_REQ_LIBS "${_currentLinkInterfaceLib}" )
                  ENDIF(_currentLinkInterfaceLib)
               ENDFOREACH(_currentLinkInterfaceLib "${_linkInterfaceLibs}")
            ENDIF(_linkInterfaceLibs)
         ELSE(_importedConfigs)
            # "Normal" libraries are just used as they are.
            LIST(APPEND _CCSR_NEW_REQ_LIBS "${_CURRENT_LIB}" )
#            MESSAGE(STATUS "Appending lib directly: ${_CURRENT_LIB}")
         ENDIF(_importedConfigs)
      ENDFOREACH(_CURRENT_LIB ${_CCSR_REQ_LIBS})

      SET(_CCSR_REQ_LIBS ${_CCSR_NEW_REQ_LIBS} )
   ENDWHILE(_CHECK_FOR_IMPORTED_TARGETS)

   # Finally we iterate once more over all libraries. This loop only removes
   # all remaining imported target names (there shouldn't be any left anyway).
   SET(_CCSR_NEW_REQ_LIBS )
   FOREACH(_CURRENT_LIB ${_CCSR_REQ_LIBS})
      GET_TARGET_PROPERTY(_importedConfigs "${_CURRENT_LIB}" IMPORTED_CONFIGURATIONS)
      IF (NOT _importedConfigs)
         LIST(APPEND _CCSR_NEW_REQ_LIBS "${_CURRENT_LIB}" )
#         MESSAGE(STATUS "final: appending ${_CURRENT_LIB}")
      ELSE (NOT _importedConfigs)
#             MESSAGE(STATUS "final: skipping ${_CURRENT_LIB}")
      ENDIF (NOT _importedConfigs)
   ENDFOREACH(_CURRENT_LIB ${_CCSR_REQ_LIBS})
#   MESSAGE(STATUS "setting -${_RESULT}- to -${_CCSR_NEW_REQ_LIBS}-")
   SET(${_RESULT} "${_CCSR_NEW_REQ_LIBS}" PARENT_SCOPE)

ENDFUNCTION(HANDLE_IMPORTED_TARGETS_IN_CMAKE_REQUIRED_LIBRARIES _CCSR_REQ_LIBS)
