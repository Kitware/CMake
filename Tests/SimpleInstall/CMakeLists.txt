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

  # Check for failure of directory installation.
  IF(WIN32 AND NOT CYGWIN)
    SET(BAT .bat)
  ELSE(WIN32 AND NOT CYGWIN)
    SET(BAT)
  ENDIF(WIN32 AND NOT CYGWIN)
  IF(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/share/TestSubDir/TSD.h")
    MESSAGE(FATAL_ERROR "Directory installation did not install TSD.h")
  ENDIF(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/share/TestSubDir/TSD.h")
  IF(EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/share/TestSubDir/CVS")
    MESSAGE(FATAL_ERROR "Directory installation installed CVS directory.")
  ENDIF(EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/share/TestSubDir/CVS")
  IF(EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/share/CVS")
    MESSAGE(FATAL_ERROR "Directory installation installed CVS directory.")
  ENDIF(EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/share/CVS")
  EXECUTE_PROCESS(
    COMMAND "${CMAKE_INSTALL_PREFIX}/MyTest/share/sample_script${BAT}"
    RESULT_VARIABLE SAMPLE_SCRIPT_RESULT
    OUTPUT_VARIABLE SAMPLE_SCRIPT_OUTPUT
    )
  IF(NOT "${SAMPLE_SCRIPT_RESULT}" MATCHES "^0$")
    MESSAGE(FATAL_ERROR "Sample script failed: [${SAMPLE_SCRIPT_RESULT}]")
  ENDIF(NOT "${SAMPLE_SCRIPT_RESULT}" MATCHES "^0$")
  IF(NOT "${SAMPLE_SCRIPT_OUTPUT}" MATCHES "Sample Script Output")
    MESSAGE(FATAL_ERROR "Bad sample script output: [${SAMPLE_SCRIPT_OUTPUT}]")
  ENDIF(NOT "${SAMPLE_SCRIPT_OUTPUT}" MATCHES "Sample Script Output")

  # Check for failure of empty directory installation.
  IF(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/share/empty")
    MESSAGE(FATAL_ERROR "Empty directory installation did not install.")
  ENDIF(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/share/empty")
  FILE(GLOB EMPTY_FILES "${CMAKE_INSTALL_PREFIX}/MyTest/share/empty/*")
  IF(EMPTY_FILES)
    MESSAGE(FATAL_ERROR "Empty directory installed [${EMPTY_FILES}].")
  ENDIF(EMPTY_FILES)

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

  # Test optional installation.
  INSTALL(FILES does_not_exist.h DESTINATION MyTest/include/foo OPTIONAL)

  # Test configuration-specific installation.
  INSTALL(FILES lib1.h RENAME lib1release.h CONFIGURATIONS Release
    DESTINATION MyTest/include/Release
    )
  INSTALL(FILES lib1.h RENAME lib1debug.h CONFIGURATIONS Debug
    DESTINATION MyTest/include/Debug
    )

  # Test directory installation.
  FILE(REMOVE_RECURSE "${CMAKE_INSTALL_PREFIX}/MyTest/share/CVS")
  FILE(REMOVE_RECURSE "${CMAKE_INSTALL_PREFIX}/MyTest/share/TestSubDir/CVS")
  INSTALL(
    DIRECTORY TestSubDir scripts/ DESTINATION MyTest/share
    PATTERN "CVS" EXCLUDE
    PATTERN "scripts/*" PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
    )

  # Test empty directory installation.
  INSTALL(DIRECTORY DESTINATION MyTest/share/empty)

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
  SET_TARGET_PROPERTIES(test4 PROPERTIES VERSION 1.2 SOVERSION 3
        INSTALL_NAME_DIR @executable_path/../lib)
ENDIF(STAGE2)

IF(CMAKE_CONFIGURATION_TYPES)
  SET(SI_CONFIG -C ${CMAKE_CFG_INTDIR})
ELSE(CMAKE_CONFIGURATION_TYPES)
  SET(SI_CONFIG)
ENDIF(CMAKE_CONFIGURATION_TYPES)

# Dummy test of CPack
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Test of packaging with cpack")
SET(CPACK_PACKAGE_VENDOR "Kitware")
SET(CPACK_INSTALL_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/PackageScript.cmake")

IF(WIN32 AND NOT UNIX)
  FIND_PROGRAM(NSIS_MAKENSIS NAMES makensis
    PATHS [HKEY_LOCAL_MACHINE\\SOFTWARE\\NSIS]
    DOC "Where is makensis.exe located"
    )
  IF(NOT NSIS_MAKENSIS)
    SET(CPACK_GENERATOR TGZ)
  ENDIF(NOT NSIS_MAKENSIS)
ENDIF(WIN32 AND NOT UNIX)
IF(UNIX AND NOT APPLE)
  SET(CPACK_GENERATOR "TGZ;STGZ;TZ")
  #  FIND_PROGRAM(found_compress
  #    NAMES compress)
  #  IF(found_compress)
  #    FIND_PROGRAM(file_command NAMES file)
  #    IF(NOT file_command)
  #      set(file_command file)
  #    ENDIF(NOT file_command)
  #    EXECUTE_PROCESS(COMMAND ${file_command} ${found_compress}
  #      OUTPUT_VARIABLE output)
  #    set(SKIP_TZ FALSE)
  #    if("${output}" MATCHES "script")
  #      set(SKIP_TZ TRUE)
  #    endif("${output}" MATCHES "script")
  #    if("${output}" MATCHES "dummy.sh")
  #      set(SKIP_TZ TRUE)
  #    endif("${output}" MATCHES "dummy.sh")
  #    if(NOT SKIP_TZ)
  #      message("compress found and it was not a script")
  #      message("output from file command: [${output}]")
  #      SET(CPACK_GENERATOR "${CPACK_GENERATOR};TZ")
  #    else(NOT SKIP_TZ)
  #      message("compress found, but it was a script so dont use it")
  #      message("output from file command: [${output}]")
  #    endif(NOT SKIP_TZ)
  #  ENDIF(found_compress)
  FIND_PROGRAM(found_bz2
    NAMES bzip2)
  IF(found_bz2)
    SET(CPACK_GENERATOR "${CPACK_GENERATOR};TBZ2")
  ENDIF(found_bz2)
ENDIF(UNIX AND NOT APPLE)

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
