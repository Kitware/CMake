PROJECT (TestSimpleInstall)
SET(CMAKE_VERBOSE_MAKEFILE 1)
#SET(EXECUTABLE_OUTPUT_PATH "${TestSimpleInstall_BINARY_DIR}/This is exec path")
SET(EXECUTABLE_OUTPUT_PATH "${TestSimpleInstall_BINARY_DIR}/ExecPath")
SET(LIBRARY_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}")

# Skip generating the rpath pointing at the build tree to make sure
# the executable is installed with the proper rpath in the install
# tree.
SET(CMAKE_SKIP_BUILD_RPATH 1)

# Make sure the executable can run from the install tree.
SET(CMAKE_INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/MyTest/lib)

# Skip the dependency that causes a build when installing.  This
# avoids infinite loops when the post-build rule below installs.
SET(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY 1)

SET(CMAKE_DEBUG_POSTFIX "_test_debug_postfix")

SET(EXTRA_INSTALL_FLAGS)
MESSAGE("Extra install: ${EXTRA_INSTALL_FLAGS}")

IF(STAGE2)
  SET(LIBPATHS
    ${CMAKE_INSTALL_PREFIX}/MyTest/lib/static
    ${CMAKE_INSTALL_PREFIX}/MyTest/lib
    )
  SET(t1NAMES test1 test1${CMAKE_DEBUG_POSTFIX} test1rel)
  SET(t2NAMES test2 test2${CMAKE_DEBUG_POSTFIX})
  SET(t4NAMES test4 test4${CMAKE_DEBUG_POSTFIX})

  # Make sure the install script ran.
  SET(CMAKE_INSTALL_SCRIPT_DID_RUN 0)
  INCLUDE(${CMAKE_INSTALL_PREFIX}/MyTest/InstallScriptOut.cmake OPTIONAL)
  IF(CMAKE_INSTALL_SCRIPT_DID_RUN)
    MESSAGE(STATUS "Stage 1 did run install script 2.")
  ELSE(CMAKE_INSTALL_SCRIPT_DID_RUN)
    MESSAGE(SEND_ERROR "Stage 1 did not run install script 2.")
  ENDIF(CMAKE_INSTALL_SCRIPT_DID_RUN)

  IF(CYGWIN OR MINGW)
    SET(LIBPATHS ${LIBPATHS} "${CMAKE_INSTALL_PREFIX}/MyTest/bin")
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

  INCLUDE_DIRECTORIES(${CMAKE_INSTALL_PREFIX}/MyTest/include)
  ADD_EXECUTABLE (SimpleInstallS2 inst2.cxx foo.c foo.h)
  TARGET_LINK_LIBRARIES(SimpleInstallS2 ${TEST1_LIBRARY} ${TEST2_LIBRARY} ${TEST4_LIBRARY})
  SET(install_target SimpleInstallS2)

  IF("${TEST1_LIBRARY}" MATCHES "static")
    MESSAGE(STATUS "test1 correctly found in lib/static")
  ELSE("${TEST1_LIBRARY}" MATCHES "static")
    MESSAGE(SEND_ERROR "test1 not found in lib/static!")
  ENDIF("${TEST1_LIBRARY}" MATCHES "static")

  # Check for failure of configuration-specific installation.
  IF(EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/include/Release/lib1debug.h")
    MESSAGE(FATAL_ERROR "Debug-configuration file installed for Release!")
  ENDIF(EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/include/Release/lib1debug.h")
  IF(EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/include/Debug/lib1release.h")
    MESSAGE(FATAL_ERROR "Release-configuration file installed for Debug!")
  ENDIF(EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/include/Debug/lib1release.h")

  # Make sure the test executable can run from the install tree.
  SET_TARGET_PROPERTIES(SimpleInstallS2 PROPERTIES
    INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/MyTest/lib)

  INSTALL_TARGETS(/MyTest/bin SimpleInstallS2)
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
    INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/MyTest/lib)

  # Test per-configuration output name.
  SET_TARGET_PROPERTIES(test1 PROPERTIES RELEASE_OUTPUT_NAME test1rel)

  IF(CMAKE_GENERATOR MATCHES "Makefiles")
    ADD_SUBDIRECTORY(TestSubDir)
    ADD_DEPENDENCIES(SimpleInstall TSD)
  ENDIF(CMAKE_GENERATOR MATCHES "Makefiles")

  ADD_DEPENDENCIES(SimpleInstall test3)
  ADD_DEPENDENCIES(test2 test3)
  ADD_DEPENDENCIES(test4 test2)

  INSTALL(TARGETS SimpleInstall test1 test2 test3
    RUNTIME DESTINATION MyTest/bin        COMPONENT Runtime     # .exe, .dll
    LIBRARY DESTINATION MyTest/lib        COMPONENT Runtime     # .so, mod.dll
    ARCHIVE DESTINATION MyTest/lib/static COMPONENT Development # .a, .lib
    )
  INSTALL(TARGETS test4 PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
    RUNTIME DESTINATION MyTest/bin
    LIBRARY DESTINATION MyTest/lib
    ARCHIVE DESTINATION MyTest/lib/static
    )
  INSTALL(FILES lib1.h DESTINATION MyTest/include/foo)
  INSTALL(FILES lib2.h
    DESTINATION MyTest/include/foo
    PERMISSIONS OWNER_READ OWNER_WRITE
    RENAME lib2renamed.h
    )
  INSTALL_FILES(/MyTest/include FILES lib3.h)

  # Test configuration-specific installation.
  INSTALL(FILES lib1.h RENAME lib1release.h CONFIGURATIONS Release
    DESTINATION MyTest/include/Release
    )
  INSTALL(FILES lib1.h RENAME lib1debug.h CONFIGURATIONS Debug
    DESTINATION MyTest/include/Debug
    )

  # Test user-specified install scripts.
  INSTALL(
    SCRIPT InstallScript1.cmake
    CODE "SET(INSTALL_CODE_DID_RUN 1)"
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

# Dummy test of CPack
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Test of packaging with cpack")
SET(CPACK_PACKAGE_VENDOR "Kitware")

IF(WIN32 AND NOT UNIX)
  FIND_PROGRAM(NSIS_MAKENSIS NAMES makensis
    PATHS [HKEY_LOCAL_MACHINE\\SOFTWARE\\NSIS]
    DOC "Where is makensis.exe located"
    )
  IF(NOT NSIS_MAKENSIS)
    SET(CPACK_GENERATOR TGZ)
  ENDIF(NOT NSIS_MAKENSIS)
ENDIF(WIN32 AND NOT UNIX)

SET(CPACK_PACKAGE_EXECUTABLES "SimpleInstall" "Simple Install")
SET(CMAKE_INSTALL_MFC_LIBRARIES 1)
INCLUDE(InstallRequiredSystemLibraries)
INCLUDE(CPack)

IF(APPLE AND NOT CTEST_TEST_CPACK)
  # Issue with packaging on the mac, so disable it for now
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
ELSE(APPLE AND NOT CTEST_TEST_CPACK)
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
    --build-target package
    COMMENT "Install Project"
    )
ENDIF(APPLE AND NOT CTEST_TEST_CPACK)
