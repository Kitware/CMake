# - Find lzma and lzmadec
# Find the native LZMA includes and library
#
#  LZMA_INCLUDE_DIR    - where to find lzma.h, etc.
#  LZMA_LIBRARIES      - List of libraries when using liblzma.
#  LZMA_FOUND          - True if liblzma found.
#  LZMADEC_INCLUDE_DIR - where to find lzmadec.h, etc.
#  LZMADEC_LIBRARIES   - List of libraries when using liblzmadec.
#  LZMADEC_FOUND       - True if liblzmadec found.

if(LZMA_INCLUDE_DIR)
  # Already in cache, be silent
  set(LZMA_FIND_QUIETLY TRUE)
endif(LZMA_INCLUDE_DIR)

find_path(LZMA_INCLUDE_DIR lzma.h)
find_library(LZMA_LIBRARY NAMES lzma )

# handle the QUIETLY and REQUIRED arguments and set LZMA_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LZMA DEFAULT_MSG LZMA_LIBRARY LZMA_INCLUDE_DIR)

if(LZMA_FOUND)
  set( LZMA_LIBRARIES ${LZMA_LIBRARY} )
else(LZMA_FOUND)
  set( LZMA_LIBRARIES )

  if(LZMADEC_INCLUDE_DIR)
    # Already in cache, be silent
    set(LZMADEC_FIND_QUIETLY TRUE)
  endif(LZMADEC_INCLUDE_DIR)

  find_path(LZMADEC_INCLUDE_DIR lzmadec.h)
  find_library(LZMADEC_LIBRARY NAMES lzmadec )

  # handle the QUIETLY and REQUIRED arguments and set LZMADEC_FOUND to TRUE if
  # all listed variables are TRUE
  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(LZMADEC DEFAULT_MSG LZMADEC_LIBRARY
    LZMADEC_INCLUDE_DIR)

  if(LZMADEC_FOUND)
    set( LZMADEC_LIBRARIES ${LZMADEC_LIBRARY} )
  else(LZMADEC_FOUND)
    set( LZMADEC_LIBRARIES )
  endif(LZMADEC_FOUND)
endif(LZMA_FOUND)


mark_as_advanced( LZMA_LIBRARY LZMA_INCLUDE_DIR
  LZMADEC_LIBRARY LZMADEC_INCLUDE_DIR )
