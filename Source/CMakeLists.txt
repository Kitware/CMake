CMAKE_MINIMUM_REQUIRED(VERSION 1.5)

SET(SRCS
cmake.cxx
cmakewizard.cxx
cmMakeDepend.cxx
cmMakefile.cxx
cmSourceFile.cxx
cmSystemTools.cxx
cmDocumentation.cxx
cmDynamicLoader.cxx
cmCommands.cxx
cmTarget.cxx
cmCustomCommand.cxx
cmCacheManager.cxx
cmSourceGroup.cxx
cmListFileCache.cxx
cmListFileLexer.c
cmGlob.cxx
cmGlobalGenerator.cxx
cmGlobalUnixMakefileGenerator.cxx
cmLocalGenerator.cxx
cmLocalUnixMakefileGenerator.cxx
cmVariableWatch.cxx
cmake.h
cmakewizard.h
cmMakeDepend.h
cmMakefile.h
cmSourceFile.h
cmSystemTools.h
cmDynamicLoader.h
cmCommands.h
cmTarget.h
cmCustomCommand.h
cmCacheManager.h
cmSourceGroup.h
cmListFileCache.h
cmGlobalGenerator.h
cmGlobalUnixMakefileGenerator.h
cmLocalGenerator.h
cmLocalUnixMakefileGenerator.h
cmVariableWatch.h
)

# configure the .h file
CONFIGURE_FILE(
${CMake_SOURCE_DIR}/Source/cmConfigure.cmake.h.in 
${CMake_BINARY_DIR}/Source/cmConfigure.h )

# add the include path to find the .h 
INCLUDE_DIRECTORIES(${CMake_BINARY_DIR}/Source)
INCLUDE_DIRECTORIES(${CMake_SOURCE_DIR}/Source)

# let cmake know it is supposed to use it
ADD_DEFINITIONS(-DCMAKE_BUILD_WITH_CMAKE)

IF (APPLE)
  SET(SRCS ${SRCS}
    cmGlobalCodeWarriorGenerator.cxx
    cmLocalCodeWarriorGenerator.cxx
    cmGlobalCodeWarriorGenerator.h
    cmLocalCodeWarriorGenerator.h
    )
ENDIF (APPLE)

IF (WIN32)
  IF(NOT UNIX)
    SET(SRCS ${SRCS}
      cmGlobalBorlandMakefileGenerator.cxx
      cmGlobalNMakeMakefileGenerator.cxx
      cmGlobalVisualStudio6Generator.cxx
      cmLocalVisualStudio6Generator.cxx
      cmGlobalBorlandMakefileGenerator.h
      cmGlobalNMakeMakefileGenerator.h
      cmGlobalVisualStudio6Generator.h
      cmLocalVisualStudio6Generator.h
      cmWin32ProcessExecution.cxx
      cmWin32ProcessExecution.h
      )
    IF(NOT MINGW)
      SET(SRCS ${SRCS}
        cmGlobalVisualStudio71Generator.h
        cmGlobalVisualStudio8Generator.h
        cmGlobalVisualStudio7Generator.h
        cmLocalVisualStudio7Generator.h
        cmGlobalVisualStudio8Generator.cxx
        cmGlobalVisualStudio71Generator.cxx
        cmGlobalVisualStudio7Generator.cxx
        cmLocalVisualStudio7Generator.cxx)
    ENDIF(NOT MINGW)
  ENDIF(NOT UNIX)
ENDIF (WIN32)

# create a library used by the command line and the GUI
ADD_LIBRARY(CMakeLib ${SRCS})
TARGET_LINK_LIBRARIES(CMakeLib cmsys)

# always link in the library
# the library is found here
LINK_DIRECTORIES(${CMake_BINARY_DIR}/Source)

IF(CMAKE_BUILD_MFC_DIALOG)
  LINK_LIBRARIES( rpcrt4.lib )
  ADD_EXECUTABLE(cmw9xcom cmw9xcom.cxx)
  TARGET_LINK_LIBRARIES(cmw9xcom CMakeLib)
  SUBDIRS(MFCDialog)
ENDIF(CMAKE_BUILD_MFC_DIALOG)

ADD_EXECUTABLE(cmake cmakemain.cxx)
ADD_EXECUTABLE(DumpDocumentation cmDumpDocumentation)

SET(CMTEST_SRCS ctest.cxx cmCTest.cxx ctest/cmCTestScriptHandler.cxx)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/Source/CTest)

IF (UNIX)
  TARGET_LINK_LIBRARIES(CMakeLib ${CMAKE_DL_LIBS})
ENDIF (UNIX)

TARGET_LINK_LIBRARIES(cmake CMakeLib)
TARGET_LINK_LIBRARIES(DumpDocumentation CMakeLib)

IF(CMAKE_BUILD_WITH_CURL)
  # Don't even look for this as we don't use that part of curl
  # and linking in the the library can cause problems.
  SET(HAVE_LIBCRYPTO 0)
  SUBDIRS(CTest)
  SET(CMTEST_SRCS ${CMTEST_SRCS} CTest/cmCTestSubmit.cxx)
  ADD_DEFINITIONS(-DHAVE_CURL)
  INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/Source/CTest/Curl
    )
ENDIF(CMAKE_BUILD_WITH_CURL)

ADD_EXECUTABLE(ctest ${CMTEST_SRCS})
TARGET_LINK_LIBRARIES(ctest CMakeLib)
IF(CMAKE_BUILD_WITH_CURL)
  TARGET_LINK_LIBRARIES(ctest Curl)
ENDIF(CMAKE_BUILD_WITH_CURL)

IF (UNIX)
  INCLUDE (${CMake_SOURCE_DIR}/Modules/FindCurses.cmake OPTIONAL)
  IF (CURSES_LIBRARY)
    INCLUDE(${CMake_SOURCE_DIR}/Source/CursesDialog/CMakeLists.txt)
  ENDIF (CURSES_LIBRARY)
ENDIF (UNIX)

OPTION(CMAKE_BUILD_WX_DIALOG "Build wxWindows dialog for CMake" FALSE)
MARK_AS_ADVANCED(CMAKE_BUILD_WX_DIALOG)
IF(CMAKE_BUILD_WX_DIALOG)
  SUBDIRS(WXDialog)
ENDIF(CMAKE_BUILD_WX_DIALOG)

IF (NOT DART_ROOT)
SET(MAKEPROGRAM ${CMAKE_MAKE_PROGRAM})
ENDIF (NOT DART_ROOT)

IF(BUILD_TESTING)
  # This variable is set by cmake, however to
  # test cmake we want to make sure that 
  # the ctest from this cmake is used for testing
  # and not the ctest from the cmake building and testing
  # cmake.
  SET(CMAKE_CTEST_COMMAND "${EXECUTABLE_OUTPUT_PATH}/ctest")

  ADD_TEST(CommandLineTest ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/CommandLineTest"
    "${CMake_BINARY_DIR}/Tests/CommandLineTest"
    --build-two-config 
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-project CommandLineTest
    --test-command CommandLineTest)
  
  ADD_TEST(CustomCommand  ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/CustomCommand"
    "${CMake_BINARY_DIR}/Tests/CustomCommand"
    --build-two-config
    --build-generator ${CMAKE_GENERATOR}
    --build-project CustomCommand
    --build-makeprogram ${MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/CustomCommand/bin"
    --test-command CustomCommand
    )

  ADD_TEST(FindPackageTest ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/FindPackageTest"
    "${CMake_BINARY_DIR}/Tests/FindPackageTest"
    --build-two-config
    --build-generator ${CMAKE_GENERATOR}
    --build-project FindPackageTest
    --build-makeprogram ${MAKEPROGRAM}
    --test-command FindPackageTest)

  ADD_TEST(SystemInformation ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/SystemInformation"
    "${CMake_BINARY_DIR}/Tests/SystemInformation"
    --build-two-config
    --build-generator ${CMAKE_GENERATOR}
    --build-project DumpInformation
    --build-makeprogram ${MAKEPROGRAM}
    --test-command DumpInformation)

  ADD_TEST(StringFileTest ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/StringFileTest"
    "${CMake_BINARY_DIR}/Tests/StringFileTest"
    --build-two-config
    --build-generator ${CMAKE_GENERATOR}
    --build-project StringFileTest
    --build-makeprogram ${MAKEPROGRAM}
    --test-command StringFileTest)
  
  ADD_TEST(TryCompile ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TryCompile"
    "${CMake_BINARY_DIR}/Tests/TryCompile"
    --build-generator ${CMAKE_GENERATOR}
    --build-project TryCompile
    --build-makeprogram ${MAKEPROGRAM}
    --build-two-config
    --test-command TryCompile)

  ADD_TEST(simple ${CMAKE_CTEST_COMMAND}
    --build-and-test 
      "${CMake_SOURCE_DIR}/Tests/Simple"
      "${CMake_BINARY_DIR}/Tests/Simple"
    --build-generator ${CMAKE_GENERATOR}
    --build-project Simple
    --build-makeprogram ${MAKEPROGRAM}
    --build-two-config
    --test-command simple)

  ADD_TEST(PreOrder ${CMAKE_CTEST_COMMAND}
    --build-and-test 
      "${CMake_SOURCE_DIR}/Tests/PreOrder"
      "${CMake_BINARY_DIR}/Tests/PreOrder"
    --build-generator ${CMAKE_GENERATOR}
    --build-project PreOrder
    --build-makeprogram ${MAKEPROGRAM}
    --build-two-config
    --test-command simple)

  ADD_TEST(conly  ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/COnly"
    "${CMake_BINARY_DIR}/Tests/COnly"
    --build-generator ${CMAKE_GENERATOR}
    --build-project conly
    --build-makeprogram ${MAKEPROGRAM}
    --build-two-config
    --test-command   conly)

  SET(SimpleInstallInstallDir 
    "${CMake_BINARY_DIR}/Tests/SimpleInstall/InstallDirectory")
  ADD_TEST(SimpleInstall ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/SimpleInstall"
    "${CMake_BINARY_DIR}/Tests/SimpleInstall"
    --build-generator ${CMAKE_GENERATOR}
    --build-project TestSimpleInstall
    --build-makeprogram ${MAKEPROGRAM}
    --build-two-config
    --build-options
     "-DCMAKE_INSTALL_PREFIX:PATH=${SimpleInstallInstallDir}"
    --test-command   ${SimpleInstallInstallDir}/bin/SimpleInstall)
  ADD_TEST(SimpleInstall-Stage2 ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/SimpleInstallS2"
    "${CMake_BINARY_DIR}/Tests/SimpleInstallS2"
    --build-generator ${CMAKE_GENERATOR}
    --build-project TestSimpleInstall
    --build-makeprogram ${MAKEPROGRAM}
    --build-two-config
    --build-options
     "-DCMAKE_INSTALL_PREFIX:PATH=${SimpleInstallInstallDir}"
     "-DSTAGE2:BOOL=1"
    --test-command   ${SimpleInstallInstallDir}/bin/SimpleInstallS2)

  ADD_TEST(X11  ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/X11"
    "${CMake_BINARY_DIR}/Tests/X11"
    --build-generator ${CMAKE_GENERATOR}
    --build-project UseX11
    --build-makeprogram ${MAKEPROGRAM}
    --build-two-config
    --test-command  UseX11)

  ADD_TEST(LoadedCommand  ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/LoadCommand"
    "${CMake_BINARY_DIR}/Tests/LoadCommand"
    --build-generator ${CMAKE_GENERATOR}
    --build-project LoadedCommand
    --build-makeprogram ${MAKEPROGRAM}
    --build-two-config
    --test-command  LoadedCommand)

  ADD_TEST(LoadedCommandOneConfig  ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/LoadCommandOneConfig"
    "${CMake_BINARY_DIR}/Tests/LoadCommandOneConfig"
    --build-generator ${CMAKE_GENERATOR}
    --build-project LoadedCommand
    --build-makeprogram ${MAKEPROGRAM}
    --test-command  LoadedCommand 
    )

  # Como does not seem to support shared libraries.
  GET_FILENAME_COMPONENT(CMAKE_BASE_NAME ${CMAKE_CXX_COMPILER} NAME_WE)
  IF(CMAKE_BASE_NAME MATCHES "^como$")
    SET(COMPILER_IS_COMO 1)
  ENDIF(CMAKE_BASE_NAME MATCHES "^como$")
  IF(NOT COMPILER_IS_COMO)
    ADD_TEST(complex   ${CMAKE_CTEST_COMMAND}
      --build-and-test 
      "${CMake_SOURCE_DIR}/Tests/Complex"
      "${CMake_BINARY_DIR}/Tests/Complex"
      --build-two-config
      --build-generator ${CMAKE_GENERATOR}
      --build-project complex
      --build-makeprogram ${MAKEPROGRAM}
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/Complex/bin"
      --test-command complex 
      )

    ADD_TEST(complexOneConfig  ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/ComplexOneConfig"
      "${CMake_BINARY_DIR}/Tests/ComplexOneConfig"
      --build-generator ${CMAKE_GENERATOR}
      --build-project complex
      --build-makeprogram ${MAKEPROGRAM}
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/ComplexOneConfig/bin"
      --test-command complex)

    ADD_TEST(complexRelativePaths  ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/ComplexRelativePaths"
      "${CMake_BINARY_DIR}/Tests/ComplexRelativePaths"
      --build-generator ${CMAKE_GENERATOR}
      --build-project complex
      --build-makeprogram ${MAKEPROGRAM}
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/ComplexRelativePaths/bin" 
      --build-options -DCMAKE_USE_RELATIVE_PATHS:BOOL=ON
      --test-command complex)

  ENDIF(NOT COMPILER_IS_COMO)

  ADD_TEST(Example ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Example"
    "${CMake_BINARY_DIR}/Example"
    --build-generator ${CMAKE_GENERATOR}
    --build-project HELLO
    --build-makeprogram ${MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Example/Demo"
    --test-command helloDemo
    )

  ADD_TEST(testing ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Testing"
    "${CMake_BINARY_DIR}/Tests/Testing"
    --build-generator ${CMAKE_GENERATOR}
    --build-project testing
    --build-makeprogram ${MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Testing/bin"
    --test-command testing
    )
  
  ADD_TEST(wrapping  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Wrapping"
    "${CMake_BINARY_DIR}/Tests/Wrapping"
    --build-generator ${CMAKE_GENERATOR}
    --build-project wrapping
    --build-makeprogram ${MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --test-command wrapping
    )

  INCLUDE ( ${CMAKE_ROOT}/Modules/FindQt.cmake )
  MARK_AS_ADVANCED(
    QT_INCLUDE_PATH
    QT_MOC_EXE
    QT_QT_LIBRARY
    QT_UIC_EXE)

  IF (QT_FOUND)
    ADD_TEST(qtwrapping  ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/Wrapping"
      "${CMake_BINARY_DIR}/Tests/Wrapping"
      --build-generator ${CMAKE_GENERATOR}
      --build-project Wrapping
      --build-makeprogram ${MAKEPROGRAM}
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
      --test-command qtwrapping
      )
  ENDIF (QT_FOUND)

  ADD_TEST(testdriver1 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TestDriver"
    "${CMake_BINARY_DIR}/Tests/TestDriver"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --build-project TestDriverTest
    --test-command TestDriverTest test1
    )

  ADD_TEST(testdriver2 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TestDriver"
    "${CMake_BINARY_DIR}/Tests/TestDriver"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --build-project TestDriverTest
    --test-command TestDriverTest test2
    )

  ADD_TEST(testdriver3  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TestDriver"
    "${CMake_BINARY_DIR}/Tests/TestDriver"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --build-project TestDriverTest
    --test-command TestDriverTest subdir/test3
    )

  ADD_TEST(dependency_w_libout  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Dependency"
    "${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut"
     --build-exe-dir "${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Exec"
     --build-project Dependency 
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
     --build-options
     -DLIBRARY_OUTPUT_PATH:PATH=${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Lib
     --test-command exec
     )


  ADD_TEST(dependency_wo_lib_out   ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Dependency"
    "${CMake_BINARY_DIR}/Tests/Dependency/WOLibOut"
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Dependency/WOLibOut/Exec"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-project Dependency 
    --test-command exec
    )

  ADD_TEST(dependency2  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Dependency"
    "${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut" 
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Exec2"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-project Dependency 
    --build-options
    -DLIBRARY_OUTPUT_PATH:PATH=${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Lib
    --test-command exec2
    )

  ADD_TEST(dependency3  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Dependency"
    "${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut"
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Exec3"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-project Dependency 
    --build-options
    -DLIBRARY_OUTPUT_PATH:PATH=${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Lib
    --test-command exec3)

  ADD_TEST(dependency4   ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Dependency"
    "${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut"
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Exec4"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-project Dependency 
    --build-options
    -DLIBRARY_OUTPUT_PATH:PATH=${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Lib
    --test-command exec4
    )

  ADD_TEST(JumpWithLibOut  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Jump"
    "${CMake_BINARY_DIR}/Tests/Jump/WithLibOut"
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Jump/WithLibOut/Executable"
    --build-project Jump
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-options
    -DLIBRARY_OUTPUT_PATH:PATH=${CMake_BINARY_DIR}/Tests/Jump/WithLibOut/Lib
    --test-command jumpExecutable
     )

  ADD_TEST(JumpNoLibOut  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Jump"
    "${CMake_BINARY_DIR}/Tests/Jump/NoLibOut"
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Jump/NoLibOut/Executable"
    --build-run-dir "${CMake_BINARY_DIR}/Tests/Jump/NoLibOut/Executable"
    --build-project Jump 
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --test-command jumpExecutable
     )

  ADD_TEST(linkline   ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/LinkLine"
    "${CMake_BINARY_DIR}/Tests/LinkLine"
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/LinkLine"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-project Linkline
    --test-command Exec
    )

  ADD_TEST(linkorder1 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/LinkLineOrder"
    "${CMake_BINARY_DIR}/Tests/LinkLineOrder" 
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/LinkLineOrder"
    --build-project LinkLineOrder
    --test-command Exec1
    )

  ADD_TEST(linkorder2  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/LinkLineOrder"
    "${CMake_BINARY_DIR}/Tests/LinkLineOrder"
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/LinkLineOrder"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-project LinkLineOrder
    --test-command Exec2
    )

  ADD_TEST(curl ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Source/CTest/Curl"
    "${CMake_BINARY_DIR}/Tests/Curl"  
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-project LIBCURL
    --test-command LIBCURL)

  ADD_TEST(kwsys ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Source/kwsys"
    "${CMake_BINARY_DIR}/Tests/kwsys"
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/kwsys"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-project kwsys
    --test-command  test1
    )

  ADD_TEST(SubDir ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/SubDir"
    "${CMake_BINARY_DIR}/Tests/SubDir"
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/SubDir/Executable"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-project SUBDIR
    --test-command test 
    "${CMake_BINARY_DIR}/Tests/SubDir/ShouldBeHere"
    "${CMake_BINARY_DIR}/Tests/SubDir/testfromsubdir.o"
    )

  IF (APPLE)
    ADD_TEST(objc++ ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/Objc++"
      "${CMake_BINARY_DIR}/Tests/Objc++"
      --build-generator ${CMAKE_GENERATOR}
      --build-makeprogram ${MAKEPROGRAM}
      --build-project  objc++
      --test-command  objc++
      )
  ENDIF (APPLE)

  IF (CTEST_TEST_CTEST)
    ADD_TEST(CTestTest ${CMAKE_CTEST_COMMAND} --force-new-ctest-process
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/CTestTest"
      "${CMake_BINARY_DIR}/Tests/CTestTest"
      --build-generator ${CMAKE_GENERATOR}
      --build-makeprogram ${MAKEPROGRAM}
      --build-project CTestTest
      --test-command ${CMAKE_CTEST_COMMAND} --force-new-ctest-process
      -S "${CMake_BINARY_DIR}/Tests/CTestTest/test.cmake" -V
      )

  ENDIF (CTEST_TEST_CTEST)
  # see if we can find a fortran compiler on the machine
  # if so, add the fortran test and see if it works.
  SET(CMAKE_Fortran_COMPILER_LIST ifort ifc efc f95 pgf95 
        lf95 xlf95 fort gfortran f90  pgf90   xlf90   epcf90 f77  fort77 frt pgf77  xlf  fl32 af77 g77  )
  FIND_PROGRAM(CMAKE_Fortran_COMPILER_FULLPATH NAMES ${CMAKE_Fortran_COMPILER_LIST} )
  MARK_AS_ADVANCED(CMAKE_Fortran_COMPILER_FULLPATH)
  IF(CMAKE_Fortran_COMPILER_FULLPATH)
    ADD_TEST(Fortran ${CMAKE_CTEST_COMMAND}
      --build-and-test 
      "${CMake_SOURCE_DIR}/Tests/Fortran"
      "${CMake_BINARY_DIR}/Tests/Fortran"
      --build-generator ${CMAKE_GENERATOR}
      --build-project Simple
      --build-makeprogram ${MAKEPROGRAM}
      --build-two-config
      --test-command testf)
  ENDIF(CMAKE_Fortran_COMPILER_FULLPATH)
     
  IF (CMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE OR WXWINDOWS_INCLUDE_DIR)
    # Will be set if the wxwindows gui is on
    ADD_TEST(UseWX ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/UseWX"
      "${CMake_BINARY_DIR}/Tests/UseWX"
      --build-generator ${CMAKE_GENERATOR}
      --build-makeprogram ${MAKEPROGRAM}
      --build-exe-dir ${CMake_BINARY_DIR}/Tests/UseWX
      --build-project UsewxWindows
      --build-options -DCMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE:FILEPATH=${CMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE}
      --test-command UseWX
    )

      
  ENDIF (CMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE OR WXWINDOWS_INCLUDE_DIR)

  IF(UNIX)
    STRING(COMPARE EQUAL "${CMAKE_INSTALL_PREFIX}" "${CMake_BINARY_DIR}/Tests/TestShellInstall/Prefix"
           PREFIX_IS_FOR_TEST)
    IF(PREFIX_IS_FOR_TEST)
      CONFIGURE_FILE(
        ${CMake_SOURCE_DIR}/Tests/TestInstall.sh.in
        ${CMake_BINARY_DIR}/Tests/TestShellInstall/TestInstall.sh
        @ONLY IMMEDIATE
        )
      ADD_TEST(ShellInstall /bin/sh ${CMake_BINARY_DIR}/Tests/TestShellInstall/TestShellInstall.sh)
    ENDIF(PREFIX_IS_FOR_TEST)
  ENDIF(UNIX)

ENDIF(BUILD_TESTING)

INCLUDE (${CMAKE_BINARY_DIR}/Source/LocalUserOptions.cmake OPTIONAL)
INCLUDE (${CMAKE_SOURCE_DIR}/Source/LocalUserOptions.cmake OPTIONAL)

INSTALL_TARGETS(/bin cmake)
INSTALL_TARGETS(/bin ctest)
INSTALL_FILES(${CMAKE_DATA_DIR}/include cmCPluginAPI.h)
