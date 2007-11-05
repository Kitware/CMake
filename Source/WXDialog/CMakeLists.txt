##---------------------------------------------------------------------------
## $RCSfile$
## $Source$
## $Revision$
## $Date$
##---------------------------------------------------------------------------
## Author:      Jorgen Bodde
## Copyright:   (c) Jorgen Bodde
## License:     wxWidgets License
##---------------------------------------------------------------------------

# wxWindows -> wxWidgets Jan Woetzel 07/2006
# tested with wx 2.6.3 with "multilib" build on Windows XP

# CMake Options
SET(CMAKE_VERBOSE_MAKEFILE TRUE)

# suppress wx header warnings? 
#SET(HAVE_ISYSTEM 1)

# in addition to wx std libs we need
SET( wxWidgets_USE_LIBS 
  std 
  html adv xml xrc )
FIND_PACKAGE(wxWidgets REQUIRED)
#INCLUDE(${CMAKE_ROOT}/Modules/FindwxWidgets.cmake)

# Start using wx stuff when it is fully found and set
IF(wxWidgets_FOUND)
  INCLUDE( ${wxWidgets_USE_FILE} )

  SET (CMAKE_SRCS 
    CMakeSetupFrame.cpp
    CMakeSetup.cpp
    CMakeSetupFrame.h
    CMakeSetup.h 
    PropertyList.cpp
    PropertyList.h
    app_resources.cpp
    app_resources.h
    progressdlg.cpp
    progressdlg.h 
    aboutdlg.cpp
    aboutdlg.h 
    optionsdlg.cpp
    optionsdlg.h 
    config.h 
    CommandLineInfo.cpp
    CommandLineInfo.h )
  
  # include .rc when windows    
  IF(WIN32)
    SET ( CMAKE_SRCS ${CMAKE_SRCS}  CMakeSetup.rc )
  ENDIF(WIN32)
  
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/Source/WXDialog
    ${wxWidgets_INCLUDE_DIR} )
  LINK_DIRECTORIES( ${wxWidgets_LINK_DIRECTORIES} 
    ${CMake_BINARY_DIR}/Source 
    ${CMake_BINARY_DIR}/Source/kwsys )
  
  IF(APPLE)
    ADD_EXECUTABLE(WXDialog MACOSX_BUNDLE  ${CMAKE_SRCS})
  ELSE(APPLE)
    ADD_EXECUTABLE(WXDialog WIN32  ${CMAKE_SRCS})
  ENDIF(APPLE)
  INSTALL_TARGETS(/bin WXDialog)
  
  # not required on MSVS beause bound to _DEBUG, 
  # but other compiler may need it.
  # However, the define prevents multiple build configurations 
  # in one build tree,
  # e.g. MSVS supports Debug;Release
  # TODO: shall we add the define or not - 
  # favor multi config or all compilers? (JW)
  SET(CMAKE_CXX_FLAGS_DEBUG 
    "${CMAKE_CXX_FLAGS_DEBUG} -D__WXDEBUG__ -DWXDEBUG=1")
  
  IF(LINUX)
    ADD_DEFINITIONS( -DLINUX=1 )
  ENDIF(LINUX)
  
  # if the checkbox view functionality is not desired,
  # exclude this part from the smple
  TARGET_LINK_LIBRARIES(WXDialog ${wxWidgets_LIBRARIES} cmsys CMakeLib)
  INSTALL_TARGETS(/bin WXDialog)
ENDIF(wxWidgets_FOUND)
