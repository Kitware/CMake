# a macro for tests that have a simple format where the name matches the
# directory and project
MACRO(ADD_TEST_MACRO NAME COMMAND)
  ADD_TEST(${NAME} ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/${NAME}"
    "${CMake_BINARY_DIR}/Tests/${NAME}"
    --build-two-config
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-project ${NAME}
    --test-command ${COMMAND})
ENDMACRO(ADD_TEST_MACRO)


# Testing
IF(BUILD_TESTING)
  # Should the long tests be run?
  OPTION(CMAKE_RUN_LONG_TESTS 
    "Should the long tests be run (such as Bootstrap)." ON)
  MARK_AS_ADVANCED(CMAKE_RUN_LONG_TESTS)

  IF (CMAKE_RUN_LONG_TESTS)
    OPTION(CTEST_TEST_CTEST 
      "Should the tests that run a full sub ctest process be run?" 
      OFF)
    MARK_AS_ADVANCED(CTEST_TEST_CTEST)
  ENDIF (CMAKE_RUN_LONG_TESTS)

  # Should CPack tests be run? By default, yes, but...
  #
  # Disable packaging test on Apple 10.3 and below. PackageMaker starts
  # DiskManagementTool as root and disowns it
  # (http://lists.apple.com/archives/installer-dev/2005/Jul/msg00005.html).
  # It is left holding open pipe handles and preventing ProcessUNIX from
  # detecting end-of-data even after its immediate child exits. Then
  # the test hangs until it times out and is killed. This is a
  # well-known bug in kwsys process execution that I would love to get
  # time to fix.
  #
  OPTION(CTEST_TEST_CPACK
    "Should the tests that use '--build-target package' be run?"
    ON)
  MARK_AS_ADVANCED(CTEST_TEST_CPACK)
  IF(APPLE AND CTEST_TEST_CPACK)
    EXECUTE_PROCESS(
      COMMAND sw_vers -productVersion
      OUTPUT_VARIABLE OSX_VERSION
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
    IF(OSX_VERSION MATCHES "^10\\.[0123]" OR OSX_VERSION MATCHES "ProductVersion:\t10\\.[0123]")
      MESSAGE(STATUS "Forcing CTEST_TEST_CPACK=OFF on OSX < 10.4")
      MESSAGE(STATUS "OSX_VERSION='${OSX_VERSION}'")
      SET(CTEST_TEST_CPACK OFF)
    ENDIF(OSX_VERSION MATCHES "^10\\.[0123]" OR OSX_VERSION MATCHES "ProductVersion:\t10\\.[0123]")
  ENDIF(APPLE AND CTEST_TEST_CPACK)

  # Use 1500 or CTEST_TEST_TIMEOUT for long test timeout value,
  # whichever is greater.
  SET(CMAKE_LONG_TEST_TIMEOUT 1500)
  IF(CTEST_TEST_TIMEOUT)
    SET(CMAKE_LONG_TEST_TIMEOUT ${CTEST_TEST_TIMEOUT})
  ENDIF(CTEST_TEST_TIMEOUT)
  IF(CMAKE_LONG_TEST_TIMEOUT LESS 1500)
    SET(CMAKE_LONG_TEST_TIMEOUT 1500)
  ENDIF(CMAKE_LONG_TEST_TIMEOUT LESS 1500)

  # add a bunch of standard build-and-test style tests
  ADD_TEST_MACRO(CommandLineTest CommandLineTest)
  ADD_TEST_MACRO(FindPackageTest FindPackageTest)
  ADD_TEST_MACRO(FindModulesExecuteAll FindModulesExecuteAll)
  ADD_TEST_MACRO(StringFileTest StringFileTest)
  ADD_TEST_MACRO(TryCompile TryCompile)
  ADD_TEST_MACRO(TarTest TarTest)
  ADD_TEST_MACRO(SystemInformation SystemInformation)
  ADD_TEST_MACRO(MathTest MathTest)
  ADD_TEST_MACRO(Simple Simple)
  ADD_TEST_MACRO(PreOrder PreOrder)
  ADD_TEST_MACRO(COnly COnly)
  ADD_TEST_MACRO(CxxOnly CxxOnly)
  ADD_TEST_MACRO(NewlineArgs NewlineArgs)
  ADD_TEST_MACRO(SetLang SetLang)
  ADD_TEST_MACRO(ExternalOBJ ExternalOBJ)
  ADD_TEST_MACRO(LoadCommand LoadedCommand)
  ADD_TEST_MACRO(LinkLine LinkLine)
  ADD_TEST_MACRO(MacroTest miniMacroTest)
  ADD_TEST_MACRO(FunctionTest miniFunctionTest)
  ADD_TEST_MACRO(ReturnTest ReturnTest)
  ADD_TEST_MACRO(Properties Properties)
  ADD_TEST_MACRO(Assembler HelloAsm)
  ADD_TEST_MACRO(SourceGroups SourceGroups)
  ADD_TEST_MACRO(Preprocess Preprocess)
  ADD_TEST_MACRO(ExportImport ExportImport)
  ADD_TEST_MACRO(PolicyScope PolicyScope)
  ADD_TEST_MACRO(Unset Unset)

  # test for correct sub-project generation
  # not implemented in VS6 or Xcode
  IF(NOT MSVC60 AND NOT XCODE AND NOT MSVC70)
    # run cmake and configure all of SubProject
    # but only build the independent executable car
    ADD_TEST(SubProject ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/SubProject"
      "${CMake_BINARY_DIR}/Tests/SubProject"
      --build-project SubProject
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-target car
      --test-command car
      )
    # For stage 2, do not run cmake again.
    # Then build the foo sub project which should build
    # the bar library which should be referenced because
    # foo links to the static library bar, but bar is not
    # directly in the foo sub project
    ADD_TEST(SubProject-Stage2  ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/SubProject/foo"
      "${CMake_BINARY_DIR}/Tests/SubProject/foo" 
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-nocmake 
      --build-project foo
      --build-target foo
      --test-command foo
      )
  ENDIF(NOT MSVC60 AND NOT XCODE AND NOT MSVC70)

  IF (CMAKE_STRICT)
    ADD_TEST_MACRO(DocTest DocTest)
  ENDIF (CMAKE_STRICT)
    
  IF(CMAKE_BUILD_NIGHTLY_WINDOWS_TEST)
    FILE(MAKE_DIRECTORY "${CMake_BINARY_DIR}/Tests/CMake-CVS")
    FILE(WRITE "${CMake_BINARY_DIR}/Tests/CMake-CVS/nightly-cmake.sh"
"cd ${CMake_BINARY_DIR}/Tests/CMake-CVS
${CMake_BINARY_DIR}/bin/cmake -DCMAKE_VERSION=CVS -P ${CMake_SOURCE_DIR}/Utilities/Release/vogon_release.cmake
${CMake_BINARY_DIR}/bin/cmake -DVERSION=CVS -P ${CMake_SOURCE_DIR}/Utilities/Release/upload_release.cmake
")
    ADD_TEST(NightlyCMakeBuild 
      /bin/sh ${CMake_BINARY_DIR}/Tests/CMake-CVS/nightly-cmake.sh)
    IF(COMMAND SET_TESTS_PROPERTIES AND COMMAND GET_TEST_PROPERTY)
      SET_TESTS_PROPERTIES ( NightlyCMakeBuild PROPERTIES TIMEOUT 3500)
    ENDIF(COMMAND SET_TESTS_PROPERTIES AND COMMAND GET_TEST_PROPERTY)
  ENDIF(CMAKE_BUILD_NIGHTLY_WINDOWS_TEST)


  # add tests with more complex invocations
  ADD_TEST(Framework ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Framework"
    "${CMake_BINARY_DIR}/Tests/Framework"
    --build-two-config
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-project Framework
    --build-options
    "-DCMAKE_INSTALL_PREFIX:PATH=${CMake_BINARY_DIR}/Tests/Framework/Install"
    --test-command bar)

  ADD_TEST(TargetName ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TargetName"
    "${CMake_BINARY_DIR}/Tests/TargetName"
    --build-two-config
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-project TargetName
    --test-command ${CMAKE_CMAKE_COMMAND} -E compare_files 
    ${CMake_SOURCE_DIR}/Tests/TargetName/scripts/hello_world
    ${CMake_BINARY_DIR}/Tests/TargetName/scripts/hello_world)

  ADD_TEST(LibName ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/LibName"
    "${CMake_BINARY_DIR}/Tests/LibName"
    --build-two-config
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-project LibName 
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/LibName/lib"
    --test-command foobar
    )

  ADD_TEST(CustComDepend ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/CustComDepend"
    "${CMake_BINARY_DIR}/Tests/CustComDepend"
    --build-two-config
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-project CustComDepend 
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/CustComDepend/bin"
    --test-command foo bar.c
    )

  ADD_TEST(CustomCommand  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/CustomCommand"
    "${CMake_BINARY_DIR}/Tests/CustomCommand"
    --build-two-config
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project CustomCommand
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/CustomCommand/bin"
    --test-command CustomCommand
    )

  ADD_TEST(CustomCommandWorkingDirectory  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/CustomCommandWorkingDirectory"
    "${CMake_BINARY_DIR}/Tests/CustomCommandWorkingDirectory"
    --build-two-config
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project TestWorkingDir
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --test-command working
    )

   #ADD_TEST(SimpleExclude ${CMAKE_CTEST_COMMAND}
   #  --build-and-test
   #  "${CMake_SOURCE_DIR}/Tests/SimpleExclude"
   #  "${CMake_BINARY_DIR}/Tests/SimpleExclude"
   #  --build-generator ${CMAKE_TEST_GENERATOR}
   #  --build-project SimpleExclude
   #  --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
   #  --build-two-config
   #  --test-command t4
   #--test-command "${CMAKE_COMMAND}"
   #"-DCONFIGURATION=\${CTEST_CONFIGURATION_TYPE}"
   #-P "${CMAKE_BINARY_DIR}/Tests/SimpleExclude/run.cmake"
   #)

#  ADD_TEST(SameName  ${CMAKE_CTEST_COMMAND}
#    --build-and-test
#    "${CMake_SOURCE_DIR}/Tests/SameName"
#    "${CMake_BINARY_DIR}/Tests/SameName"
#    --build-generator ${CMAKE_TEST_GENERATOR}
#    --build-project SameName
#    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
#    --build-two-config
#    --test-command
#    "${CMake_BINARY_DIR}/Tests/SameName/Exe1/mytest2")

  ADD_TEST(OutOfSource ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/OutOfSource"
    "${CMake_BINARY_DIR}/Tests/OutOfSource"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project OutOfSource
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-two-config
    --test-command
    "${CMake_BINARY_DIR}/Tests/OutOfSource/SubDir/OutOfSourceSubdir/simple")

  ADD_TEST(BuildDepends ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/BuildDepends"
    "${CMake_BINARY_DIR}/Tests/BuildDepends"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project BuildDepends
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    )
  
  SET(SimpleInstallInstallDir
    "${CMake_BINARY_DIR}/Tests/SimpleInstall/InstallDirectory")
  ADD_TEST(SimpleInstall ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/SimpleInstall"
    "${CMake_BINARY_DIR}/Tests/SimpleInstall"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project TestSimpleInstall
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-two-config
    --build-options
    "-DCMAKE_INSTALL_PREFIX:PATH=${SimpleInstallInstallDir}"
    --test-command   ${SimpleInstallInstallDir}/MyTest/bin/SimpleInstExe)
  ADD_TEST(SimpleInstall-Stage2 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/SimpleInstallS2"
    "${CMake_BINARY_DIR}/Tests/SimpleInstallS2"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project TestSimpleInstall
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-two-config
    --build-options
    "-DCMAKE_INSTALL_PREFIX:PATH=${SimpleInstallInstallDir}"
    "-DSTAGE2:BOOL=1"
    --test-command   ${SimpleInstallInstallDir}/MyTest/bin/SimpleInstExeS2)

  # By default, run the CPackComponents test if the CTEST_TEST_CPACK
  # option is ON:
  #
  set(CTEST_RUN_CPackComponents ${CTEST_TEST_CPACK})

  # But on Windows, only run the CPackComponents test if the NSIS
  # installer builder is available:
  #
  if(WIN32)
    find_program(NSIS_MAKENSIS_EXECUTABLE NAMES makensis
      PATHS [HKEY_LOCAL_MACHINE\\SOFTWARE\\NSIS]
      DOC "makensis.exe location"
      )
    if(NSIS_MAKENSIS_EXECUTABLE)
      set(CTEST_RUN_CPackComponents ON)
    else(NSIS_MAKENSIS_EXECUTABLE)
      set(CTEST_RUN_CPackComponents OFF)
    endif(NSIS_MAKENSIS_EXECUTABLE)
  endif(WIN32)

  IF(CTEST_RUN_CPackComponents)
    ADD_TEST(CPackComponents ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/CPackComponents"
      "${CMake_BINARY_DIR}/Tests/CPackComponents"
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-project CPackComponents
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-two-config
      --build-target package
      --test-command ${CMAKE_CMAKE_COMMAND}
        "-DCPackComponents_BINARY_DIR:PATH=${CMake_BINARY_DIR}/Tests/CPackComponents"
        -P "${CMake_SOURCE_DIR}/Tests/CPackComponents/VerifyResult.cmake")
  ENDIF(CTEST_RUN_CPackComponents)

  ADD_TEST(X11  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/X11"
    "${CMake_BINARY_DIR}/Tests/X11"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project UseX11
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-two-config
    --test-command  UseX11)

  ADD_TEST(LoadedCommandOneConfig  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/LoadCommandOneConfig"
    "${CMake_BINARY_DIR}/Tests/LoadCommandOneConfig"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project LoadCommand
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --test-command  LoadedCommand
    )

  # Como does not seem to support shared libraries.
  GET_FILENAME_COMPONENT(CMAKE_BASE_NAME ${CMAKE_CXX_COMPILER} NAME_WE)
  IF(CMAKE_BASE_NAME MATCHES "^como$")
    SET(COMPILER_IS_COMO 1)
  ENDIF(CMAKE_BASE_NAME MATCHES "^como$")
  IF(NOT COMPILER_IS_COMO)
    ADD_TEST(complex  ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/Complex"
      "${CMake_BINARY_DIR}/Tests/Complex"
      --build-two-config
      --build-config-sample "${CMAKE_CTEST_COMMAND}"
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-project Complex
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/Complex/bin"
      --build-options
      -DCMAKE_TEST_DIFFERENT_GENERATOR:BOOL=${CMAKE_TEST_DIFFERENT_GENERATOR}
      -DCMAKE_TEST_SYSTEM_LIBRARIES:BOOL=${CMAKE_TEST_SYSTEM_LIBRARIES}
      --test-command complex
      )

    ADD_TEST(complexOneConfig  ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/ComplexOneConfig"
      "${CMake_BINARY_DIR}/Tests/ComplexOneConfig"
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-project Complex
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/ComplexOneConfig/bin"
      --build-options
      -DCMAKE_TEST_DIFFERENT_GENERATOR:BOOL=${CMAKE_TEST_DIFFERENT_GENERATOR}
      -DCMAKE_TEST_SYSTEM_LIBRARIES:BOOL=${CMAKE_TEST_SYSTEM_LIBRARIES}
      --test-command complex)

# This fails on VS 70
# works on Xcode and makefiles
#      ADD_TEST(ConvLibrary ${CMAKE_CTEST_COMMAND}
#        --build-and-test
#        "${CMake_SOURCE_DIR}/Tests/ConvLibrary"
#        "${CMake_BINARY_DIR}/Tests/ConvLibrary"
#        --build-two-config
#        --build-generator ${CMAKE_TEST_GENERATOR}
#        --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
#        --build-project ConvLibrary
#        --test-command bartest)


#    ADD_TEST(complexRelativePaths  ${CMAKE_CTEST_COMMAND}
#      --build-and-test
#      "${CMake_SOURCE_DIR}/Tests/ComplexRelativePaths"
#      "${CMake_BINARY_DIR}/Tests/ComplexRelativePaths"
#      --build-generator ${CMAKE_TEST_GENERATOR}
#      --build-project complex
#      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
#      --build-exe-dir "${CMake_BINARY_DIR}/Tests/ComplexRelativePaths/bin"
#      --build-options -DCMAKE_USE_RELATIVE_PATHS:BOOL=ON
#      --test-command complex)

  ENDIF(NOT COMPILER_IS_COMO)

  ADD_TEST(Example ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Example"
    "${CMake_BINARY_DIR}/Example"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project HELLO
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Example/Demo"
    --test-command helloDemo
    )

  # do each of the tutorial steps
  FOREACH(STP RANGE 1 7)
    ADD_TEST(TutorialStep${STP} ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/Tutorial/Step${STP}"
      "${CMake_BINARY_DIR}/Tests/Tutorial/Step${STP}"
      --build-two-config
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-project Tutorial
      --test-command Tutorial 25.0)
  ENDFOREACH(STP)

  ADD_TEST(testing ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Testing"
    "${CMake_BINARY_DIR}/Tests/Testing"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project Testing
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Testing/bin"
    --test-command testing
    )

  ADD_TEST(wrapping  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Wrapping"
    "${CMake_BINARY_DIR}/Tests/Wrapping"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project Wrapping
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --test-command wrapping
    )
  ADD_TEST(qtwrapping  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Wrapping"
    "${CMake_BINARY_DIR}/Tests/Wrapping"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project Wrapping
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
      --test-command qtwrapping
      )

  ADD_TEST(testdriver1 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TestDriver"
    "${CMake_BINARY_DIR}/Tests/TestDriver"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --build-project TestDriverTest
    --test-command TestDriverTest test1
    )

  ADD_TEST(testdriver2 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TestDriver"
    "${CMake_BINARY_DIR}/Tests/TestDriver"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --build-project TestDriverTest
    --test-command TestDriverTest test2
    )

  ADD_TEST(testdriver3  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TestDriver"
    "${CMake_BINARY_DIR}/Tests/TestDriver"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --build-project TestDriverTest
    --test-command TestDriverTest subdir/test3
    )

  ADD_TEST(Dependency ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Dependency"
    "${CMake_BINARY_DIR}/Tests/Dependency"
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Dependency/Exec"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-project Dependency
    --test-command exec
    )

  IF("${CMAKE_SYSTEM_NAME}" MATCHES syllable)

# RPATH isn't supported under Syllable, so the tests don't
# find their libraries. In order to fix that LIBRARY_OUTPUT_DIR
# in the tests would have to be adjusted to ${EXECUTABLE_OUTPUT_DIR}/lib .
# For now we just require on Syllable that the user adjusts the DLL_PATH 
# environment variable, so except the two tests below all other tests will succeed.

    SET(_DLL_PATH "$ENV{DLL_PATH}")
    IF(NOT "${_DLL_PATH}" MATCHES "^(.*:)?\\@bindir\\@/\\.(:.*)?$")
      MESSAGE(FATAL_ERROR "In order to successfully run the CMake test suite on Syllable you need to add \"\\@bindir\\@/.\" to the DLL_PATH environment variable")
    ENDIF(NOT "${_DLL_PATH}" MATCHES "^(.*:)?\\@bindir\\@/\\.(:.*)?$")
    IF(NOT "${_DLL_PATH}" MATCHES "^(.*:)?\\@bindir\\@/\\.\\./lib(:.*)?$")
      MESSAGE(FATAL_ERROR "In order to successfully run the CMake test suite on Syllable you need to add \"\\@bindir\\@/../lib\" to the DLL_PATH environment variable")
    ENDIF(NOT "${_DLL_PATH}" MATCHES "^(.*:)?\\@bindir\\@/\\.\\./lib(:.*)?$")

  ELSE("${CMAKE_SYSTEM_NAME}" MATCHES syllable)

    ADD_TEST(JumpWithLibOut  ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/Jump"
      "${CMake_BINARY_DIR}/Tests/Jump/WithLibOut"
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/Jump/WithLibOut/Executable"
      --build-project Jump
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
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
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --test-command jumpExecutable
      )

    ADD_TEST(Plugin ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/Plugin"
      "${CMake_BINARY_DIR}/Tests/Plugin"
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-project Plugin
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-two-config
      --test-command bin/example)

    IF(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG)
      ADD_TEST_MACRO(RuntimePath RuntimePath)
    ENDIF(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG)
  ENDIF("${CMAKE_SYSTEM_NAME}" MATCHES syllable)

  ADD_TEST(linkorder1 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/LinkLineOrder"
    "${CMake_BINARY_DIR}/Tests/LinkLineOrder"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-project LinkLineOrder
    --test-command Exec1
    )

  ADD_TEST(linkorder2  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/LinkLineOrder"
    "${CMake_BINARY_DIR}/Tests/LinkLineOrder"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-project LinkLineOrder
    --test-command Exec2
    )

  IF(NOT CMAKE_TEST_DIFFERENT_GENERATOR)
    ADD_TEST(kwsys ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Source/kwsys"
      "${CMake_BINARY_DIR}/Tests/kwsys"
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-project kwsys
      --test-command kwsysTestsCxx testIOS
      )
  ENDIF(NOT CMAKE_TEST_DIFFERENT_GENERATOR)
  SET(MAKE_IS_GNU )
  IF(${CMAKE_TEST_MAKEPROGRAM} MATCHES make)
    EXEC_PROGRAM(
      ${CMAKE_TEST_MAKEPROGRAM} ARGS no_such_target --version
      RETURN_VALUE res OUTPUT_VARIABLE out
      )
    IF("${res}" EQUAL 0)
      IF("${out}" MATCHES "GNU")
        SET(MAKE_IS_GNU 1)
      ENDIF("${out}" MATCHES "GNU")
    ENDIF("${res}" EQUAL 0)
  ENDIF(${CMAKE_TEST_MAKEPROGRAM} MATCHES make)

  # only add this test on platforms that support it
  # some old versions of make simply cannot handle spaces in paths
  IF (MAKE_IS_GNU OR 
      "${CMAKE_TEST_MAKEPROGRAM}" MATCHES "nmake|gmake|wmake" OR
      "${CMAKE_TEST_GENERATOR}" MATCHES "Visual Studio|XCode|Borland")
    ADD_TEST(SubDirSpaces ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/SubDirSpaces"
      "${CMake_BINARY_DIR}/Tests/SubDirSpaces"
      --build-exe-dir 
      "${CMake_BINARY_DIR}/Tests/SubDirSpaces/Executable Sources"
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-project SUBDIR
      --test-command test
      "${CMake_BINARY_DIR}/Tests/SubDirSpaces/ShouldBeHere"
      "${CMake_BINARY_DIR}/Tests/SubDirSpaces/testfromsubdir.obj"
      )
  ENDIF (MAKE_IS_GNU OR
    "${CMAKE_TEST_MAKEPROGRAM}" MATCHES "nmake|gmake|wmake" OR
    "${CMAKE_TEST_GENERATOR}" MATCHES "Visual Studio|XCode|Borland")

  IF (WIN32)
    ADD_TEST(SubDir ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/SubDir"
      "${CMake_BINARY_DIR}/Tests/SubDir"
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/SubDir/Executable"
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-project SUBDIR
      --test-command test
      "${CMake_BINARY_DIR}/Tests/SubDir/ShouldBeHere"
      "${CMake_BINARY_DIR}/Tests/SubDir/testfromsubdir.obj"
      )
  ELSE (WIN32)
    ADD_TEST(SubDir ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/SubDir"
      "${CMake_BINARY_DIR}/Tests/SubDir"
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/SubDir/Executable"
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-project SUBDIR
      --test-command test
      "${CMake_BINARY_DIR}/Tests/SubDir/ShouldBeHere"
      "${CMake_BINARY_DIR}/Tests/SubDir/testfromsubdir.o"
      )
  ENDIF (WIN32)

  IF(CMAKE_TEST_MSVC)
    ADD_TEST_MACRO(PrecompiledHeader foo)
  ENDIF(CMAKE_TEST_MSVC)

  IF("${CMAKE_TEST_GENERATOR}" MATCHES "Makefile")
    ADD_TEST(MakeClean ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/MakeClean"
      "${CMake_BINARY_DIR}/Tests/MakeClean"
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-project MakeClean
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-exe-dir "${CMake_BINARY_DIR}/MakeClean"
      --test-command check_clean
      )
  ENDIF("${CMAKE_TEST_GENERATOR}" MATCHES "Makefile")

  IF(${CMAKE_TEST_GENERATOR} MATCHES "Visual Studio")
    ADD_TEST(VSExternalInclude ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/VSExternalInclude"
      "${CMake_BINARY_DIR}/Tests/VSExternalInclude"
      --build-two-config
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-project VSExternalInclude
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --test-command VSExternalInclude)
  ENDIF(${CMAKE_TEST_GENERATOR} MATCHES "Visual Studio")

  IF (APPLE AND CMAKE_COMPILER_IS_GNUCXX)
    SET(BundleTestInstallDir
      "${CMake_BINARY_DIR}/Tests/BundleTest/InstallDirectory")
    ADD_TEST(BundleTest ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/BundleTest"
      "${CMake_BINARY_DIR}/Tests/BundleTest"
      --build-two-config
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-project BundleTest
      --build-target install
#     --build-target package
      --build-options "-DCMAKE_INSTALL_PREFIX:PATH=${BundleTestInstallDir}"
       "-DCMake_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}"
      --test-command
      ${BundleTestInstallDir}/Applications/SecondBundleExe.app/Contents/MacOS/SecondBundleExe)

    ADD_TEST_MACRO(ObjC++ ObjC++)
  ENDIF (APPLE AND CMAKE_COMPILER_IS_GNUCXX)

  IF(APPLE AND CTEST_TEST_CPACK)
    ADD_TEST(BundleGeneratorTest ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/BundleGeneratorTest"
      "${CMake_BINARY_DIR}/Tests/BundleGeneratorTest"
      --build-two-config
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM} 
      --build-project BundleGeneratorTest
      --build-target package
      --build-options "-DCMAKE_INSTALL_PREFIX:PATH=${CMake_BINARY_DIR}/Tests/BundleGeneratorTest/InstallDirectory"
      )
  ENDIF(APPLE AND CTEST_TEST_CPACK)

  SET(CTEST_TEST_UPDATE 1)
  IF(CTEST_TEST_UPDATE)
    # Test CTest Update with Subversion
    FIND_PACKAGE(Subversion QUIET)
    IF(Subversion_FOUND)
      GET_FILENAME_COMPONENT(_Subversion_BIN_DIR
        ${Subversion_SVN_EXECUTABLE} PATH)
      FIND_PROGRAM(Subversion_SVNADMIN_EXECUTABLE svnadmin
        HINTS ${_Subversion_BIN_DIR}
        )
      MARK_AS_ADVANCED(Subversion_SVNADMIN_EXECUTABLE)
      IF(NOT Subversion_SVNADMIN_EXECUTABLE)
        SET(Subversion_FOUND FALSE)
      ENDIF(NOT Subversion_SVNADMIN_EXECUTABLE)
    ENDIF(Subversion_FOUND)
    IF(Subversion_FOUND)
      SET(CTestUpdateSVN_DIR "CTest UpdateSVN")
      CONFIGURE_FILE("${CMake_SOURCE_DIR}/Tests/CTestUpdateSVN.cmake.in"
        "${CMake_BINARY_DIR}/Tests/CTestUpdateSVN.cmake" @ONLY)
      ADD_TEST(CTest.UpdateSVN ${CMAKE_CMAKE_COMMAND}
        -P "${CMake_BINARY_DIR}/Tests/CTestUpdateSVN.cmake"
        )
    ENDIF(Subversion_FOUND)

    # Test CTest Update with CVS
    IF(UNIX)
      # The test is expected to work by default on UNIX-like systems.
      SET(CTEST_TEST_UPDATE_CVS 1)
    ELSE(UNIX)
      # Special CVS configuration is needed for test to pass on Windows.
      OPTION(CTEST_TEST_UPDATE_CVS
        "Enable CTest.UpdateCVS test.  Requires extra CVS setup on windows."
        OFF)
      MARK_AS_ADVANCED(CTEST_TEST_UPDATE_CVS)
    ENDIF(UNIX)
    IF(CTEST_TEST_UPDATE_CVS)
      IF(EXISTS ${CMAKE_ROOT}/Modules/FindCVS.cmake)
        FIND_PACKAGE(CVS QUIET)
      ELSE(EXISTS ${CMAKE_ROOT}/Modules/FindCVS.cmake)
        FIND_PROGRAM(CVS_EXECUTABLE NAMES cvs)
        SET(CVS_FOUND ${CVS_EXECUTABLE})
      ENDIF(EXISTS ${CMAKE_ROOT}/Modules/FindCVS.cmake)
    ENDIF(CTEST_TEST_UPDATE_CVS)
    IF(CTEST_TEST_UPDATE_CVS AND CVS_FOUND)
      SET(CTestUpdateCVS_DIR "CTest UpdateCVS")
      CONFIGURE_FILE("${CMake_SOURCE_DIR}/Tests/CTestUpdateCVS.cmake.in"
        "${CMake_BINARY_DIR}/Tests/CTestUpdateCVS.cmake" @ONLY)
      ADD_TEST(CTest.UpdateCVS ${CMAKE_CMAKE_COMMAND}
        -P "${CMake_BINARY_DIR}/Tests/CTestUpdateCVS.cmake"
        )
    ENDIF(CTEST_TEST_UPDATE_CVS AND CVS_FOUND)

  ENDIF(CTEST_TEST_UPDATE)

  IF (CTEST_TEST_CTEST AND CMAKE_RUN_LONG_TESTS)
    CONFIGURE_FILE("${CMake_SOURCE_DIR}/Tests/CTestTest/test.cmake.in"
      "${CMake_BINARY_DIR}/Tests/CTestTest/test.cmake" @ONLY ESCAPE_QUOTES)
    ADD_TEST(CTestTest ${CMAKE_CTEST_COMMAND}
      -S "${CMake_BINARY_DIR}/Tests/CTestTest/test.cmake" -V
      --output-log "${CMake_BINARY_DIR}/Tests/CTestTest/testOutput.log"
      )
    CONFIGURE_FILE("${CMake_SOURCE_DIR}/Tests/CTestTest2/test.cmake.in"
      "${CMake_BINARY_DIR}/Tests/CTestTest2/test.cmake" @ONLY ESCAPE_QUOTES)
    ADD_TEST(CTestTest2 ${CMAKE_CTEST_COMMAND}
      -S "${CMake_BINARY_DIR}/Tests/CTestTest2/test.cmake" -V
      --output-log "${CMake_BINARY_DIR}/Tests/CTestTest2/testOutput.log"
      )
    CONFIGURE_FILE("${CMake_SOURCE_DIR}/Tests/CTestTest3/test.cmake.in"
      "${CMake_BINARY_DIR}/Tests/CTestTest3/test.cmake" @ONLY ESCAPE_QUOTES)
    ADD_TEST(CTestTest3 ${CMAKE_CTEST_COMMAND}
      -S "${CMake_BINARY_DIR}/Tests/CTestTest3/test.cmake" -V
      --output-log "${CMake_BINARY_DIR}/Tests/CTestTest3/testOutput.log"
      )
    # these tests take a long time, make sure they have it
    IF(COMMAND SET_TESTS_PROPERTIES AND COMMAND GET_TEST_PROPERTY)
      # if timeouts have not already been set
      GET_TEST_PROPERTY(CTestTest TIMEOUT PREVIOUS_TIMEOUT)
      IF ("${PREVIOUS_TIMEOUT}" MATCHES NOTFOUND)
        SET_TESTS_PROPERTIES ( CTestTest
          PROPERTIES TIMEOUT ${CMAKE_LONG_TEST_TIMEOUT})
      ENDIF ("${PREVIOUS_TIMEOUT}" MATCHES NOTFOUND)

      GET_TEST_PROPERTY(CTestTest2 TIMEOUT PREVIOUS_TIMEOUT)
      IF ("${PREVIOUS_TIMEOUT}" MATCHES NOTFOUND)
        SET_TESTS_PROPERTIES ( CTestTest2
          PROPERTIES TIMEOUT ${CMAKE_LONG_TEST_TIMEOUT})
      ENDIF ("${PREVIOUS_TIMEOUT}" MATCHES NOTFOUND)

      GET_TEST_PROPERTY(CTestTest3 TIMEOUT PREVIOUS_TIMEOUT)
      IF ("${PREVIOUS_TIMEOUT}" MATCHES NOTFOUND)
        SET_TESTS_PROPERTIES ( CTestTest3
          PROPERTIES TIMEOUT ${CMAKE_LONG_TEST_TIMEOUT})
      ENDIF ("${PREVIOUS_TIMEOUT}" MATCHES NOTFOUND)
    ENDIF(COMMAND SET_TESTS_PROPERTIES AND COMMAND GET_TEST_PROPERTY)
  ENDIF (CTEST_TEST_CTEST AND CMAKE_RUN_LONG_TESTS)
  
  IF("${CMAKE_TEST_GENERATOR}" MATCHES Xcode)
    SET(CMAKE_SKIP_BOOTSTRAP_TEST 1)
  ENDIF("${CMAKE_TEST_GENERATOR}" MATCHES Xcode)
  IF(EXISTS "${CMAKE_BINARY_DIR}/CMakeLists.txt")
    # If there is CMakeLists.txt in the binary tree, assume in-source build
    SET(CMAKE_SKIP_BOOTSTRAP_TEST 1)
  ENDIF(EXISTS "${CMAKE_BINARY_DIR}/CMakeLists.txt")
  IF(UNIX AND CMAKE_RUN_LONG_TESTS AND NOT CMAKE_SKIP_BOOTSTRAP_TEST)


    ADD_TEST(BootstrapTest ${CMAKE_CTEST_COMMAND}
      --build-and-test
      ${CMake_SOURCE_DIR}
      ${CMake_BINARY_DIR}/Tests/BootstrapTest
      --build-nocmake
      --build-noclean
      --build-makeprogram ${CMake_SOURCE_DIR}/bootstrap
      --build-generator "${CMAKE_TEST_GENERATOR}"
      --test-command 
      ${CMake_BINARY_DIR}/Tests/BootstrapTest/Bootstrap.cmk/cmake)

    # provide more time for the bootstrap test
    IF (COMMAND GET_TEST_PROPERTY)
      GET_TEST_PROPERTY(BootstrapTest TIMEOUT PREVIOUS_TIMEOUT)
      IF ("${PREVIOUS_TIMEOUT}" MATCHES NOTFOUND)
        SET_TESTS_PROPERTIES ( BootstrapTest
          PROPERTIES TIMEOUT ${CMAKE_LONG_TEST_TIMEOUT})
      ENDIF ("${PREVIOUS_TIMEOUT}" MATCHES NOTFOUND)
    ENDIF (COMMAND GET_TEST_PROPERTY)
      
  ENDIF(UNIX AND CMAKE_RUN_LONG_TESTS AND NOT CMAKE_SKIP_BOOTSTRAP_TEST)

  # fortran does not work for IDE builds because
  # CMAKE_STANDARD_LIBRARIES needs to be per language
  IF(CMAKE_TEST_GENERATOR MATCHES "Makefiles"
     OR CMAKE_TEST_GENERATOR MATCHES "KDevelop")
    # see if we can find a fortran compiler on the machine
    # if so, add the fortran test and see if it works.
    SET(CMAKE_Fortran_COMPILER_LIST ifort ifc efc f95 pgf95
      lf95 xlf95 fort gfortran f90  pgf90   xlf90
      epcf90 f77  fort77 frt pgf77  xlf  fl32 af77 g77  )
    FIND_PROGRAM(CMAKE_Fortran_COMPILER_FULLPATH NAMES 
      ${CMAKE_Fortran_COMPILER_LIST} )
    MARK_AS_ADVANCED(CMAKE_Fortran_COMPILER_FULLPATH)
    IF(CMAKE_Fortran_COMPILER_FULLPATH)
      ADD_TEST(Fortran ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/Fortran"
        "${CMake_BINARY_DIR}/Tests/Fortran"
        --build-generator ${CMAKE_TEST_GENERATOR}
        --build-project testf
        --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
        --build-two-config
        --test-command testf)
    ENDIF(CMAKE_Fortran_COMPILER_FULLPATH)
  ENDIF(CMAKE_TEST_GENERATOR MATCHES "Makefiles"
     OR CMAKE_TEST_GENERATOR MATCHES "KDevelop")

  IF(NOT CMAKE_TEST_GENERATOR MATCHES "Xcode")
    INCLUDE(FindJava)
    IF(JAVA_COMPILE AND JAVA_RUNTIME AND JAVA_ARCHIVE AND NOT MINGW)
      GET_FILENAME_COMPONENT(JNIPATH ${JAVA_COMPILE} PATH)
      FIND_FILE(JNI_H jni.h
        "${JNIPATH}/../include"
        "${JNIPATH}/../java/include")
      IF(JNI_H)
        FILE(READ "${JNI_H}" JNI_FILE)
        IF("${JNI_FILE}" MATCHES "JDK1_2")
          ADD_TEST(Java ${CMAKE_CTEST_COMMAND}
            --build-and-test
            "${CMake_SOURCE_DIR}/Tests/Java"
            "${CMake_BINARY_DIR}/Tests/Java"
            --build-generator ${CMAKE_TEST_GENERATOR}
            --build-project hello
            --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
            --build-two-config
            --build-run-dir "${CMake_BINARY_DIR}/Tests/Java/"
            --test-command ${JAVA_RUNTIME} -classpath hello.jar HelloWorld)
        ENDIF("${JNI_FILE}" MATCHES "JDK1_2")
      ENDIF(JNI_H)
    ENDIF(JAVA_COMPILE AND JAVA_RUNTIME AND JAVA_ARCHIVE AND NOT MINGW)
  ENDIF(NOT CMAKE_TEST_GENERATOR MATCHES "Xcode")

  # add some cross compiler tests, for now only with makefile based generators
  IF(CMAKE_TEST_GENERATOR MATCHES "Makefiles" OR CMAKE_TEST_GENERATOR MATCHES "KDevelop")

    # if sdcc is found, build the SimpleCOnly project with sdcc
    FIND_PROGRAM(SDCC_EXECUTABLE sdcc)
    MARK_AS_ADVANCED(SDCC_EXECUTABLE)
    IF(SDCC_EXECUTABLE)
      ADD_TEST(SimpleCOnly_sdcc ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/SimpleCOnly"
        "${CMake_BINARY_DIR}/Tests/SimpleCOnly_sdcc"
        --build-generator ${CMAKE_TEST_GENERATOR}
        --build-project SimpleC
        --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
        --build-options
        "-DCMAKE_SYSTEM_NAME=Generic"
        "-DCMAKE_C_COMPILER=${SDCC_EXECUTABLE}")
    ENDIF(SDCC_EXECUTABLE)
    
    
    FIND_PROGRAM(MINGW_CC_LINUX2WIN_EXECUTABLE i586-mingw32msvc-gcc)
    FIND_PROGRAM(MINGW_CXX_LINUX2WIN_EXECUTABLE i586-mingw32msvc-g++)
    MARK_AS_ADVANCED(MINGW_CC_LINUX2WIN_EXECUTABLE MINGW_CXX_LINUX2WIN_EXECUTABLE)
    IF(MINGW_CC_LINUX2WIN_EXECUTABLE  AND  MINGW_CXX_LINUX2WIN_EXECUTABLE)
      ADD_TEST(Simple_Mingw_Linux2Win ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/Simple"
        "${CMake_BINARY_DIR}/Tests/Simple_Mingw_Linux2Win"
        --build-generator ${CMAKE_TEST_GENERATOR}
        --build-project Simple
        --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
        --build-options
        "-DCMAKE_SYSTEM_NAME=Windows"
        "-DCMAKE_C_COMPILER=${MINGW_CC_LINUX2WIN_EXECUTABLE}"
        "-DCMAKE_CXX_COMPILER=${MINGW_CXX_LINUX2WIN_EXECUTABLE}")
    ENDIF(MINGW_CC_LINUX2WIN_EXECUTABLE  AND  MINGW_CXX_LINUX2WIN_EXECUTABLE)
    
    
  ENDIF(CMAKE_TEST_GENERATOR MATCHES "Makefiles" OR CMAKE_TEST_GENERATOR MATCHES "KDevelop")

  IF(BUILD_WXDialog AND wxWidgets_CONFIG_EXECUTABLE)
    # Will be set if the WX gui is on
    ADD_TEST(UseWX ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/UseWX"
      "${CMake_BINARY_DIR}/Tests/UseWX"
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-exe-dir ${CMake_BINARY_DIR}/Tests/UseWX
      --build-project UsewxWidgets
      --build-options -DwxWidgets_CONFIG_EXECUTABLE=${wxWidgets_CONFIG_EXECUTABLE}
      --test-command UseWX
      )
  ENDIF(BUILD_WXDialog AND wxWidgets_CONFIG_EXECUTABLE)

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

  IF(CMAKE_TEST_PLPLOT_DIR)
    ADD_TEST(plplot ${CMAKE_CTEST_COMMAND} -V -S ${CMAKE_TEST_PLPLOT_DIR}/../../EasyDashboardScripts/plplot.cmake )
    SET_TESTS_PROPERTIES ( plplot PROPERTIES TIMEOUT 5400)
  ENDIF(CMAKE_TEST_PLPLOT_DIR)

  IF(CMAKE_TEST_CHICKEN_DIR)
    ADD_TEST(Chicken ${CMAKE_CTEST_COMMAND} -V -S ${CMAKE_TEST_CHICKEN_DIR}/../../EasyDashboardScripts/Chicken.cmake )
    SET_TESTS_PROPERTIES ( Chicken PROPERTIES TIMEOUT 5400)
  ENDIF(CMAKE_TEST_CHICKEN_DIR)

  IF(CMAKE_TEST_KDELIBS_ALPHA_1_DIR)
    ADD_TEST(KDELibsAlpha1 ${CMAKE_CTEST_COMMAND} -V -S ${CMAKE_TEST_KDELIBS_ALPHA_1_DIR}/../../EasyDashboardScripts/kdelibs.cmake )
    SET_TESTS_PROPERTIES ( KDELibsAlpha1 PROPERTIES TIMEOUT 5400)
  ENDIF(CMAKE_TEST_KDELIBS_ALPHA_1_DIR)

ENDIF(BUILD_TESTING)

SUBDIRS(CMakeTests)
