#
# This CMake file contains two macros to assist with searching for OSG
# libraries and nodekits.
#

#
# OSG_FIND_PATH
#
function(OSG_FIND_PATH module header)
   string(TOUPPER ${module} module_uc)

   # Try the user's environment request before anything else.
   find_path(${module_uc}_INCLUDE_DIR ${header}
       HINTS
            $ENV{${module_uc}_DIR}
            $ENV{OSG_DIR}
            $ENV{OSGDIR}
       PATH_SUFFIXES include
       PATHS
            ~/Library/Frameworks
            /Library/Frameworks
            /sw # Fink
            /opt/local # DarwinPorts
            /opt/csw # Blastwave
            /opt
            [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OpenThreads_ROOT]
            [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]
   )
endfunction(OSG_FIND_PATH module header)


#
# OSG_FIND_LIBRARY
#
function(OSG_FIND_LIBRARY module library)
   string(TOUPPER ${module} module_uc)

   find_library(${module_uc}_LIBRARY
       NAMES ${library}
       HINTS
            $ENV{${module_uc}_DIR}
            $ENV{OSG_DIR}
            $ENV{OSGDIR}
       PATH_SUFFIXES lib64 lib
       PATHS
            ~/Library/Frameworks
            /Library/Frameworks
            /usr/local
            /usr
            /sw # Fink
            /opt/local # DarwinPorts
            /opt/csw # Blastwave
            /opt
            [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]
   )

   find_library(${module_uc}_LIBRARY_DEBUG
       NAMES ${library}d
       HINTS
            $ENV{${module_uc}_DIR}
            $ENV{OSG_DIR}
            $ENV{OSGDIR}
       PATH_SUFFIXES lib64 lib
       PATHS
            ~/Library/Frameworks
            /Library/Frameworks
            /usr/local
            /usr
            /sw # Fink
            /opt/local # DarwinPorts
            /opt/csw # Blastwave
            /opt
            [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]
    )

   if(NOT ${module_uc}_LIBRARY_DEBUG)
      # They don't have a debug library
      set(${module_uc}_LIBRARY_DEBUG ${${module_uc}_LIBRARY} PARENT_SCOPE)
      set(${module_uc}_LIBRARIES ${${module_uc}_LIBRARY} PARENT_SCOPE)
   else()
      # They really have a FOO_LIBRARY_DEBUG
      set(${module_uc}_LIBRARIES 
          optimized ${${module_uc}_LIBRARY}
          debug ${${module_uc}_LIBRARY_DEBUG}
          PARENT_SCOPE
      )
   endif()


endfunction(OSG_FIND_LIBRARY module library)

#
# OSG_MARK_AS_ADVANCED
# Just a convenience function for calling MARK_AS_ADVANCED
#
function(OSG_MARK_AS_ADVANCED module)
   mark_as_advanced(${module}_INCLUDE_DIR)
   mark_as_advanced(${module}_LIBRARY)
   mark_as_advanced(${module}_LIBRARY_DEBUG)
endfunction()
