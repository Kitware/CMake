PROJECT (TestSimpleInstall)

#SET(EXECUTABLE_OUTPUT_PATH "${TestSimpleInstall_BINARY_DIR}/This is exec path")
SET(EXECUTABLE_OUTPUT_PATH "${TestSimpleInstall_BINARY_DIR}/ExecPath")
SET(LIBRARY_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}")

SET(CMAKE_DEBUG_POSTFIX "_test_debug_postfix")

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

IF(STAGE2)
  SET(LIBPATHS "${CMAKE_INSTALL_PREFIX}/lib")
  SET(t1NAMES test1 test1${CMAKE_DEBUG_POSTFIX})
  SET(t2NAMES test2 test2${CMAKE_DEBUG_POSTFIX})

  IF(CYGWIN OR MINGW)
    SET(LIBPATHS ${LIBPATHS} "${CMAKE_INSTALL_PREFIX}/bin")
  ENDIF(CYGWIN OR MINGW)
  MESSAGE("Search for library in: ${LIBPATHS}")
  FIND_LIBRARY(TEST1_LIBRARY 
    NAMES ${t1NAMES}
    PATHS ${LIBPATHS}
    DOC "First library")
  FIND_LIBRARY(TEST2_LIBRARY 
    NAMES ${t2NAMES}
    PATHS ${LIBPATHS}
    DOC "First library")

  ADD_EXECUTABLE (SimpleInstallS2 inst.cxx foo.c foo.h)
  TARGET_LINK_LIBRARIES(SimpleInstallS2 ${TEST1_LIBRARY} ${TEST2_LIBRARY})
  SET(install_target SimpleInstallS2)

  INSTALL_TARGETS(/bin SimpleInstallS2)
ELSE(STAGE2)
  # this is stage 1, so create libraries and modules and install everything
  ADD_LIBRARY(test1 STATIC lib1.cxx)
  ADD_LIBRARY(test2 SHARED lib2.cxx)
  ADD_LIBRARY(test3 MODULE lib3.cxx)

  ADD_EXECUTABLE (SimpleInstall inst.cxx foo.c foo.h)
  TARGET_LINK_LIBRARIES(SimpleInstall test1 test2)
  SET(install_target SimpleInstall)

  ADD_DEPENDENCIES(SimpleInstall test3)

  INSTALL_TARGETS(/bin SimpleInstall)
  INSTALL_TARGETS(/lib test1 test2 test3)
  INSTALL_TARGETS(/include lib1.h lib2.h lib3.h)
  SET_TARGET_PROPERTIES(SimpleInstall PROPERTIES PRE_INSTALL_SCRIPT 
    ${CMAKE_CURRENT_SOURCE_DIR}/PreInstall.cmake)
  SET_TARGET_PROPERTIES(SimpleInstall PROPERTIES POST_INSTALL_SCRIPT 
    ${CMAKE_CURRENT_SOURCE_DIR}/PostInstall.cmake)
ENDIF(STAGE2)

ADD_CUSTOM_COMMAND(
  TARGET ${install_target}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND}
  ARGS ${EXTRA_INSTALL_FLAGS} 
  -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake"
  COMMENT "Install Project"
  )


