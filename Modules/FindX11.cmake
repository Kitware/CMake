#
# try to find X11 on UNIX systems.
#
# The following values are defined
# CMAKE_HAS_X           - True if X11 is available
# CMAKE_X_INCLUDE_DIRS  - include directories to use X11
# CMAKE_X_LIBS          - link against these to use X11

IF (UNIX)
  SET(CMAKE_HAS_X 0)

  SET(CMAKE_X11_INC_SEARCH_PATH
    /usr/X11R6/include 
    /usr/local/include 
    /usr/include/X11
    /usr/openwin/include 
    /usr/openwin/share/include 
    /opt/graphics/OpenGL/include
    /usr/include
  )

  SET(CMAKE_X11_LIB_SEARCH_PATH
    /usr/X11R6/lib
    /usr/local/lib 
    /usr/openwin/lib 
    /usr/lib 
  )

  FIND_PATH(CMAKE_X11_INCLUDE_PATH X11/X.h ${CMAKE_X11_INC_SEARCH_PATH})
  FIND_PATH(CMAKE_Xlib_INCLUDE_PATH X11/Xlib.h ${CMAKE_X11_INC_SEARCH_PATH})
  FIND_PATH(CMAKE_Xutil_INCLUDE_PATH X11/Xutil.h ${CMAKE_X11_INC_SEARCH_PATH})
  FIND_LIBRARY(CMAKE_X11_LIB X11 ${CMAKE_X11_LIB_SEARCH_PATH})
  FIND_LIBRARY(CMAKE_Xext_LIB Xext ${CMAKE_X11_LIB_SEARCH_PATH})

  IF(CMAKE_X11_INCLUDE_PATH)
    SET(CMAKE_X_INCLUDE_DIRS ${CMAKE_X_INCLUDE_DIRS} ${CMAKE_X11_INCLUDE_PATH})
  ENDIF(CMAKE_X11_INCLUDE_PATH)

  IF(CMAKE_Xlib_INCLUDE_PATH)
    SET(CMAKE_X_INCLUDE_DIRS ${CMAKE_X_INCLUDE_DIRS} ${CMAKE_Xlib_INCLUDE_PATH})
  ENDIF(CMAKE_Xlib_INCLUDE_PATH)

  IF(CMAKE_Xutil_INCLUDE_PATH)
    SET(CMAKE_X_INCLUDE_DIRS ${CMAKE_X_INCLUDE_DIRS} ${CMAKE_Xutil_INCLUDE_PATH})
  ENDIF(CMAKE_Xutil_INCLUDE_PATH)

  IF(CMAKE_X11_LIB)
    SET(CMAKE_X_LIBS ${CMAKE_X_LIBS} ${CMAKE_X11_LIB})
  ENDIF(CMAKE_X11_LIB)

  IF(CMAKE_Xext_LIB)
    SET(CMAKE_X_LIBS ${CMAKE_X_LIBS} ${CMAKE_Xext_LIB})
  ENDIF(CMAKE_Xext_LIB)

  # Deprecated variable for backwards compatibility with CMake 1.4
  IF(CMAKE_X11_INCLUDE_PATH)
    IF(CMAKE_X_LIBS)
      SET(CMAKE_HAS_X 1)
    ENDIF(CMAKE_X_LIBS)
  ENDIF(CMAKE_X11_INCLUDE_PATH)

  IF(CMAKE_HAS_X)
    INCLUDE(${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
    INCLUDE(${CMAKE_ROOT}/Modules/CheckLibraryExists.cmake)

    # Translated from an autoconf-generated configure script.
    # See libs.m4 in autoconf's m4 directory.
    IF($ENV{ISC} MATCHES "^yes$")
      SET(CMAKE_X_EXTRA_LIBS -lnsl_s -linet)
    ELSE($ENV{ISC} MATCHES "^yes$")
      SET(CMAKE_X_EXTRA_LIBS "")

      # See if XOpenDisplay in X11 works by itself.
      CHECK_LIBRARY_EXISTS("${CMAKE_X_LIBS}" "XOpenDisplay" "" CMAKE_LIB_X11_SOLO)
      IF(NOT CMAKE_LIB_X11_SOLO)
        # Find library needed for dnet_ntoa.
        CHECK_LIBRARY_EXISTS("dnet" "dnet_ntoa" "" CMAKE_LIB_DNET_HAS_DNET_NTOA) 
        IF (CMAKE_LIB_DNET_HAS_DNET_NTOA)
          SET (CMAKE_X_EXTRA_LIBS ${CMAKE_X_EXTRA_LIBS} -ldnet)
        ELSE (CMAKE_LIB_DNET_HAS_DNET_NTOA)
          CHECK_LIBRARY_EXISTS("dnet_stub" "dnet_ntoa" "" CMAKE_LIB_DNET_STUB_HAS_DNET_NTOA) 
          IF (CMAKE_LIB_DNET_STUB_HAS_DNET_NTOA)
            SET (CMAKE_X_EXTRA_LIBS ${CMAKE_X_EXTRA_LIBS} -ldnet_stub)
          ENDIF (CMAKE_LIB_DNET_STUB_HAS_DNET_NTOA)
        ENDIF (CMAKE_LIB_DNET_HAS_DNET_NTOA)
      ENDIF(NOT CMAKE_LIB_X11_SOLO)

      # Find library needed for gethostbyname.
      CHECK_FUNCTION_EXISTS("gethostbyname" CMAKE_HAVE_GETHOSTBYNAME)
      IF(NOT CMAKE_HAVE_GETHOSTBYNAME)
        CHECK_LIBRARY_EXISTS("nsl" "gethostbyname" "" CMAKE_LIB_NSL_HAS_GETHOSTBYNAME) 
        IF (CMAKE_LIB_NSL_HAS_GETHOSTBYNAME)
          SET (CMAKE_X_EXTRA_LIBS ${CMAKE_X_EXTRA_LIBS} -lnsl)
        ELSE (CMAKE_LIB_NSL_HAS_GETHOSTBYNAME)
          CHECK_LIBRARY_EXISTS("bsd" "gethostbyname" "" CMAKE_LIB_BSD_HAS_GETHOSTBYNAME) 
          IF (CMAKE_LIB_BSD_HAS_GETHOSTBYNAME)
            SET (CMAKE_X_EXTRA_LIBS ${CMAKE_X_EXTRA_LIBS} -lbsd)
          ENDIF (CMAKE_LIB_BSD_HAS_GETHOSTBYNAME)
        ENDIF (CMAKE_LIB_NSL_HAS_GETHOSTBYNAME)
      ENDIF(NOT CMAKE_HAVE_GETHOSTBYNAME)

      # Find library needed for connect.
      CHECK_FUNCTION_EXISTS("connect" CMAKE_HAVE_CONNECT)
      IF(NOT CMAKE_HAVE_CONNECT)
        CHECK_LIBRARY_EXISTS("socket" "connect" "" CMAKE_LIB_SOCKET_HAS_CONNECT) 
        IF (CMAKE_LIB_SOCKET_HAS_CONNECT)
          SET (CMAKE_X_EXTRA_LIBS -lsocket ${CMAKE_X_EXTRA_LIBS})
        ENDIF (CMAKE_LIB_SOCKET_HAS_CONNECT)
      ENDIF(NOT CMAKE_HAVE_CONNECT)

      # Find library needed for remove.
      CHECK_FUNCTION_EXISTS("remove" CMAKE_HAVE_REMOVE)
      IF(NOT CMAKE_HAVE_REMOVE)
        CHECK_LIBRARY_EXISTS("posix" "remove" "" CMAKE_LIB_POSIX_HAS_REMOVE) 
        IF (CMAKE_LIB_POSIX_HAS_REMOVE)
          SET (CMAKE_X_EXTRA_LIBS ${CMAKE_X_EXTRA_LIBS} -lposix)
        ENDIF (CMAKE_LIB_POSIX_HAS_REMOVE)
      ENDIF(NOT CMAKE_HAVE_REMOVE)

      # Find library needed for shmat.
      CHECK_FUNCTION_EXISTS("shmat" CMAKE_HAVE_SHMAT)
      IF(NOT CMAKE_HAVE_SHMAT)
        CHECK_LIBRARY_EXISTS("ipc" "shmat" "" CMAKE_LIB_IPS_HAS_SHMAT) 
        IF (CMAKE_LIB_IPS_HAS_SHMAT)
          SET (CMAKE_X_EXTRA_LIBS ${CMAKE_X_EXTRA_LIBS} -lipc)
        ENDIF (CMAKE_LIB_IPS_HAS_SHMAT)
      ENDIF(NOT CMAKE_HAVE_SHMAT)
    ENDIF($ENV{ISC} MATCHES "^yes$")

    CHECK_LIBRARY_EXISTS("ICE" "IceConnectionNumber" ""
                         CMAKE_LIB_ICE_HAS_ICECONNECTIONNUMBER)
    IF(CMAKE_LIB_ICE_HAS_ICECONNECTIONNUMBER)
      SET (CMAKE_X_PRE_LIBS -lSM -lICE)
    ENDIF(CMAKE_LIB_ICE_HAS_ICECONNECTIONNUMBER)

    # Build the final list of libraries.
    SET (CMAKE_X_LIBS ${CMAKE_X_PRE_LIBS} ${CMAKE_X_LIBS} ${CMAKE_X_EXTRA_LIBS})
  ENDIF(CMAKE_HAS_X)

  MARK_AS_ADVANCED(
    CMAKE_X11_INCLUDE_PATH
    CMAKE_X11_LIB
    CMAKE_Xext_LIB
    CMAKE_Xlib_INCLUDE_PATH
    CMAKE_Xutil_INCLUDE_PATH
    CMAKE_X_LIBS
    )

ENDIF (UNIX)
