SOURCE_FILES(SRCS 
CMakeSetup.cpp
CMakeSetup.rc
CMakeSetupDialog.cpp
PathDialog.cpp
PropertyList.cpp
StdAfx.cpp
CMakeCommandLineInfo.cpp
res/CMakeSetupDialog.ico
res/CMakeSetupDialog.rc2
)

# add stuff to use MFC in this executable
ADD_DEFINITIONS(-D_AFXDLL)
SET(CMAKE_MFC_FLAG 6)

ADD_EXECUTABLE(CMakeSetup WIN32 SRCS)
ADD_DEPENDENCIES(CMakeSetup cmake)
