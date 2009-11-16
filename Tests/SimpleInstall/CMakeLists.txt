cmake_minimum_required (VERSION 2.6)
PROJECT (TestSimpleInstall)
SET(CMAKE_VERBOSE_MAKEFILE 1)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY
  "${TestSimpleInstall_BINARY_DIR}/bin")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
  "${TestSimpleInstall_BINARY_DIR}/lib/static")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY
  "${TestSimpleInstall_BINARY_DIR}/lib")

# Skip generating the rpath pointing at the build tree to make sure
# the executable is installed with the proper rpath in the install
# tree.
SET(CMAKE_SKIP_BUILD_RPATH 1)

# Make sure the executable can run from the install tree.
SET(CMAKE_INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/MyTest/lib)

# Skip the dependency that causes a build when installing.  This
# avoids infinite loops when the post-build rule below installs.
SET(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY 1)
SET(CMAKE_SKIP_PACKAGE_ALL_DEPENDENCY 1)

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
  SET(t4NAMES test4out test4out${CMAKE_DEBUG_POSTFIX})

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

  # Test importing a library found on disk.
  ADD_LIBRARY(lib_test4 UNKNOWN IMPORTED)
  SET_PROPERTY(TARGET lib_test4 PROPERTY IMPORTED_LOCATION ${TEST4_LIBRARY})

  INCLUDE_DIRECTORIES(${CMAKE_INSTALL_PREFIX}/MyTest/include)
  ADD_EXECUTABLE (SimpleInstExeS2 inst2.cxx foo.c foo.h)
  TARGET_LINK_LIBRARIES(SimpleInstExeS2 ${TEST1_LIBRARY} ${TEST2_LIBRARY} lib_test4)
  SET(install_target SimpleInstExeS2)

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
  IF(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/share/TestSubDir/TSD.h")
    MESSAGE(FATAL_ERROR "Directory installation did not install TSD.h")
  ENDIF(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/share/TestSubDir/TSD.h")
  IF(EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/share/TestSubDir/CVS")
    MESSAGE(FATAL_ERROR "Directory installation installed CVS directory.")
  ENDIF(EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/share/TestSubDir/CVS")
  IF(EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/share/CVS")
    MESSAGE(FATAL_ERROR "Directory installation installed CVS directory.")
  ENDIF(EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/share/CVS")
  IF(EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/share/CMakeLists.txt")
    MESSAGE(FATAL_ERROR "Directory installation installed CMakeLists.txt.")
  ENDIF(EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/share/CMakeLists.txt")
  IF(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/share/alt/TestSubDir/TSD.h")
    MESSAGE(FATAL_ERROR "Directory installation did not install alternate TSD.h")
  ENDIF(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/share/alt/TestSubDir/TSD.h")
  IF(EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/share/alt/TestSubDir/TSD.cxx")
    MESSAGE(FATAL_ERROR "Directory installation installed alternate TSD.cxx")
  ENDIF(EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/share/alt/TestSubDir/TSD.cxx")

  # Check that scripts properly installed.
  IF(WIN32 AND NOT CYGWIN)
    SET(BAT .bat)
  ELSE(WIN32 AND NOT CYGWIN)
    SET(BAT)
  ENDIF(WIN32 AND NOT CYGWIN)
  FOREACH(loc share share/old1 share/old2 share/old3 share/alt)
    SET(CUR_SCRIPT "${CMAKE_INSTALL_PREFIX}/MyTest/${loc}/sample_script${BAT}")
    EXECUTE_PROCESS(
      COMMAND ${CUR_SCRIPT}
      RESULT_VARIABLE SAMPLE_SCRIPT_RESULT
      OUTPUT_VARIABLE SAMPLE_SCRIPT_OUTPUT
      )
    IF(NOT "${SAMPLE_SCRIPT_RESULT}" MATCHES "^0$")
      MESSAGE(FATAL_ERROR
        "Sample script [${CUR_SCRIPT}] failed: [${SAMPLE_SCRIPT_RESULT}]")
    ENDIF(NOT "${SAMPLE_SCRIPT_RESULT}" MATCHES "^0$")
    IF(NOT "${SAMPLE_SCRIPT_OUTPUT}" MATCHES "Sample Script Output")
      MESSAGE(FATAL_ERROR
        "Bad sample script [${CUR_SCRIPT}] output: [${SAMPLE_SCRIPT_OUTPUT}]")
    ENDIF(NOT "${SAMPLE_SCRIPT_OUTPUT}" MATCHES "Sample Script Output")
  ENDFOREACH(loc)

  # Check for failure of empty directory installation.
  IF(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/share/empty")
    MESSAGE(FATAL_ERROR "Empty directory installation did not install.")
  ENDIF(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/MyTest/share/empty")
  FILE(GLOB EMPTY_FILES "${CMAKE_INSTALL_PREFIX}/MyTest/share/empty/*")
  IF(EMPTY_FILES)
    MESSAGE(FATAL_ERROR "Empty directory installed [${EMPTY_FILES}].")
  ENDIF(EMPTY_FILES)

  # Make sure the test executable can run from the install tree.
  SET_TARGET_PROPERTIES(SimpleInstExeS2 PROPERTIES
    INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/MyTest/lib)

  INSTALL_TARGETS(/MyTest/bin SimpleInstExeS2)

# try to import the exported targets again
  SET(SimpleInstallS1_DIR ${CMAKE_INSTALL_PREFIX}/MyTest/lib)
  FIND_PACKAGE(SimpleInstallS1 REQUIRED)
  GET_TARGET_PROPERTY(simpleInstallImported S1_SimpleInstall IMPORTED)
  IF(NOT simpleInstallImported)
    MESSAGE(FATAL_ERROR "Target S1_SimpleInstall could not be imported")
  ENDIF(NOT simpleInstallImported)

ELSE(STAGE2)
  # Wipe out the install directory to do a fresh test.
  FILE(REMOVE_RECURSE ${CMAKE_INSTALL_PREFIX}/MyTest)

  # this is stage 1, so create libraries and modules and install everything
  ADD_LIBRARY(test1 STATIC lib1.cxx)
  ADD_LIBRARY(test2 SHARED lib2.cxx)
  ADD_LIBRARY(test3 MODULE lib3.cxx)
  ADD_LIBRARY(test4 SHARED lib4.cxx)

  # Test <ARCHIVE|LIBRARY|RUNTIME>_OUTPUT_NAME properties.
  SET_PROPERTY(TARGET test4 PROPERTY ARCHIVE_OUTPUT_NAME test4out)
  SET_PROPERTY(TARGET test4 PROPERTY LIBRARY_OUTPUT_NAME test4out)

  ADD_EXECUTABLE (SimpleInstall inst.cxx foo.c foo.h)
  TARGET_LINK_LIBRARIES(SimpleInstall test1 test2 test4)
  SET(install_target SimpleInstall)

  # Make sure the test executable can run from the install tree.
  SET_TARGET_PROPERTIES(SimpleInstall PROPERTIES
    INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/MyTest/lib)

  # Test per-configuration output name.
  SET_TARGET_PROPERTIES(test1 PROPERTIES RELEASE_OUTPUT_NAME test1rel)
  SET_TARGET_PROPERTIES(test2 PROPERTIES PUBLIC_HEADER foo.h)

  IF(CMAKE_GENERATOR MATCHES "Makefiles")
    ADD_SUBDIRECTORY(TestSubDir)
    ADD_DEPENDENCIES(SimpleInstall TSD)
  ENDIF(CMAKE_GENERATOR MATCHES "Makefiles")

  ADD_DEPENDENCIES(SimpleInstall test3)
  ADD_DEPENDENCIES(test2 test3)
  ADD_DEPENDENCIES(test4 test2)

  INSTALL(TARGETS SimpleInstall test1 test2 test3 EXPORT SimpleInstallS1
    RUNTIME DESTINATION MyTest/bin        COMPONENT Runtime     # .exe, .dll
    LIBRARY DESTINATION MyTest/lib        COMPONENT Runtime     # .so, mod.dll
    ARCHIVE DESTINATION MyTest/lib/static COMPONENT Development # .a, .lib
    PUBLIC_HEADER DESTINATION MyTest/include COMPONENT Development 
    )

  INSTALL(TARGETS test4 PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
    RUNTIME DESTINATION MyTest/bin
    LIBRARY DESTINATION MyTest/lib
    ARCHIVE DESTINATION MyTest/lib/static
    OPTIONAL # for coverage...target should always exist
    )
  INSTALL(FILES lib1.h DESTINATION MyTest/include/foo)
  INSTALL(FILES lib2.h
    DESTINATION MyTest/include/foo
    COMPONENT Development
    PERMISSIONS OWNER_READ OWNER_WRITE
    RENAME lib2renamed.h
    )

  # Test old-style install commands.
  INSTALL_FILES(/MyTest/include FILES lib3.h)
  INSTALL_FILES(/MyTest/include/old .h lib3)
  INSTALL_FILES(/MyTest/include/old "^lib2.h$")
  INSTALL_PROGRAMS(/MyTest/share/old1 FILES
    scripts/sample_script scripts/sample_script.bat)
  INSTALL_PROGRAMS(/MyTest/share/old2
    scripts/sample_script scripts/sample_script.bat)

# "export" the targets collected in "SimpleInstallS1"
  INSTALL(EXPORT SimpleInstallS1 FILE SimpleInstallS1Config.cmake 
                                 DESTINATION MyTest/lib 
                                 NAMESPACE S1_ )

  EXPORT(TARGETS SimpleInstall test1 test2 test3 
         FILE "${CMAKE_CURRENT_BINARY_DIR}/SimpleInstallS1Config.cmake"
         NAMESPACE S2_ )

  ADD_SUBDIRECTORY(scripts)

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
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE
    DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                          GROUP_READ GROUP_EXECUTE
    PATTERN "CVS" EXCLUDE
    REGEX "\\.txt$" EXCLUDE
    PATTERN "scripts/*" PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
    )

  # Alternate directory installation for coverage.
  INSTALL(
    DIRECTORY scripts/ DESTINATION MyTest/share/alt
    COMPONENT Development
    USE_SOURCE_PERMISSIONS
    PATTERN "CVS" EXCLUDE
    REGEX "\\.txt$" EXCLUDE
    )
  INSTALL(
    DIRECTORY TestSubDir DESTINATION MyTest/share/alt
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE
    DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                          GROUP_READ GROUP_EXECUTE
    FILES_MATCHING PATTERN "*.h"
    )

  # Test empty directory installation.
  INSTALL(DIRECTORY DESTINATION MyTest/share/empty)

  # Test optional directory installation.
  INSTALL(DIRECTORY does-not-exist DESTINATION MyTest/share OPTIONAL)

  # Test user-specified install scripts, with and without COMPONENT.
  INSTALL(
    SCRIPT InstallScript1.cmake
    CODE "SET(INSTALL_CODE_DID_RUN 1)"
    SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/InstallScript2.cmake
    )
  INSTALL(
    SCRIPT InstallScript3.cmake
    CODE "SET(INSTALL_CODE_WITH_COMPONENT_DID_RUN 1)"
    SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/InstallScript4.cmake
    COMPONENT Development
    )
  SET_DIRECTORY_PROPERTIES(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES
    "${CMAKE_INSTALL_PREFIX}/InstallScriptOut.cmake;${CMAKE_INSTALL_PREFIX}/InstallScript4Out.cmake")

  SET_TARGET_PROPERTIES(SimpleInstall PROPERTIES OUTPUT_NAME SimpleInstExe)
  # Disable VERSION test until it is implemented in the XCode generator.
  IF(NOT XCODE)
    SET_TARGET_PROPERTIES(SimpleInstall PROPERTIES VERSION 1.2)
  ENDIF(NOT XCODE)
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
SET(CMAKE_INSTALL_DEBUG_LIBRARIES 1)
INCLUDE(InstallRequiredSystemLibraries)

IF(CTEST_TEST_CPACK)
  SET(PACKAGE_TARGET --build-target package)

  # Avoid settings that require the .zip file command line tools...
  # (just build an NSIS installer without component support)
  #
  SET(CPACK_BINARY_ZIP OFF)
  SET(CPACK_MONOLITHIC_INSTALL ON)
ELSE(CTEST_TEST_CPACK)
  SET(PACKAGE_TARGET)
ENDIF(CTEST_TEST_CPACK)

INCLUDE(CPack)

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
  ${PACKAGE_TARGET}
  COMMENT "Install Project"
  )
