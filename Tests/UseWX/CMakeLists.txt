PROJECT (UsewxWindows)

INCLUDE( ${CMAKE_ROOT}/Modules/Use_wxWindows.cmake)

ADD_EXECUTABLE (UseWX WX.cxx)

#TODO: 
#- dll linking is not working on WIN32, need to '-DWXUSINGDLL' to preprocessor

