INCLUDE (${CMAKE_ROOT}/Modules/FindwxWindows.cmake)

IF ( CMAKE_WX_CAN_COMPILE )

  SET( WX_SRCS
       cmWXCacheProperty.cxx
       cmWXCommandLineInfo.cxx
       cmWXMainFrame.cxx
       wxCMakeSetup.cxx
     )

  IF(WXWINDOWS_INCLUDE_PATH)
    INCLUDE_DIRECTORIES(${WXWINDOWS_INCLUDE_PATH})
  ENDIF(WXWINDOWS_INCLUDE_PATH)

  IF(CMAKE_WX_CXX_FLAGS)
    SET(CMAKE_CXX_FLAGS "${CMAKE_WX_CXX_FLAGS} ${CMAKE_CXX_FLAGS}")
  ENDIF(CMAKE_WX_CXX_FLAGS)

  INCLUDE_DIRECTORIES(${CMake_SOURCE_DIR}/Source)

  IF(WIN32)
    SET(WIN32_EXECUTABLE WIN32)
  ENDIF(WIN32)

  ADD_EXECUTABLE(wxCMakeSetup ${WIN32_EXECUTABLE} ${WX_SRCS})
  TARGET_LINK_LIBRARIES(wxCMakeSetup ${WXWINDOWS_LIBRARY})
  TARGET_LINK_LIBRARIES(wxCMakeSetup CMakeLib)

  IF(APPLE)
    SET(APPLE_GUI_EXECUTABLE wxCMakeSetup)
    SET(APPLE_GUI_INFO_STRING "CMake version 1.5.0, (c) 2002 Kitware Inc.")
    SET(APPLE_GUI_IDENTIFIER "com.kitware.wxCMakeSetup")
    SET(APPLE_GUI_LONG_VERSION_STRING "1.5.0, (c) 2002 Kitware Inc.")
    SET(APPLE_GUI_BUNDLE_NAME "CMake")
    SET(APPLE_GUI_SHORT_VERSION_STRING "1.5.0")
    SET(APPLE_GUI_BUNDLE_VERSION "1.5.0")
    SET(APPLE_GUI_COPYRIGHT "Copyright 2002 Kitware Inc.")
    CONFIGURE_FILE(${CMAKE_ROOT}/Templates/AppleInfo.plist
                   ${EXECUTABLE_OUTPUT_PATH}/CMake.app/Contents/Info.plist)
    MAKE_DIRECTORY(${EXECUTABLE_OUTPUT_PATH}/CMake.app/Contents/MacOS)
    #WRITE_FILE(${EXECUTABLE_OUTPUT_PATH}/CMake.app/Contents/PkgInfo "APPL????")
    ADD_EXECUTABLE(CMake.app/Contents/MacOS/wxCMakeSetup ${WIN32_EXECUTABLE} ${WX_SRCS})
    TARGET_LINK_LIBRARIES(CMake.app/Contents/MacOS/wxCMakeSetup ${WXWINDOWS_LIBRARY})
    TARGET_LINK_LIBRARIES(CMake.app/Contents/MacOS/wxCMakeSetup CMakeLib)

    ADD_CUSTOM_COMMAND(
                SOURCE CMake.app/Contents/MacOS/wxCMakeSetup
                COMMAND ${CMAKE_COMMAND}
                ARGS -E chdir ${EXECUTABLE_OUTPUT_PATH}/CMake.app/Contents/MacOS
                `${CMAKE_WX_CONFIG} --rezflags`
                ${EXECUTABLE_OUTPUT_PATH}/CMake.app/Contents/MacOS/wxCMakeSetup
                TARGET CMake.app/Contents/MacOS/wxCMakeSetup
                )
  ENDIF(APPLE)

  INSTALL_TARGETS(/bin wxCMakeSetup)

  IF(BUILD_TESTING)
    ADD_TEST(wxCMakeSetupRunTest 
             ${CMake_BINARY_DIR}/Source/wxCMakeSetup 
             -Q -A:ON -B:${CMake_BINARY_DIR}/Tests/Simple 
             -H:${CMake_SOURCE_DIR}/Tests/Simple)
  ENDIF(BUILD_TESTING)
ELSE ( CMAKE_WX_CAN_COMPILE )

  MESSAGE("Cannot find wxWindows libraries and/or header files")

ENDIF ( CMAKE_WX_CAN_COMPILE )
