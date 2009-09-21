# - Find alsa
# Find the alsa libraries (asound)
#
#  This module defines the following variables:
#     ALSA_FOUND       - True if ALSA_INCLUDE_DIR & ALSA_LIBRARY are found
#     ALSA_LIBRARIES   - Set when ALSA_LIBRARY is found
#     ALSA_INCLUDE_DIRS - Set when ALSA_INCLUDE_DIR is found
#
#     ALSA_INCLUDE_DIR - where to find asoundlib.h, etc.
#     ALSA_LIBRARY     - the asound library
#

find_path(ALSA_INCLUDE_DIR NAMES asoundlib.h
          PATH_SUFFIXES alsa
          DOC "The ALSA (asound) include directory"
)

find_library(ALSA_LIBRARY NAMES asound
          DOC "The ALSA (asound) library"
)

# handle the QUIETLY and REQUIRED arguments and set ALSA_FOUND to TRUE if 
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ALSA DEFAULT_MSG ALSA_LIBRARY ALSA_INCLUDE_DIR)

if(ALSA_FOUND)
  set( ALSA_LIBRARIES ${ALSA_LIBRARY} )
  set( ALSA_INCLUDE_DIRS ${ALSA_INCLUDE_DIR} )
endif()

mark_as_advanced(ALSA_INCLUDE_DIR ALSA_LIBRARY)
