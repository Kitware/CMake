# - Find X11 installation
# Try to find X11 on UNIX systems. The following values are defined
#  X11_FOUND        - True if X11 is available
#  X11_INCLUDE_DIR  - include directories to use X11
#  X11_LIBRARIES    - link against these to use X11

IF (UNIX)
  SET(X11_FOUND 0)
  # X11 is never a framework and some header files may be
  # found in tcl on the mac
  SET(CMAKE_FIND_FRAMEWORK_SAVE ${CMAKE_FIND_FRAMEWORK})
  SET(CMAKE_FIND_FRAMEWORK NEVER)
  SET(X11_INC_SEARCH_PATH
    /usr/X11R6/include 
    /usr/local/include 
    /usr/include/X11
    /usr/openwin/include 
    /usr/openwin/share/include 
    /opt/graphics/OpenGL/include
    /usr/include
  )

  SET(X11_LIB_SEARCH_PATH
    /usr/X11R6/lib
    /usr/local/lib 
    /usr/openwin/lib 
    /usr/lib 
  )

  FIND_PATH(X11_X11_INCLUDE_PATH X11/X.h ${X11_INC_SEARCH_PATH})
  FIND_PATH(X11_Xlib_INCLUDE_PATH X11/Xlib.h ${X11_INC_SEARCH_PATH})
  FIND_PATH(X11_Xutil_INCLUDE_PATH X11/Xutil.h ${X11_INC_SEARCH_PATH})
  FIND_LIBRARY(X11_X11_LIB X11 ${X11_LIB_SEARCH_PATH})
  FIND_LIBRARY(X11_Xext_LIB Xext ${X11_LIB_SEARCH_PATH})

  IF(X11_X11_INCLUDE_PATH)
    SET(X11_INCLUDE_DIR ${X11_INCLUDE_DIR} ${X11_X11_INCLUDE_PATH})
  ENDIF(X11_X11_INCLUDE_PATH)

  IF(X11_Xlib_INCLUDE_PATH)
    SET(X11_INCLUDE_DIR ${X11_INCLUDE_DIR} ${X11_Xlib_INCLUDE_PATH})
  ENDIF(X11_Xlib_INCLUDE_PATH)

  IF(X11_Xutil_INCLUDE_PATH)
    SET(X11_INCLUDE_DIR ${X11_INCLUDE_DIR} ${X11_Xutil_INCLUDE_PATH})
  ENDIF(X11_Xutil_INCLUDE_PATH)

  IF(X11_X11_LIB)
    SET(X11_LIBRARIES ${X11_LIBRARIES} ${X11_X11_LIB})
  ENDIF(X11_X11_LIB)

  IF(X11_Xext_LIB)
    SET(X11_LIBRARIES ${X11_LIBRARIES} ${X11_Xext_LIB})
  ENDIF(X11_Xext_LIB)

  # Deprecated variable for backwards compatibility with CMake 1.4
  IF(X11_X11_INCLUDE_PATH)
    IF(X11_LIBRARIES)
      SET(X11_FOUND 1)
    ENDIF(X11_LIBRARIES)
  ENDIF(X11_X11_INCLUDE_PATH)

  SET(X11_LIBRARY_DIR "")
  IF(X11_X11_LIB)
    GET_FILENAME_COMPONENT(X11_LIBRARY_DIR ${X11_X11_LIB} PATH)
  ENDIF(X11_X11_LIB)

  IF(X11_FOUND)
    INCLUDE(CheckFunctionExists)
    INCLUDE(CheckLibraryExists)

    # Translated from an autoconf-generated configure script.
    # See libs.m4 in autoconf's m4 directory.
    IF($ENV{ISC} MATCHES "^yes$")
      SET(X11_X_EXTRA_LIBS -lnsl_s -linet)
    ELSE($ENV{ISC} MATCHES "^yes$")
      SET(X11_X_EXTRA_LIBS "")

      # See if XOpenDisplay in X11 works by itself.
      CHECK_LIBRARY_EXISTS("${X11_LIBRARIES}" "XOpenDisplay" "${X11_LIBRARY_DIR}" X11_LIB_X11_SOLO)
      IF(NOT X11_LIB_X11_SOLO)
        # Find library needed for dnet_ntoa.
        CHECK_LIBRARY_EXISTS("dnet" "dnet_ntoa" "" X11_LIB_DNET_HAS_DNET_NTOA) 
        IF (X11_LIB_DNET_HAS_DNET_NTOA)
          SET (X11_X_EXTRA_LIBS ${X11_X_EXTRA_LIBS} -ldnet)
        ELSE (X11_LIB_DNET_HAS_DNET_NTOA)
          CHECK_LIBRARY_EXISTS("dnet_stub" "dnet_ntoa" "" X11_LIB_DNET_STUB_HAS_DNET_NTOA) 
          IF (X11_LIB_DNET_STUB_HAS_DNET_NTOA)
            SET (X11_X_EXTRA_LIBS ${X11_X_EXTRA_LIBS} -ldnet_stub)
          ENDIF (X11_LIB_DNET_STUB_HAS_DNET_NTOA)
        ENDIF (X11_LIB_DNET_HAS_DNET_NTOA)
      ENDIF(NOT X11_LIB_X11_SOLO)

      # Find library needed for gethostbyname.
      CHECK_FUNCTION_EXISTS("gethostbyname" CMAKE_HAVE_GETHOSTBYNAME)
      IF(NOT CMAKE_HAVE_GETHOSTBYNAME)
        CHECK_LIBRARY_EXISTS("nsl" "gethostbyname" "" CMAKE_LIB_NSL_HAS_GETHOSTBYNAME) 
        IF (CMAKE_LIB_NSL_HAS_GETHOSTBYNAME)
          SET (X11_X_EXTRA_LIBS ${X11_X_EXTRA_LIBS} -lnsl)
        ELSE (CMAKE_LIB_NSL_HAS_GETHOSTBYNAME)
          CHECK_LIBRARY_EXISTS("bsd" "gethostbyname" "" CMAKE_LIB_BSD_HAS_GETHOSTBYNAME) 
          IF (CMAKE_LIB_BSD_HAS_GETHOSTBYNAME)
            SET (X11_X_EXTRA_LIBS ${X11_X_EXTRA_LIBS} -lbsd)
          ENDIF (CMAKE_LIB_BSD_HAS_GETHOSTBYNAME)
        ENDIF (CMAKE_LIB_NSL_HAS_GETHOSTBYNAME)
      ENDIF(NOT CMAKE_HAVE_GETHOSTBYNAME)

      # Find library needed for connect.
      CHECK_FUNCTION_EXISTS("connect" CMAKE_HAVE_CONNECT)
      IF(NOT CMAKE_HAVE_CONNECT)
        CHECK_LIBRARY_EXISTS("socket" "connect" "" CMAKE_LIB_SOCKET_HAS_CONNECT) 
        IF (CMAKE_LIB_SOCKET_HAS_CONNECT)
          SET (X11_X_EXTRA_LIBS -lsocket ${X11_X_EXTRA_LIBS})
        ENDIF (CMAKE_LIB_SOCKET_HAS_CONNECT)
      ENDIF(NOT CMAKE_HAVE_CONNECT)

      # Find library needed for remove.
      CHECK_FUNCTION_EXISTS("remove" CMAKE_HAVE_REMOVE)
      IF(NOT CMAKE_HAVE_REMOVE)
        CHECK_LIBRARY_EXISTS("posix" "remove" "" CMAKE_LIB_POSIX_HAS_REMOVE) 
        IF (CMAKE_LIB_POSIX_HAS_REMOVE)
          SET (X11_X_EXTRA_LIBS ${X11_X_EXTRA_LIBS} -lposix)
        ENDIF (CMAKE_LIB_POSIX_HAS_REMOVE)
      ENDIF(NOT CMAKE_HAVE_REMOVE)

      # Find library needed for shmat.
      CHECK_FUNCTION_EXISTS("shmat" CMAKE_HAVE_SHMAT)
      IF(NOT CMAKE_HAVE_SHMAT)
        CHECK_LIBRARY_EXISTS("ipc" "shmat" "" CMAKE_LIB_IPS_HAS_SHMAT) 
        IF (CMAKE_LIB_IPS_HAS_SHMAT)
          SET (X11_X_EXTRA_LIBS ${X11_X_EXTRA_LIBS} -lipc)
        ENDIF (CMAKE_LIB_IPS_HAS_SHMAT)
      ENDIF(NOT CMAKE_HAVE_SHMAT)
    ENDIF($ENV{ISC} MATCHES "^yes$")

    CHECK_LIBRARY_EXISTS("ICE" "IceConnectionNumber" "${X11_LIBRARY_DIR}"
                         CMAKE_LIB_ICE_HAS_ICECONNECTIONNUMBER)
    IF(CMAKE_LIB_ICE_HAS_ICECONNECTIONNUMBER)
      SET (X11_X_PRE_LIBS -lSM -lICE)
    ENDIF(CMAKE_LIB_ICE_HAS_ICECONNECTIONNUMBER)
    # Build the final list of libraries.
    SET (X11_LIBRARIES ${X11_X_PRE_LIBS} ${X11_LIBRARIES} ${X11_X_EXTRA_LIBS})
  ENDIF(X11_FOUND)

  MARK_AS_ADVANCED(
    X11_X11_INCLUDE_PATH
    X11_X11_LIB
    X11_Xext_LIB
    X11_Xlib_INCLUDE_PATH
    X11_Xutil_INCLUDE_PATH
    X11_LIBRARIES
    )
  SET(CMAKE_FIND_FRAMEWORK ${CMAKE_FIND_FRAMEWORK_SAVE})
ENDIF (UNIX)
