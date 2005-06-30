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


#wxWidgets build related stuff
SET(WXW_USE_DEBUG OFF)
SET(WXW_USE_UNICODE OFF)
SET(WXW_USE_SHARED OFF)
SET(WXW_USE_UNIV OFF)
SET(WXW_USE_MONO OFF)
SET(WXW_FILE_VERSION "26")
SET(WXW_VERSION "2.6")

#CMake Options
SET(CMAKE_VERBOSE_MAKEFILE TRUE)

IF(WIN32)
    INCLUDE (${CMAKE_SOURCE_DIR}/Source/WXDialog/bin/FindwxW.cmake)
ELSE(WIN32)
    INCLUDE (${CMAKE_SOURCE_DIR}/Source/WXDialog/bin/FindwxWin.cmake)
    
    # sync flags 
    SET(WXWIDGETS_FOUND            ${WXWINDOWS_FOUND})
    SET(WXWIDGETS_INCLUDE_DIR      ${WXWINDOWS_INCLUDE_DIR})
    SET(WXWIDGETS_LINK_DIRECTORIES ${WXWINDOWS_LINK_DIRECTORIES})
    SET(WXWIDGETS_LIBRARIES        ${WXWINDOWS_LIBRARIES})
ENDIF(WIN32)

# Old find script

## Start using wx stuff when it is fully found and set
IF(WXWIDGETS_FOUND)

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
        SET ( CMAKE_SRCS 
                ${CMAKE_SRCS} 
                CMakeSetup.rc )
    ENDIF(WIN32)
     
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/Source/WXDialog
                        ${WXWIDGETS_INCLUDE_DIR})
    LINK_DIRECTORIES( ${WXWIDGETS_LINK_DIRECTORIES} ${CMake_BINARY_DIR}/Source ${CMake_BINARY_DIR}/Source/kwsys )

    ADD_EXECUTABLE(WXDialog WIN32  ${CMAKE_SRCS})
        
    SET(CMAKE_CXX_FLAGS "${CMAKE_WX_CXX_FLAGS}")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D__WXDEBUG__ -DWXDEBUG=1")
                                     
    IF(LINUX)
      ADD_DEFINITIONS( -DLINUX=1 )
    ENDIF(LINUX)
    
    # if the checkbox view functionality is not desired,
    # exclude this part from the sample
    
    TARGET_LINK_LIBRARIES(WXDialog ${WXWIDGETS_LIBRARIES} cmsys CMakeLib)
        
    # if UPX is found, make a target
    
    #INCLUDE (${CMAKE_SOURCE_DIR}/Source/WXDialog/bin/FindUPX.cmake)    
    #IF(UPX_FOUND)
    #END(UPX_FOUND)
        
ENDIF(WXWIDGETS_FOUND)
