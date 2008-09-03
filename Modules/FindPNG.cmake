# - Find the native PNG includes and library
#
# This module defines
#  PNG_INCLUDE_DIR, where to find png.h, etc.
#  PNG_LIBRARIES, the libraries to link against to use PNG.
#  PNG_DEFINITIONS - You should ADD_DEFINITONS(${PNG_DEFINITIONS}) before compiling code that includes png library files.
#  PNG_FOUND, If false, do not try to use PNG.
# also defined, but not for general use are
#  PNG_LIBRARY, where to find the PNG library.
# None of the above will be defined unles zlib can be found.
# PNG depends on Zlib
INCLUDE(FindZLIB)

IF(ZLIB_FOUND)
  FIND_PATH(PNG_PNG_INCLUDE_DIR png.h
  /usr/local/include/libpng             # OpenBSD
  )

  SET(PNG_NAMES ${PNG_NAMES} png libpng png12 libpng12)
  FIND_LIBRARY(PNG_LIBRARY NAMES ${PNG_NAMES} )

  IF (PNG_LIBRARY AND PNG_PNG_INCLUDE_DIR)
      # png.h includes zlib.h. Sigh.
      SET(PNG_INCLUDE_DIR ${PNG_PNG_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} )
      SET(PNG_LIBRARIES ${PNG_LIBRARY} ${ZLIB_LIBRARY})

      IF (CYGWIN)
        IF(BUILD_SHARED_LIBS)
           # No need to define PNG_USE_DLL here, because it's default for Cygwin.
        ELSE(BUILD_SHARED_LIBS)
          SET (PNG_DEFINITIONS -DPNG_STATIC)
        ENDIF(BUILD_SHARED_LIBS)
      ENDIF (CYGWIN)

  ENDIF (PNG_LIBRARY AND PNG_PNG_INCLUDE_DIR)

ENDIF(ZLIB_FOUND)

# handle the QUIETLY and REQUIRED arguments and set PNG_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PNG DEFAULT_MSG PNG_LIBRARY PNG_PNG_INCLUDE_DIR)

MARK_AS_ADVANCED(PNG_PNG_INCLUDE_DIR PNG_LIBRARY )
