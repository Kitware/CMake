# also add the install directory of the running cmake to the search directories
# CMAKE_ROOT is CMAKE_INSTALL_PREFIX/share/cmake, so we need to go two levels up
GET_FILENAME_COMPONENT(_CMAKE_INSTALL_DIR "${CMAKE_ROOT}" PATH)
GET_FILENAME_COMPONENT(_CMAKE_INSTALL_DIR "${_CMAKE_INSTALL_DIR}" PATH)

SET(CMAKE_SYSTEM_INCLUDE_PATH ${CMAKE_SYSTEM_INCLUDE_PATH}
  # Standard
  /include /usr/include /usr/local/include

  # Windows API on Cygwin
  /usr/include/w32api

  # X11
  /usr/X11R6/include /usr/include/X11

  # Other
  /opt/local/include /usr/pkg/include
  /opt/csw/include /opt/include  
  /usr/openwin/include
  "${_CMAKE_INSTALL_DIR}/include"
  "${CMAKE_INSTALL_PREFIX}/include"
  )

SET(CMAKE_SYSTEM_LIBRARY_PATH ${CMAKE_SYSTEM_LIBRARY_PATH}
  # Standard
  /lib     /usr/lib     /usr/local/lib

  # Windows API on Cygwin
  /usr/lib/w32api

  # X11
  /usr/X11R6/lib /usr/lib/X11

  # Other
  /opt/local/lib /usr/pkg/lib
  /opt/csw/lib /opt/lib
  /usr/openwin/lib
  "${_CMAKE_INSTALL_DIR}/lib"
  "${CMAKE_INSTALL_PREFIX}/lib"
  )

SET(CMAKE_SYSTEM_PROGRAM_PATH ${CMAKE_SYSTEM_PROGRAM_PATH}
  /bin /usr/bin /usr/local/bin /usr/pkg/bin /sbin
  "${_CMAKE_INSTALL_DIR}/bin"
  "${CMAKE_INSTALL_PREFIX}/bin"
  )

SET(CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
  ${CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES}
  /lib /usr/lib /usr/lib32 /usr/lib64
  )

