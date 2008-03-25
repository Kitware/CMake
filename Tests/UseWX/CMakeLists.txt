cmake_minimum_required (VERSION 2.6)
PROJECT (UsewxWidgets)

FIND_PACKAGE(wxWidgets REQUIRED)
INCLUDE( ${wxWidgets_USE_FILE} )

ADD_EXECUTABLE (UseWX WIN32 WX.cxx)
TARGET_LINK_LIBRARIES(UseWX ${wxWidgets_LIBRARIES} )
