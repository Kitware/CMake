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
    SET(APPLE_GUI_INFO_STRING "CMake version ${CMake_VERSION_MAJOR}, (c) 2003 Kitware Inc.")
    SET(APPLE_GUI_IDENTIFIER "com.kitware.wxCMakeSetup")
    SET(APPLE_GUI_LONG_VERSION_STRING "${CMake_VERSION_MAJOR}, (c) 2003 Kitware Inc.")
    SET(APPLE_GUI_BUNDLE_NAME "CMake")
    SET(APPLE_GUI_SHORT_VERSION_STRING "${CMake_VERSION_MAJOR}")
    SET(APPLE_GUI_BUNDLE_VERSION "${CMake_VERSION_MAJOR}")
    SET(APPLE_GUI_ICON "CMakeIcon.icns")
    SET(APPLE_GUI_COPYRIGHT "Copyright 2002 Kitware Inc.")
    CONFIGURE_FILE(${CMake_SOURCE_DIR}/Templates/AppleInfo.plist
                   ${EXECUTABLE_OUTPUT_PATH}/CMake.app/Contents/Info.plist)
    CONFIGURE_FILE(${CMake_SOURCE_DIR}/Source/WXDialog/CMakeIcon.icns
                   ${EXECUTABLE_OUTPUT_PATH}/CMake.app/Contents/Resources/CMakeIcon.icns COPYONLY)
    MAKE_DIRECTORY(${EXECUTABLE_OUTPUT_PATH}/CMake.app/Contents/MacOS)
    #WRITE_FILE(${EXECUTABLE_OUTPUT_PATH}/CMake.app/Contents/PkgInfo "APPL????")
    ADD_EXECUTABLE(CMake.app/Contents/MacOS/wxCMakeSetup ${WIN32_EXECUTABLE} ${WX_SRCS})
    TARGET_LINK_LIBRARIES(CMake.app/Contents/MacOS/wxCMakeSetup ${WXWINDOWS_LIBRARY})
    TARGET_LINK_LIBRARIES(CMake.app/Contents/MacOS/wxCMakeSetup CMakeLib)

    # Support building by both CMake 1.6 and 1.8.
    IF(NOT CMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE)
      IF(CMAKE_WX_CONFIG)
        SET(CMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE ${CMAKE_WX_CONFIG})
      ENDIF(CMAKE_WX_CONFIG)
    ENDIF(NOT CMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE)

    ADD_CUSTOM_COMMAND(
                SOURCE CMake.app/Contents/MacOS/wxCMakeSetup
                COMMAND ${CMAKE_COMMAND}
                ARGS -E chdir "\"${EXECUTABLE_OUTPUT_PATH}/CMake.app/Contents/MacOS\""
                `${CMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE} --rezflags`
                "\"${EXECUTABLE_OUTPUT_PATH}/CMake.app/Contents/MacOS/wxCMakeSetup\""
                TARGET CMake.app/Contents/MacOS/wxCMakeSetup
                )
  ENDIF(APPLE)

  INSTALL_TARGETS(/bin wxCMakeSetup)

  IF(BUILD_TESTING)
    ADD_TEST(wxCMakeSetupRunTest 
             ${EXECUTABLE_OUTPUT_PATH}/wxCMakeSetup 
             -Q -A:ON -B:${CMake_BINARY_DIR}/Tests/Simple 
             -H:${CMake_SOURCE_DIR}/Tests/Simple)
  ENDIF(BUILD_TESTING)
ELSE ( CMAKE_WX_CAN_COMPILE )

  MESSAGE("Cannot find wxWindows libraries and/or header files")

ENDIF ( CMAKE_WX_CAN_COMPILE )
