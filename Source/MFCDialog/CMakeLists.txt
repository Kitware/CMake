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

IF(MSVC AND NOT MSVC60 AND NOT MSVC70 AND NOT MSVC71)
  SET(exe "${CMAKE_CFG_INTDIR}/CMakeSetup.exe")
  IF(EXECUTABLE_OUTPUT_PATH)
    SET(exe "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/CMakeSetup.exe")
  ENDIF(EXECUTABLE_OUTPUT_PATH)

  # Solve the "things named like *Setup prompt for admin privileges
  # on Vista" problem by merging a manifest fragment that contains a
  # requestedExecutionLevel element:
  #
  ADD_CUSTOM_COMMAND(TARGET CMakeSetup
    POST_BUILD COMMAND mt
    "-inputresource:${exe};#1"
    -manifest "${CMAKE_CURRENT_SOURCE_DIR}/CMakeSetupManifest.xml"
    "-outputresource:${exe};#1"
    VERBATIM 
  )
ENDIF(MSVC AND NOT MSVC60 AND NOT MSVC70 AND NOT MSVC71)

INSTALL_TARGETS(/bin CMakeSetup)
