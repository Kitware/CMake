PROJECT (TestSimpleInstall)

#SET(EXECUTABLE_OUTPUT_PATH "${TestSimpleInstall_BINARY_DIR}/This is exec path")
SET(EXECUTABLE_OUTPUT_PATH "${TestSimpleInstall_BINARY_DIR}/ExecPath")
SET(LIBRARY_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}")

SET(CMAKE_DEBUG_POSTFIX "_test_debug_postfix")
ADD_LIBRARY(test1 STATIC lib1.cxx)
ADD_LIBRARY(test2 SHARED lib2.cxx)
ADD_LIBRARY(test3 MODULE lib3.cxx)
ADD_EXECUTABLE (SimpleInstall inst.cxx foo.c foo.h)
TARGET_LINK_LIBRARIES(SimpleInstall test1 test2 test3)

INSTALL_TARGETS(/bin SimpleInstall)
INSTALL_TARGETS(/lib test1 test2 test3)
INSTALL_TARGETS(/include lib1.h lib2.h lib3.h)

SET(EXTRA_INSTALL_FLAGS)
IF(CMAKE_GENERATOR MATCHES "^Visual Studio")
  MESSAGE("CMake generator: ${CMAKE_GENERATOR}")
  FOREACH(build_type Debug Release RelWithDebInfo MinSizeRel)
    IF(MAKEPROGRAM MATCHES "/build ${build_type}")
      SET(EXTRA_INSTALL_FLAGS -DBUILD_TYPE=${build_type})
    ENDIF(MAKEPROGRAM MATCHES "/build ${build_type}")
    IF(MAKEPROGRAM MATCHES "- ${build_type}")
      SET(EXTRA_INSTALL_FLAGS -DBUILD_TYPE=${build_type})
    ENDIF(MAKEPROGRAM MATCHES "- ${build_type}")
  ENDFOREACH(build_type)
  IF(NOT EXTRA_INSTALL_FLAGS)
    IF(BUILD_TYPE)
      SET(EXTRA_INSTALL_FLAGS -DBUILD_TYPE=${BUILD_TYPE})
    ENDIF(BUILD_TYPE)
  ENDIF(NOT EXTRA_INSTALL_FLAGS)
ENDIF(CMAKE_GENERATOR MATCHES "^Visual Studio")

MESSAGE("Extra install: ${EXTRA_INSTALL_FLAGS}")

ADD_CUSTOM_COMMAND(
  TARGET SimpleInstall
  POST_BUILD
  COMMAND ${CMAKE_COMMAND}
  ARGS ${EXTRA_INSTALL_FLAGS} -Pcmake_install.cmake
  COMMENT "Install Project"
  )
