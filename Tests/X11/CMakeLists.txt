# a simple C only test case
cmake_minimum_required (VERSION 2.6)
PROJECT (UseX11 CXX C)

INCLUDE (${CMAKE_ROOT}/Modules/FindX11.cmake)
MESSAGE("X11_FOUND: ${X11_FOUND}")

ADD_EXECUTABLE (UseX11 X11.c)

# so for universal binaries this test will fail if 
# 
IF(APPLE)
  LIST(LENGTH CMAKE_OSX_ARCHITECTURES NUMARCH)
  IF(NUMARCH GREATER 1)
    IF(NOT EXISTS /usr/X11R6/lib//libSM.6.dylib)
      SET(X11_FOUND FALSE)
      MESSAGE("disable X11, because of universal binary and sysroot")
    ENDIF(NOT EXISTS /usr/X11R6/lib//libSM.6.dylib)
  ENDIF(NUMARCH GREATER 1)
ENDIF(APPLE)

IF(X11_FOUND)
  ADD_DEFINITIONS(-DCMAKE_HAS_X)
  INCLUDE_DIRECTORIES(${X11_INCLUDE_DIR})
  TARGET_LINK_LIBRARIES(UseX11 ${X11_LIBRARIES})
  IF(APPLE)
    ADD_EXECUTABLE(HelloWorldX11 HelloWorldX11.cxx)
    TARGET_LINK_LIBRARIES(HelloWorldX11 ${X11_LIBRARIES})
    install( TARGETS HelloWorldX11 DESTINATION bin)
    # build a CPack driven installer package
    set(CPACK_PACKAGE_NAME HelloWorldX11Package)
    set(CPACK_PACKAGE_EXECUTABLES HelloWorldX11 HelloWorldX11)
    include(CPack)
  ENDIF(APPLE)
ENDIF(X11_FOUND)
