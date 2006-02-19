PROJECT (TestSimpleInstall)
SET(CMAKE_VERBOSE_MAKEFILE 1)
#SET(EXECUTABLE_OUTPUT_PATH "${TestSimpleInstall_BINARY_DIR}/This is exec path")
SET(EXECUTABLE_OUTPUT_PATH "${TestSimpleInstall_BINARY_DIR}/ExecPath")
SET(LIBRARY_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}")

# Skip generating the rpath pointing at the build tree to make sure
# the executable is installed with the proper rpath in the install
# tree.
SET(CMAKE_SKIP_BUILD_RPATH 1)

# Skip the dependency that causes a build when installing.  This
# avoids infinite loops when the post-build rule below installs.
SET(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY 1)

SET(CMAKE_DEBUG_POSTFIX "_test_debug_postfix")

SET(EXTRA_INSTALL_FLAGS)
MESSAGE("Extra install: ${EXTRA_INSTALL_FLAGS}")

IF(STAGE2)
  SET(LIBPATHS "${CMAKE_INSTALL_PREFIX}/lib")
  SET(t1NAMES test1 test1${CMAKE_DEBUG_POSTFIX})
  SET(t2NAMES test2 test2${CMAKE_DEBUG_POSTFIX})
  SET(t4NAMES test4 test4${CMAKE_DEBUG_POSTFIX})

  # Make sure the install script ran.
  SET(CMAKE_INSTALL_SCRIPT_DID_RUN 0)
  INCLUDE(${CMAKE_INSTALL_PREFIX}/InstallScriptOut.cmake OPTIONAL)
  IF(CMAKE_INSTALL_SCRIPT_DID_RUN)
    MESSAGE(STATUS "Stage 1 did run install script 2.")
  ELSE(CMAKE_INSTALL_SCRIPT_DID_RUN)
    MESSAGE(SEND_ERROR "Stage 1 did not run install script 2.")
  ENDIF(CMAKE_INSTALL_SCRIPT_DID_RUN)

  IF(CYGWIN OR MINGW)
    SET(LIBPATHS ${LIBPATHS} "${CMAKE_INSTALL_PREFIX}/bin")
  ENDIF(CYGWIN OR MINGW)
  MESSAGE("Search for library in: ${LIBPATHS}")

  SET(TEST1_LIBRARY "TEST1_LIBRARY-NOTFOUND" CACHE FILEPATH "Force find." FORCE)
  SET(TEST2_LIBRARY "TEST2_LIBRARY-NOTFOUND" CACHE FILEPATH "Force find." FORCE)
  SET(TEST4_LIBRARY "TEST4_LIBRARY-NOTFOUND" CACHE FILEPATH "Force find." FORCE)

  FIND_LIBRARY(TEST1_LIBRARY 
    NAMES ${t1NAMES}
    PATHS ${LIBPATHS}
    DOC "First library")
  FIND_LIBRARY(TEST2_LIBRARY 
    NAMES ${t2NAMES}
    PATHS ${LIBPATHS}
    DOC "Second library")
  FIND_LIBRARY(TEST4_LIBRARY 
    NAMES ${t4NAMES}
    PATHS ${LIBPATHS}
    DOC "Fourth library")

  ADD_EXECUTABLE (SimpleInstallS2 inst.cxx foo.c foo.h)
  TARGET_LINK_LIBRARIES(SimpleInstallS2 ${TEST1_LIBRARY} ${TEST2_LIBRARY} ${TEST4_LIBRARY})
  SET(install_target SimpleInstallS2)

  # Make sure the test executable can run from the install tree.
  SET_TARGET_PROPERTIES(SimpleInstallS2 PROPERTIES
    INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)

  INSTALL_TARGETS(/bin SimpleInstallS2)
ELSE(STAGE2)
  # this is stage 1, so create libraries and modules and install everything
  ADD_LIBRARY(test1 STATIC lib1.cxx)
  ADD_LIBRARY(test2 SHARED lib2.cxx)
  ADD_LIBRARY(test3 MODULE lib3.cxx)
  ADD_LIBRARY(test4 SHARED lib4.cxx)

  ADD_EXECUTABLE (SimpleInstall inst.cxx foo.c foo.h)
  TARGET_LINK_LIBRARIES(SimpleInstall test1 test2 test4)
  SET(install_target SimpleInstall)

  # Make sure the test executable can run from the install tree.
  SET_TARGET_PROPERTIES(SimpleInstall PROPERTIES
    INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)

  IF(CMAKE_GENERATOR MATCHES "Makefiles")
    ADD_SUBDIRECTORY(TestSubDir)
    ADD_DEPENDENCIES(SimpleInstall TSD)
  ENDIF(CMAKE_GENERATOR MATCHES "Makefiles")

  ADD_DEPENDENCIES(SimpleInstall test3)
  ADD_DEPENDENCIES(test2 test3)
  ADD_DEPENDENCIES(test4 test2)

  INSTALL(TARGETS SimpleInstall test1 test2 test3 test4
          RUNTIME DESTINATION bin LIBRARY DESTINATION lib)
  INSTALL_FILES(/include FILES lib1.h lib2.h lib3.h)

  # Test user-specified install scripts.
  INSTALL(
    SCRIPT InstallScript1.cmake
    SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/InstallScript2.cmake
    )
  SET_DIRECTORY_PROPERTIES(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_INSTALL_PREFIX}/InstallScriptOut.cmake)

  SET_TARGET_PROPERTIES(SimpleInstall PROPERTIES OUTPUT_NAME SimpleInstallExe)
  # Disable VERSION test until it is implemented in the XCode generator.
  #SET_TARGET_PROPERTIES(SimpleInstall PROPERTIES VERSION 1.2)
  SET_TARGET_PROPERTIES(SimpleInstall PROPERTIES PRE_INSTALL_SCRIPT 
    ${CMAKE_CURRENT_SOURCE_DIR}/PreInstall.cmake)
  SET_TARGET_PROPERTIES(SimpleInstall PROPERTIES POST_INSTALL_SCRIPT 
    ${CMAKE_CURRENT_SOURCE_DIR}/PostInstall.cmake)
  SET_TARGET_PROPERTIES(test4 PROPERTIES VERSION 1.2 SOVERSION 3)
ENDIF(STAGE2)

IF(CMAKE_CONFIGURATION_TYPES)
  SET(SI_CONFIG -C ${CMAKE_CFG_INTDIR})
ELSE(CMAKE_CONFIGURATION_TYPES)
  SET(SI_CONFIG)
ENDIF(CMAKE_CONFIGURATION_TYPES)

ADD_CUSTOM_COMMAND(
  TARGET ${install_target}
  POST_BUILD
  COMMAND ${CMAKE_CTEST_COMMAND}
  ARGS ${SI_CONFIG}
       --build-and-test
       ${CMAKE_SOURCE_DIR}
       ${CMAKE_BINARY_DIR}
       --build-generator ${CMAKE_GENERATOR}
       --build-project ${PROJECT_NAME}
       --build-makeprogram ${CMAKE_MAKE_PROGRAM}
       --build-noclean
       --build-target install
  COMMENT "Install Project"
  )
