SET( SRCS 
CMakeSetup.cpp
MakeHelp.cpp
CMakeSetup.rc
CMakeSetupDialog.cpp
PathDialog.cpp
PropertyList.cpp
StdAfx.cpp
CMakeCommandLineInfo.cpp
)

# add stuff to use MFC in this executable
ADD_DEFINITIONS(-D_AFXDLL)
SET(CMAKE_MFC_FLAG 6)

ADD_EXECUTABLE(CMakeSetup WIN32 ${SRCS})
TARGET_LINK_LIBRARIES(CMakeSetup CMakeLib)
ADD_DEPENDENCIES(CMakeSetup cmake)
