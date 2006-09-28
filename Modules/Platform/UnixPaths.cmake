SET(CMAKE_SYSTEM_INCLUDE_PATH ${CMAKE_SYSTEM_INCLUDE_PATH}
  # Standard
  /include /usr/include /usr/local/include

  # Windows API on Cygwin
  /usr/include/w32api

  # X11
  /usr/X11R6/include /usr/include/X11

  # Other
  /opt/local/include /usr/pkg/include
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
  )

SET(CMAKE_SYSTEM_PROGRAM_PATH ${CMAKE_SYSTEM_PROGRAM_PATH}
  /bin /usr/bin /usr/local/bin /usr/pkg/bin /sbin
  )
