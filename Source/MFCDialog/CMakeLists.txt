SET( SRCS 
  CMakeSetup.h
  CMakeSetup.cpp
  MakeHelp.h
  MakeHelp.cpp
  CMakeGenDialog.h
  CMakeGenDialog.cpp
  CMakeSetup.rc
  CMakeSetupDialog.h
  CMakeSetupDialog.cpp
  PathDialog.h
  PathDialog.cpp
  PropertyList.cpp
  StdAfx.h
  StdAfx.cpp
  resource.h
  CMakeCommandLineInfo.cpp
)

# add stuff to use MFC in this executable
ADD_DEFINITIONS(-D_AFXDLL)

# Use of CMAKE_MFC_FLAG 
# Values: 
# 0: Use Standard Windows Libraries 
# 1: Use MFC in a Static Library 
# 2: Use MFC in a Shared DLL 
SET(CMAKE_MFC_FLAG 2)

ADD_EXECUTABLE(CMakeSetup WIN32 ${SRCS})
TARGET_LINK_LIBRARIES(CMakeSetup CMakeLib)
ADD_DEPENDENCIES(CMakeSetup cmake)
INSTALL_TARGETS(/bin CMakeSetup)
