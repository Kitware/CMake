SET(UNIX 1)

# also add the install directory of the running cmake to the search directories
# CMAKE_ROOT is CMAKE_INSTALL_PREFIX/share/cmake, so we need to go two levels up
GET_FILENAME_COMPONENT(_CMAKE_INSTALL_DIR "${CMAKE_ROOT}" PATH)
GET_FILENAME_COMPONENT(_CMAKE_INSTALL_DIR "${_CMAKE_INSTALL_DIR}" PATH)

# List common installation prefixes.  These will be used for all
# search types.
LIST(APPEND CMAKE_SYSTEM_PREFIX_PATH
  # Standard
  / /usr /usr/local

  # CMake install location
  "${_CMAKE_INSTALL_DIR}"

  # Project install destination.
  "${CMAKE_INSTALL_PREFIX}"
  )

# List common include file locations not under the common prefixes.
LIST(APPEND CMAKE_SYSTEM_INCLUDE_PATH
  # Windows API on Cygwin
  /usr/include/w32api

  # X11
  /usr/X11R6/include /usr/include/X11

  # Other
  /opt/local/include /usr/pkg/include
  /opt/csw/include /opt/include  
  /usr/openwin/include
  )

LIST(APPEND CMAKE_SYSTEM_LIBRARY_PATH
  # Windows API on Cygwin
  /usr/lib/w32api

  # X11
  /usr/X11R6/lib /usr/lib/X11

  # Other
  /opt/local/lib /usr/pkg/lib
  /opt/csw/lib /opt/lib 
  /usr/openwin/lib
  )

LIST(APPEND CMAKE_SYSTEM_PROGRAM_PATH
  /usr/pkg/bin
  )

LIST(APPEND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
  /lib /usr/lib /usr/lib32 /usr/lib64
  )

# Enable use of lib64 search path variants by default.
SET_PROPERTY(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS TRUE)
