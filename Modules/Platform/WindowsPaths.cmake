GET_FILENAME_COMPONENT(_CMAKE_INSTALL_DIR "${CMAKE_ROOT}" PATH)
GET_FILENAME_COMPONENT(_CMAKE_INSTALL_DIR "${_CMAKE_INSTALL_DIR}" PATH)

# List common installation prefixes.  These will be used for all
# search types.
LIST(APPEND CMAKE_SYSTEM_PREFIX_PATH
  # Standard
  "$ENV{ProgramFiles}"

  # CMake install location
  "${_CMAKE_INSTALL_DIR}"

  # Project install destination.
  "${CMAKE_INSTALL_PREFIX}"

  # MinGW (useful when cross compiling from linux with CMAKE_FIND_ROOT_PATH set)
  /
  )

LIST(APPEND CMAKE_SYSTEM_INCLUDE_PATH
  )

# mingw can also link against dlls which can also be in /bin, so list this too
LIST(APPEND CMAKE_SYSTEM_LIBRARY_PATH
  "${CMAKE_INSTALL_PREFIX}/bin"
  "${_CMAKE_INSTALL_DIR}/bin"
  /bin
  )

LIST(APPEND CMAKE_SYSTEM_PROGRAM_PATH
  )
