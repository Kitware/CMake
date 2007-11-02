
SET(QT_MIN_VERSION "4.3.0")
FIND_PACKAGE(Qt4 REQUIRED)

IF(NOT QT4_FOUND)
  MESSAGE(SEND_ERROR "Failed to find Qt 4.3 or greater.")
ELSE(NOT QT4_FOUND)

  SET(QT_USE_QTMAIN TRUE)
  INCLUDE(${QT_USE_FILE})

  SET(SRCS
    CMakeSetup.cxx
    CMakeSetupDialog.cxx
    CMakeSetupDialog.h
    QCMake.cxx
    QCMake.h
    QCMakeCacheView.cxx
    QCMakeCacheView.h
    )

  QT4_WRAP_UI(UI_SRCS 
    CMakeSetupDialog.ui
    )
  QT4_WRAP_CPP(MOC_SRCS 
    CMakeSetupDialog.h
    QCMake.h
    QCMakeCacheView.h
    )
  QT4_ADD_RESOURCES(RC_SRCS CMakeSetup.qrc)

  SET(SRCS ${SRCS} ${UI_SRCS} ${MOC_SRCS} ${RC_SRCS})

  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

  ADD_EXECUTABLE(QtDialog WIN32 MACOSX_BUNDLE ${SRCS})
  TARGET_LINK_LIBRARIES(QtDialog CMakeLib ${QT_LIBRARIES})
  ADD_DEPENDENCIES(QtDialog cmake)

ENDIF(NOT QT4_FOUND)

