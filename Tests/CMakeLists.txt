# a macro for tests that have a simple format where the name matches the
# directory and project
MACRO(ADD_TEST_MACRO NAME COMMAND)
  STRING(REPLACE "." "/" dir "${NAME}")
  STRING(REGEX REPLACE "[^.]*\\." "" proj "${NAME}")
  ADD_TEST(${NAME} ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/${dir}"
    "${CMake_BINARY_DIR}/Tests/${dir}"
    --build-two-config
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-project ${proj}
    --test-command ${COMMAND})
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${dir}")
ENDMACRO(ADD_TEST_MACRO)

# Fake a user home directory to avoid polluting the real one.
IF(DEFINED ENV{HOME} AND NOT CTEST_NO_TEST_HOME)
  SET(TEST_HOME "${CMake_BINARY_DIR}/Tests/CMakeFiles/TestHome")
  FILE(MAKE_DIRECTORY "${TEST_HOME}")
  FILE(WRITE "${TEST_HOME}/.cvspass" ":pserver:anoncvs@www.cmake.org:/cvsroot/KWSys A\n")
  SET(TEST_HOME_ENV_CODE "# Fake a user home directory to avoid polluting the real one.
SET(ENV{HOME} \"${TEST_HOME}\")")
ENDIF()

# Choose a default configuration for CTest tests.
SET(CTestTest_CONFIG Debug)
IF(NOT CMAKE_CONFIGURATION_TYPES AND CMAKE_BUILD_TYPE)
  SET(CTestTest_CONFIG ${CMAKE_BUILD_TYPE})
ENDIF()

CONFIGURE_FILE(${CMake_SOURCE_DIR}/Tests/EnforceConfig.cmake.in
               ${CMake_BINARY_DIR}/Tests/EnforceConfig.cmake @ONLY)

# Testing
IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(CMakeLib)

  # Collect a list of all test build directories.
  SET(TEST_BUILD_DIRS)

  # Should the long tests be run?
  OPTION(CMAKE_RUN_LONG_TESTS 
    "Should the long tests be run (such as Bootstrap)." ON)
  MARK_AS_ADVANCED(CMAKE_RUN_LONG_TESTS)

  IF (CMAKE_RUN_LONG_TESTS)
    OPTION(CTEST_TEST_CTEST 
      "Should the tests that run a full sub ctest process be run?" 
      OFF)
    MARK_AS_ADVANCED(CTEST_TEST_CTEST)

    OPTION(TEST_KDE4_STABLE_BRANCH
      "Should the KDE4 stable branch test be run?" 
      OFF)
    MARK_AS_ADVANCED(TEST_KDE4_STABLE_BRANCH)
  ENDIF (CMAKE_RUN_LONG_TESTS)

  # Should tests that use CVS be run?
  #
  set(do_cvs_tests 0)

  if(EXISTS ${CMAKE_ROOT}/Modules/FindCVS.cmake)
    find_package(CVS QUIET)
  else(EXISTS ${CMAKE_ROOT}/Modules/FindCVS.cmake)
    find_program(CVS_EXECUTABLE NAMES cvs)
  endif(EXISTS ${CMAKE_ROOT}/Modules/FindCVS.cmake)

  if(CVS_EXECUTABLE)
    set(do_cvs_tests 1)
  endif(CVS_EXECUTABLE)

  if(do_cvs_tests AND NOT UNIX)
    if("${CVS_EXECUTABLE}" MATCHES "cygwin")
      set(do_cvs_tests 0)
    endif("${CVS_EXECUTABLE}" MATCHES "cygwin")
  endif(do_cvs_tests AND NOT UNIX)

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
  SET(CTEST_TEST_OSX_ARCH 0)
  IF(APPLE)
    EXECUTE_PROCESS(
      COMMAND sw_vers -productVersion
      OUTPUT_VARIABLE OSX_VERSION
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
    IF(OSX_VERSION MATCHES "^10\\.[0123]" OR OSX_VERSION MATCHES "ProductVersion:\t10\\.[0123]")
      MESSAGE(STATUS "Forcing CTEST_TEST_CPACK=OFF on OSX < 10.4")
      MESSAGE(STATUS "OSX_VERSION='${OSX_VERSION}'")
      SET(CTEST_TEST_CPACK OFF)
    ELSE(OSX_VERSION MATCHES "^10\\.[0123]" OR OSX_VERSION MATCHES "ProductVersion:\t10\\.[0123]")
      SET(CTEST_TEST_OSX_ARCH 1)
    ENDIF(OSX_VERSION MATCHES "^10\\.[0123]" OR OSX_VERSION MATCHES "ProductVersion:\t10\\.[0123]")
  ENDIF(APPLE)

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
  ADD_TEST_MACRO(IPO COnly/COnly)
  ADD_TEST_MACRO(OutDir runtime/OutDir)
  ADD_TEST_MACRO(NewlineArgs NewlineArgs)
  ADD_TEST_MACRO(SetLang SetLang)
  ADD_TEST_MACRO(ExternalOBJ ExternalOBJ)
  ADD_TEST_MACRO(LoadCommand LoadedCommand)
  ADD_TEST_MACRO(LinkDirectory bin/LinkDirectory)
  ADD_TEST_MACRO(LinkLanguage LinkLanguage)
  ADD_TEST_MACRO(LinkLine LinkLine)
  ADD_TEST_MACRO(MacroTest miniMacroTest)
  ADD_TEST_MACRO(FunctionTest miniFunctionTest)
  ADD_TEST_MACRO(ReturnTest ReturnTest)
  ADD_TEST_MACRO(Properties Properties)
  ADD_TEST_MACRO(Assembler HelloAsm)
  ADD_TEST_MACRO(SourceGroups SourceGroups)
  ADD_TEST_MACRO(Preprocess Preprocess)
  ADD_TEST_MACRO(ExportImport ExportImport)
  ADD_TEST_MACRO(Unset Unset)
  ADD_TEST_MACRO(PolicyScope PolicyScope)
  ADD_TEST_MACRO(CrossCompile CrossCompile)
  SET_TESTS_PROPERTIES(CrossCompile PROPERTIES
    PASS_REGULAR_EXPRESSION "TRY_RUN.. invoked in cross-compiling mode")
  IF("${CMAKE_TEST_GENERATOR}" MATCHES "Make")
    ADD_TEST_MACRO(Policy0002 Policy0002)
  ENDIF("${CMAKE_TEST_GENERATOR}" MATCHES "Make")
  IF(CTEST_TEST_OSX_ARCH)
    ADD_TEST_MACRO(Architecture Architecture)
    SET_TESTS_PROPERTIES(Architecture PROPERTIES
      PASS_REGULAR_EXPRESSION "(file is not of required architecture|does not match cputype|not the architecture being linked)")
  ENDIF(CTEST_TEST_OSX_ARCH)

  LIST(APPEND TEST_BUILD_DIRS ${CMake_TEST_INSTALL_PREFIX})

  SET(CMAKE_BUILD_TEST_SOURCE_DIR "${CMake_SOURCE_DIR}/Tests/COnly")
  SET(CMAKE_BUILD_TEST_BINARY_DIR "${CMake_BINARY_DIR}/Tests/CMakeBuildCOnly")
  CONFIGURE_FILE("${CMake_SOURCE_DIR}/Tests/CMakeBuildTest.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CMakeBuildTest.cmake" @ONLY)
  ADD_TEST(CMakeBuildTest ${CMAKE_CMAKE_COMMAND} -P
    "${CMake_BINARY_DIR}/Tests/CMakeBuildTest.cmake")
  LIST(APPEND TEST_BUILD_DIRS ${CMAKE_BUILD_TEST_BINARY_DIR})

  ADD_TEST_MACRO(Module.CheckTypeSize CheckTypeSize)

  ADD_TEST(LinkFlags-prepare
    ${CMAKE_CTEST_COMMAND} -C \${CTEST_CONFIGURATION_TYPE}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/LinkFlags"
    "${CMake_BINARY_DIR}/Tests/LinkFlags"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-project LinkFlags
    --build-target LinkFlags
    --build-options -DTEST_CONFIG=\${CTEST_CONFIGURATION_TYPE}
    )
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/LinkFlags")

  MACRO(ADD_LINK_FLAGS_TEST name depends)
    ADD_TEST(LinkFlags-${name}
      ${CMAKE_CMAKE_COMMAND} --build "${CMake_BINARY_DIR}/Tests/LinkFlags"
      --target LinkFlags_${name} --config \${CTEST_CONFIGURATION_TYPE}
      )
    SET_TESTS_PROPERTIES(LinkFlags-${name} PROPERTIES
      PASS_REGULAR_EXPRESSION "BADFLAG" DEPENDS LinkFlags-${depends})
  ENDMACRO()
  ADD_LINK_FLAGS_TEST(lib prepare)
  ADD_LINK_FLAGS_TEST(dll lib)
  ADD_LINK_FLAGS_TEST(exe dll)
  ADD_LINK_FLAGS_TEST(lib_config exe)
  ADD_LINK_FLAGS_TEST(dll_config lib_config)
  ADD_LINK_FLAGS_TEST(exe_config dll_config)

  # If we are running right now with a UnixMakefiles based generator,
  # build the "Simple" test with the ExtraGenerators, if available
  # This doesn't test whether the generated project files work (unfortunately), 
  # mainly it tests that cmake doesn't crash when generating these project files.
  IF(${CMAKE_TEST_GENERATOR} MATCHES "Unix Makefiles"  OR  ${CMAKE_TEST_GENERATOR} MATCHES "KDevelop")
    # check which generators we have
    EXEC_PROGRAM(${CMAKE_CMAKE_COMMAND} ARGS --help OUTPUT_VARIABLE cmakeOutput )
    # check for the Eclipse generator
    IF ("${cmakeOutput}" MATCHES Eclipse)
      ADD_TEST(Simple_EclipseGenerator ${CMAKE_CTEST_COMMAND}
         --build-and-test
         "${CMake_SOURCE_DIR}/Tests/Simple"
         "${CMake_BINARY_DIR}/Tests/Simple_EclipseGenerator"
         --build-two-config
         --build-generator "Eclipse CDT4 - Unix Makefiles"
         --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
         --build-project Simple
         --test-command Simple)
      LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Simple_EclipseGenerator")
    ENDIF ("${cmakeOutput}" MATCHES Eclipse)

    # check for the CodeBlocks generator
    IF ("${cmakeOutput}" MATCHES CodeBlocks)
      ADD_TEST(Simple_CodeBlocksGenerator ${CMAKE_CTEST_COMMAND}
         --build-and-test
         "${CMake_SOURCE_DIR}/Tests/Simple"
         "${CMake_BINARY_DIR}/Tests/Simple_CodeBlocksGenerator"
         --build-two-config
         --build-generator "CodeBlocks - Unix Makefiles"
         --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
         --build-project Simple
         --test-command Simple)
      LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Simple_CodeBlocksGenerator")
    ENDIF ("${cmakeOutput}" MATCHES CodeBlocks)

    # check for the KDevelop3 generator
    IF ("${cmakeOutput}" MATCHES KDevelop3)
      ADD_TEST(Simple_KDevelop3Generator ${CMAKE_CTEST_COMMAND}
         --build-and-test
         "${CMake_SOURCE_DIR}/Tests/Simple"
         "${CMake_BINARY_DIR}/Tests/Simple_KDevelop3Generator"
         --build-two-config
         --build-generator "KDevelop3 - Unix Makefiles"
         --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
         --build-project Simple
         --test-command Simple)
      LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Simple_KDevelop3Generator")
    ENDIF ("${cmakeOutput}" MATCHES KDevelop3)

  ENDIF(${CMAKE_TEST_GENERATOR} MATCHES "Unix Makefiles"  OR  ${CMAKE_TEST_GENERATOR} MATCHES "KDevelop")

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
    SET_TESTS_PROPERTIES ( SubProject-Stage2 PROPERTIES DEPENDS SubProject)
    LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/SubProject")
  ENDIF(NOT MSVC60 AND NOT XCODE AND NOT MSVC70)

  IF (CMAKE_STRICT)
    ADD_TEST_MACRO(DocTest DocTest)
  ENDIF (CMAKE_STRICT)
  # macro to add a test that will build a nightly release
  # of CMake for given platform using the release scripts
  MACRO(ADD_NIGHTLY_BUILD_TEST name script)
    SET(_TEST_DIR "${CMake_BINARY_DIR}/Tests/${name}")
    FILE(MAKE_DIRECTORY "${_TEST_DIR}")
    FILE(WRITE "${_TEST_DIR}/nightly-cmake.sh"
      "cd ${_TEST_DIR}
${CMake_BINARY_DIR}/bin/cmake -DCMAKE_CREATE_VERSION=next -P ${CMake_SOURCE_DIR}/Utilities/Release/${script}
${CMake_BINARY_DIR}/bin/cmake -DVERSION=master -P ${CMake_SOURCE_DIR}/Utilities/Release/upload_release.cmake
    ")
    ADD_TEST(${name} /bin/sh ${_TEST_DIR}/nightly-cmake.sh)
    IF(COMMAND SET_TESTS_PROPERTIES AND COMMAND GET_TEST_PROPERTY)
      SET_TESTS_PROPERTIES (${name} PROPERTIES TIMEOUT ${CMAKE_LONG_TEST_TIMEOUT})
    ENDIF(COMMAND SET_TESTS_PROPERTIES AND COMMAND GET_TEST_PROPERTY)
  ENDMACRO(ADD_NIGHTLY_BUILD_TEST)
  IF(CMAKE_BUILD_NIGHTLY_RELEASES)
    ADD_NIGHTLY_BUILD_TEST(CMakeNightlyWindows
      dash2win64_release.cmake)
    ADD_NIGHTLY_BUILD_TEST(CMakeNightlyMac
      dashmacmini2_release.cmake)
    ADD_NIGHTLY_BUILD_TEST(CMakeNightlyLinux
      magrathea_release.cmake)
  ENDIF(CMAKE_BUILD_NIGHTLY_RELEASES)

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
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Framework")

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
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/TargetName")

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
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/LibName")

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
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CustComDepend")

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
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CustomCommand")

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
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CustomCommandWorkingDirectory")

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
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/OutOfSource")
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/OutOfSourceDeep")
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/OutOfBinary")

  ADD_TEST(BuildDepends ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/BuildDepends"
    "${CMake_BINARY_DIR}/Tests/BuildDepends"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project BuildDepends
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    )
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/BuildDepends")
  
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
    "-DCTEST_TEST_CPACK:BOOL=${CTEST_TEST_CPACK}"
    --test-command   ${SimpleInstallInstallDir}/MyTest/bin/SimpleInstExe)
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/SimpleInstall")
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
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/SimpleInstallS2")

  # By default, run the CPackComponents test if the CTEST_TEST_CPACK
  # option is ON:
  #
  set(CTEST_RUN_CPackComponents ${CTEST_TEST_CPACK})
  set(CTEST_package_X11_TEST ${CTEST_TEST_CPACK})

  find_program(NSIS_MAKENSIS_EXECUTABLE NAMES makensis
    PATHS [HKEY_LOCAL_MACHINE\\SOFTWARE\\NSIS]
    DOC "makensis program location"
    )

  # But on Windows, only run the CPackComponents test if the NSIS
  # installer builder is available:
  #
  if(WIN32)
    if(NSIS_MAKENSIS_EXECUTABLE)
      set(CTEST_RUN_CPackComponents ON)
    else(NSIS_MAKENSIS_EXECUTABLE)
      set(CTEST_RUN_CPackComponents OFF)
      set(CTEST_package_X11_TEST OFF)
    endif(NSIS_MAKENSIS_EXECUTABLE)
  endif(WIN32)

  IF(CTEST_RUN_CPackComponents)
    set(CPackComponents_EXTRA_OPTIONS)
    if(APPLE)
      set(CPackComponents_EXTRA_OPTIONS -DCPACK_BINARY_DRAGNDROP:BOOL=ON)
    endif(APPLE)
    if(NSIS_MAKENSIS_EXECUTABLE)
      set(CPackComponents_EXTRA_OPTIONS ${CPackComponents_EXTRA_OPTIONS}
        -DCPACK_BINARY_NSIS:BOOL=ON)
    endif(NSIS_MAKENSIS_EXECUTABLE)

    ADD_TEST(CPackComponents ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/CPackComponents"
      "${CMake_BINARY_DIR}/Tests/CPackComponents"
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-project CPackComponents
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-two-config
      --build-target package
      --build-options
        -DCPACK_BINARY_DEB:BOOL=${CPACK_BINARY_DEB}
        -DCPACK_BINARY_RPM:BOOL=${CPACK_BINARY_RPM}
        ${CPackComponents_EXTRA_OPTIONS}
        --graphviz=CPackComponents.dot
      --test-command ${CMAKE_CMAKE_COMMAND}
        "-DCPackComponents_BINARY_DIR:PATH=${CMake_BINARY_DIR}/Tests/CPackComponents"
        -P "${CMake_SOURCE_DIR}/Tests/CPackComponents/VerifyResult.cmake")
    LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CPackComponents")
  ENDIF(CTEST_RUN_CPackComponents)

  # By default, turn this test off (because it takes a long time...)
  #
  if(NOT DEFINED CTEST_RUN_CPackTestAllGenerators)
    set(CTEST_RUN_CPackTestAllGenerators OFF)

    # ...but: if it appears to be a coverage dashboard, or long tests are
    # on, then set it to the generic CTEST_TEST_CPACK setting.
    #
    if(CMAKE_CXX_FLAGS MATCHES "-ftest-coverage" OR
      NOT "$ENV{COVFILE}" STREQUAL "" OR
      CMAKE_RUN_LONG_TESTS)
      set(CTEST_RUN_CPackTestAllGenerators ${CTEST_TEST_CPACK})
    endif(CMAKE_CXX_FLAGS MATCHES "-ftest-coverage" OR
      NOT "$ENV{COVFILE}" STREQUAL "" OR
      CMAKE_RUN_LONG_TESTS)
  endif(NOT DEFINED CTEST_RUN_CPackTestAllGenerators)

  IF(CTEST_RUN_CPackTestAllGenerators)
    ADD_TEST(CPackTestAllGenerators ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/CPackTestAllGenerators"
      "${CMake_BINARY_DIR}/Tests/CPackTestAllGenerators"
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-project CPackTestAllGenerators
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --test-command
      ${CMAKE_CMAKE_COMMAND}
        -D dir=${CMake_BINARY_DIR}/Tests/CPackTestAllGenerators
        -D cpack=${CMAKE_CPACK_COMMAND}
        -P ${CMake_SOURCE_DIR}/Tests/CPackTestAllGenerators/RunCPack.cmake
      )
    LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CPackTestAllGenerators")
  ENDIF(CTEST_RUN_CPackTestAllGenerators)

  IF(CTEST_package_X11_TEST)
    SET(X11_build_target_arg --build-target package)
  ELSE(CTEST_package_X11_TEST)
    SET(X11_build_target_arg)
  ENDIF(CTEST_package_X11_TEST)

  ADD_TEST(X11 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/X11"
    "${CMake_BINARY_DIR}/Tests/X11"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project UseX11
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-two-config
    ${X11_build_target_arg}
    --test-command  UseX11)
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/X11")

  if(NOT DEFINED CTEST_RUN_CMakeTestAllGenerators)
    set(CTEST_RUN_CMakeTestAllGenerators ON)
  endif(NOT DEFINED CTEST_RUN_CMakeTestAllGenerators)

  IF(CTEST_RUN_CMakeTestAllGenerators)
    ADD_TEST(CMakeTestAllGenerators ${CMAKE_CMAKE_COMMAND}
        -D dir=${CMake_BINARY_DIR}/Tests/CMakeTestAllGenerators
        -D CMake_SOURCE_DIR=${CMake_SOURCE_DIR}
        -P ${CMake_SOURCE_DIR}/Tests/CMakeTestAllGenerators/RunCMake.cmake
      )
    LIST(APPEND TEST_BUILD_DIRS
      "${CMake_BINARY_DIR}/Tests/CMakeTestAllGenerators")
  ENDIF(CTEST_RUN_CMakeTestAllGenerators)

  if(NOT DEFINED CTEST_RUN_CMakeTestBadCommandLines)
    set(CTEST_RUN_CMakeTestBadCommandLines ON)
  endif(NOT DEFINED CTEST_RUN_CMakeTestBadCommandLines)

  IF(CTEST_RUN_CMakeTestBadCommandLines)
    ADD_TEST(CMakeTestBadCommandLines ${CMAKE_CMAKE_COMMAND}
        -D dir=${CMake_BINARY_DIR}/Tests/CMakeTestBadCommandLines
        -D gen=${CMAKE_TEST_GENERATOR}
        -D CMake_SOURCE_DIR=${CMake_SOURCE_DIR}
        -P ${CMake_SOURCE_DIR}/Tests/CMakeTestBadCommandLines/RunCMake.cmake
      )
    LIST(APPEND TEST_BUILD_DIRS
      "${CMake_BINARY_DIR}/Tests/CMakeTestBadCommandLines")
  ENDIF(CTEST_RUN_CMakeTestBadCommandLines)

  if(NOT DEFINED CTEST_RUN_CMakeTestMultipleConfigures)
    set(CTEST_RUN_CMakeTestMultipleConfigures ON)
  endif(NOT DEFINED CTEST_RUN_CMakeTestMultipleConfigures)

  IF(CTEST_RUN_CMakeTestMultipleConfigures)
    ADD_TEST(CMakeTestMultipleConfigures ${CMAKE_CMAKE_COMMAND}
        -D dir=${CMake_BINARY_DIR}/Tests/CMakeTestMultipleConfigures
        -D gen=${CMAKE_TEST_GENERATOR}
        -D CMake_SOURCE_DIR=${CMake_SOURCE_DIR}
        -P ${CMake_SOURCE_DIR}/Tests/CMakeTestMultipleConfigures/RunCMake.cmake
      )
    LIST(APPEND TEST_BUILD_DIRS
      "${CMake_BINARY_DIR}/Tests/CMakeTestMultipleConfigures")
  ENDIF(CTEST_RUN_CMakeTestMultipleConfigures)

  ADD_TEST(LoadedCommandOneConfig  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/LoadCommandOneConfig"
    "${CMake_BINARY_DIR}/Tests/LoadCommandOneConfig"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project LoadCommand
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --test-command  LoadedCommand
    )
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/LoadCommandOneConfig")

  # Como does not seem to support shared libraries.
  GET_FILENAME_COMPONENT(CMAKE_BASE_NAME ${CMAKE_CXX_COMPILER} NAME_WE)
  IF(CMAKE_BASE_NAME MATCHES "^como$")
    SET(COMPILER_IS_COMO 1)
  ENDIF(CMAKE_BASE_NAME MATCHES "^como$")
  IF(NOT COMPILER_IS_COMO)
    SET(COMPLEX_TEST_CMAKELIB 1)
    IF(CMAKE_TEST_DIFFERENT_GENERATOR OR CMAKE_TEST_SYSTEM_LIBRARIES)
      SET(COMPLEX_TEST_CMAKELIB 0)
    ENDIF(CMAKE_TEST_DIFFERENT_GENERATOR OR CMAKE_TEST_SYSTEM_LIBRARIES)
    IF(BORLAND)
      SET(COMPLEX_TEST_CMAKELIB 0)
    ENDIF(BORLAND)
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
      -DCOMPLEX_TEST_CMAKELIB:BOOL=${COMPLEX_TEST_CMAKELIB}
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      --test-command complex
      )
    LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Complex")

    ADD_TEST(complexOneConfig  ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/ComplexOneConfig"
      "${CMake_BINARY_DIR}/Tests/ComplexOneConfig"
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-project Complex
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/ComplexOneConfig/bin"
      --build-options
      -DCOMPLEX_TEST_CMAKELIB:BOOL=${COMPLEX_TEST_CMAKELIB}
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      --test-command complex)
    LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/ComplexOneConfig")
    # because of the registry write these tests depend on each other
    SET_TESTS_PROPERTIES ( complex PROPERTIES DEPENDS complexOneConfig)

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
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Example")

  ADD_TEST(Environment ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Environment"
    "${CMake_BINARY_DIR}/Tests/Environment"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project EnvironmentProj
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Environment"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Environment")

  ADD_TEST(ExternalProject ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/ExternalProject"
    "${CMake_BINARY_DIR}/Tests/ExternalProject"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project ExternalProjectTest
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/ExternalProject"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/ExternalProject")
  # The ExternalProject test takes 900 seconds on some machines!
  GET_TEST_PROPERTY(ExternalProject TIMEOUT PREVIOUS_TIMEOUT)
  IF("${PREVIOUS_TIMEOUT}" MATCHES NOTFOUND)
    SET_TESTS_PROPERTIES(ExternalProject PROPERTIES TIMEOUT 1000)
  ENDIF("${PREVIOUS_TIMEOUT}" MATCHES NOTFOUND)

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
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Tutorial")

  ADD_TEST(testing ${CMAKE_CTEST_COMMAND} -C \${CTEST_CONFIGURATION_TYPE}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Testing"
    "${CMake_BINARY_DIR}/Tests/Testing"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project Testing
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --test-command ${CMAKE_CTEST_COMMAND} -C \${CTEST_CONFIGURATION_TYPE}
    )
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Testing")

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
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Wrapping")

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
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/TestDriver")

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
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Dependency")

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
    LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Jump")

    ADD_TEST(Plugin ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/Plugin"
      "${CMake_BINARY_DIR}/Tests/Plugin"
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-project Plugin
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-two-config
      --test-command bin/example)
    LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Plugin")

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
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/LinkLineOrder")
  SET_TESTS_PROPERTIES ( qtwrapping PROPERTIES DEPENDS wrapping)
  SET_TESTS_PROPERTIES ( testdriver1 PROPERTIES DEPENDS qtwrapping)
  SET_TESTS_PROPERTIES ( testdriver2 PROPERTIES DEPENDS testdriver1)
  SET_TESTS_PROPERTIES ( testdriver3 PROPERTIES DEPENDS testdriver2)
  SET_TESTS_PROPERTIES ( linkorder2 PROPERTIES DEPENDS linkorder1)
  SET_TESTS_PROPERTIES ( SimpleInstall-Stage2 PROPERTIES DEPENDS SimpleInstall)

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
    LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/kwsys")
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
    LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/SubDirSpaces")
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
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/SubDir")

  IF(CMAKE_TEST_MSVC)
    ADD_TEST_MACRO(PrecompiledHeader foo)
    ADD_TEST_MACRO(ModuleDefinition example_exe)
  ENDIF(CMAKE_TEST_MSVC)

  ADD_TEST_MACRO(CheckCompilerRelatedVariables CheckCompilerRelatedVariables)

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
    LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/MakeClean")
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
    LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/VSExternalInclude")
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
    LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/BundleTest")

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
    LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/BundleGeneratorTest")
  ENDIF(APPLE AND CTEST_TEST_CPACK)

  # Make sure CTest can handle a test with no newline in output.
  ADD_TEST(CTest.NoNewline
    ${CMAKE_CMAKE_COMMAND} -E echo_append "This line has no newline!")

  # A simple test for ctest in script mode
  CONFIGURE_FILE("${CMake_SOURCE_DIR}/Tests/CTestScriptMode/CTestTestScriptMode.cmake.in"
          "${CMake_BINARY_DIR}/Tests/CTestScriptMode/CTestTestScriptMode.cmake" @ONLY)
#  ADD_TEST(CTest.ScriptMode ${CMAKE_CTEST_COMMAND}
#        -S "${CMake_BINARY_DIR}/Tests/CTestScriptMode/CTestTestScriptMode.cmake"
#        )

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
      LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${CTestUpdateSVN_DIR}")
    ENDIF(Subversion_FOUND)

    # Test CTest Update with CVS
    IF(EXISTS ${CMAKE_ROOT}/Modules/FindCVS.cmake)
      FIND_PACKAGE(CVS QUIET)
    ELSE(EXISTS ${CMAKE_ROOT}/Modules/FindCVS.cmake)
      FIND_PROGRAM(CVS_EXECUTABLE NAMES cvs)
      SET(CVS_FOUND ${CVS_EXECUTABLE})
    ENDIF(EXISTS ${CMAKE_ROOT}/Modules/FindCVS.cmake)
    SET(CTEST_TEST_UPDATE_CVS ${CVS_FOUND})
    IF(CTEST_TEST_UPDATE_CVS AND NOT UNIX)
      IF("${CVS_EXECUTABLE}" MATCHES "cygwin")
        MESSAGE(STATUS "No CTest.UpdateCVS test with cygwin cvs.exe outside cygwin!")
        SET(CTEST_TEST_UPDATE_CVS 0)
      ENDIF("${CVS_EXECUTABLE}" MATCHES "cygwin")
    ENDIF(CTEST_TEST_UPDATE_CVS AND NOT UNIX)
    IF(CTEST_TEST_UPDATE_CVS)
      SET(CTestUpdateCVS_DIR "CTest UpdateCVS")
      CONFIGURE_FILE("${CMake_SOURCE_DIR}/Tests/CTestUpdateCVS.cmake.in"
        "${CMake_BINARY_DIR}/Tests/CTestUpdateCVS.cmake" @ONLY)
      ADD_TEST(CTest.UpdateCVS ${CMAKE_CMAKE_COMMAND}
        -P "${CMake_BINARY_DIR}/Tests/CTestUpdateCVS.cmake"
        )
      LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${CTestUpdateCVS_DIR}")
    ENDIF(CTEST_TEST_UPDATE_CVS)

    # Test CTest Update with BZR
    FIND_PROGRAM(BZR_EXECUTABLE NAMES bzr)
    MARK_AS_ADVANCED(BZR_EXECUTABLE)
    SET(CTEST_TEST_UPDATE_BZR 0)
    IF(BZR_EXECUTABLE)
      IF(NOT "${BZR_EXECUTABLE}" MATCHES "cygwin" OR UNIX)
        SET(CTEST_TEST_UPDATE_BZR 1)
      ENDIF(NOT "${BZR_EXECUTABLE}" MATCHES "cygwin" OR UNIX)
    ENDIF(BZR_EXECUTABLE)
    IF(CTEST_TEST_UPDATE_BZR)
      # Check if xmloutput plugin is there
      EXECUTE_PROCESS(COMMAND ${BZR_EXECUTABLE} xmlplugins RESULT_VARIABLE xmlplugres
        OUTPUT_QUIET ERROR_QUIET)
      IF( NOT ${xmlplugres} )
        SET(CTestUpdateBZR_DIR "CTest UpdateBZR")
        CONFIGURE_FILE("${CMake_SOURCE_DIR}/Tests/CTestUpdateBZR.cmake.in"
          "${CMake_BINARY_DIR}/Tests/CTestUpdateBZR.cmake" @ONLY)
        ADD_TEST(CTest.UpdateBZR ${CMAKE_CMAKE_COMMAND}
          -P "${CMake_BINARY_DIR}/Tests/CTestUpdateBZR.cmake"
          )
        LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${CTestUpdateBZR_DIR}")
        SET(CTestUpdateBZR_DIR "CTest UpdateBZR_CLocale")
        CONFIGURE_FILE("${CMake_SOURCE_DIR}/Tests/CTestUpdateBZR.cmake.in"
          "${CMake_BINARY_DIR}/Tests/CTestUpdateBZR_CLocale.cmake" @ONLY)
        ADD_TEST(CTest.UpdateBZR.CLocale ${CMAKE_CMAKE_COMMAND}
          -P "${CMake_BINARY_DIR}/Tests/CTestUpdateBZR_CLocale.cmake"
          )
        SET_TESTS_PROPERTIES(CTest.UpdateBZR.CLocale PROPERTIES ENVIRONMENT LC_ALL=C)
        LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${CTestUpdateBZR_DIR}")
      ENDIF( NOT ${xmlplugres} )
    ENDIF(CTEST_TEST_UPDATE_BZR)

    # Test CTest Update with GIT
    FIND_PROGRAM(GIT_EXECUTABLE NAMES git)
    MARK_AS_ADVANCED(GIT_EXECUTABLE)
    SET(CTEST_TEST_UPDATE_GIT 0)
    IF(GIT_EXECUTABLE)
      IF(NOT "${GIT_EXECUTABLE}" MATCHES "cygwin" OR UNIX)
        SET(CTEST_TEST_UPDATE_GIT 1)
      ENDIF(NOT "${GIT_EXECUTABLE}" MATCHES "cygwin" OR UNIX)
    ENDIF(GIT_EXECUTABLE)
    IF(CTEST_TEST_UPDATE_GIT)
      SET(CTestUpdateGIT_DIR "CTest UpdateGIT")
      CONFIGURE_FILE("${CMake_SOURCE_DIR}/Tests/CTestUpdateGIT.cmake.in"
        "${CMake_BINARY_DIR}/Tests/CTestUpdateGIT.cmake" @ONLY)
      ADD_TEST(CTest.UpdateGIT ${CMAKE_CMAKE_COMMAND}
        -P "${CMake_BINARY_DIR}/Tests/CTestUpdateGIT.cmake"
        )
      LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${CTestUpdateGIT_DIR}")
    ENDIF(CTEST_TEST_UPDATE_GIT)

    # Test CTest Update with HG
    FIND_PROGRAM(HG_EXECUTABLE NAMES hg)
    MARK_AS_ADVANCED(HG_EXECUTABLE)
    SET(CTEST_TEST_UPDATE_HG 0)
    IF(HG_EXECUTABLE)
      IF(NOT "${HG_EXECUTABLE}" MATCHES "cygwin" OR UNIX)
        SET(CTEST_TEST_UPDATE_HG 1)
      ENDIF(NOT "${HG_EXECUTABLE}" MATCHES "cygwin" OR UNIX)
    ENDIF(HG_EXECUTABLE)
    IF(CTEST_TEST_UPDATE_HG)
      SET(CTestUpdateHG_DIR "CTest UpdateHG")
      CONFIGURE_FILE("${CMake_SOURCE_DIR}/Tests/CTestUpdateHG.cmake.in"
        "${CMake_BINARY_DIR}/Tests/CTestUpdateHG.cmake" @ONLY)
      ADD_TEST(CTest.UpdateHG ${CMAKE_CMAKE_COMMAND}
        -P "${CMake_BINARY_DIR}/Tests/CTestUpdateHG.cmake"
        )
      LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${CTestUpdateHG_DIR}")
    ENDIF(CTEST_TEST_UPDATE_HG)
  ENDIF(CTEST_TEST_UPDATE)

  CONFIGURE_FILE(
    "${CMake_SOURCE_DIR}/Tests/CTestTestFailure/testNoBuild.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestFailure/testNoBuild.cmake"
    @ONLY ESCAPE_QUOTES)
  ADD_TEST(CTestTestNoBuild ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestFailure/testNoBuild.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestFailure/testOut1.log"
    )
  SET_TESTS_PROPERTIES(CTestTestNoBuild PROPERTIES
    FAIL_REGULAR_EXPRESSION "Error" WILL_FAIL true)

  CONFIGURE_FILE(
    "${CMake_SOURCE_DIR}/Tests/CTestTestFailure/testNoExe.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestFailure/testNoExe.cmake"
    @ONLY ESCAPE_QUOTES)
  ADD_TEST(CTestTestNoExe ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestFailure/testNoExe.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestFailure/testOut2.log"
    )
  SET_TESTS_PROPERTIES(CTestTestNoExe PROPERTIES DEPENDS CTestTestNoBuild
    PASS_REGULAR_EXPRESSION "Could not find executable"
    FAIL_REGULAR_EXPRESSION "SegFault")


  # Use macro, not function so that build can still be driven by CMake 2.4.
  # After 2.6 is required, this could be a function without the extra 'set'
  # calls.
  #
  macro(add_config_tests cfg)
    set(cfg "${cfg}")
    set(base "${CMake_BINARY_DIR}/Tests/CTestConfig")

    # Test -S script with a -C config arg to ctest:
    configure_file(
      "${CMake_SOURCE_DIR}/Tests/CTestConfig/script.cmake.in"
      "${base}/${cfg}-script.cmake"
      @ONLY ESCAPE_QUOTES)
    add_test(CTestConfig.Script.${cfg} ${CMAKE_CTEST_COMMAND}
      -C ${cfg}
      -S "${base}/${cfg}-script.cmake" -VV
      --output-log "${base}/${cfg}-script.log"
      )

    # Test -D dashboard with a -C config arg to ctest.
    # (Actual commands inside a cmake -P script because we need to be able to set
    #  the working directory reliably...)
    configure_file(
      "${CMake_SOURCE_DIR}/Tests/CTestConfig/dashboard.cmake.in"
      "${base}/${cfg}-dashboard.cmake"
      @ONLY ESCAPE_QUOTES)
    add_test(CTestConfig.Dashboard.${cfg} ${CMAKE_CMAKE_COMMAND}
      -P "${base}/${cfg}-dashboard.cmake" -VV
      )
  endmacro()

  add_config_tests(Debug)
  add_config_tests(MinSizeRel)
  add_config_tests(Release)
  add_config_tests(RelWithDebInfo)

  add_test(CMakeCommands.build_command ${CMAKE_CMAKE_COMMAND}
    -DCMake_SOURCE_DIR=${CMake_SOURCE_DIR}
    -Ddir=${CMake_BINARY_DIR}/Tests/CMakeCommands/build_command
    -Dgen=${CMAKE_TEST_GENERATOR}
    -P "${CMake_SOURCE_DIR}/Tests/CMakeCommands/build_command/RunCMake.cmake"
  )

  CONFIGURE_FILE(
    "${CMake_SOURCE_DIR}/Tests/CTestTestCrash/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestCrash/test.cmake"
    @ONLY ESCAPE_QUOTES)
  ADD_TEST(CTestTestCrash ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestCrash/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestCrash/testOutput.log"
    )
  # with watcom the SEGFAULT is not found, it just fails
  IF(CMAKE_TEST_GENERATOR MATCHES "Watcom WMake")
    SET_TESTS_PROPERTIES(CTestTestCrash PROPERTIES
      PASS_REGULAR_EXPRESSION "Failed")
  ELSE(CMAKE_TEST_GENERATOR MATCHES "Watcom WMake")
    SET_TESTS_PROPERTIES(CTestTestCrash PROPERTIES
      PASS_REGULAR_EXPRESSION "SegFault")
  ENDIF(CMAKE_TEST_GENERATOR MATCHES "Watcom WMake")
    
  CONFIGURE_FILE(
    "${CMake_SOURCE_DIR}/Tests/CTestTestBadExe/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestBadExe/test.cmake"
    @ONLY ESCAPE_QUOTES)
  ADD_TEST(CTestTestBadExe ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestBadExe/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestBadExe/testOutput.log"
    )
  SET(CTestTestBadExe_REGEX "BAD_COMMAND")
  # some cygwin can not be made to produce a BAD_COMMAND error
  # the best we can get from it is a failed test
  IF(CYGWIN)
    SET(CTestTestBadExe_REGEX "(\\*\\*\\*Failed)|BAD_COMMAND")
  ENDIF()
  SET_TESTS_PROPERTIES(CTestTestBadExe PROPERTIES
    PASS_REGULAR_EXPRESSION "${CTestTestBadExe_REGEX}")

  CONFIGURE_FILE(
    "${CMake_SOURCE_DIR}/Tests/CTestTestParallel/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestParallel/test.cmake"
    @ONLY ESCAPE_QUOTES)
  ADD_TEST(CTestTestParallel ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestParallel/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestParallel/testOutput.log"
    )

  CONFIGURE_FILE(
    "${CMake_SOURCE_DIR}/Tests/CTestTestResourceLock/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestResourceLock/test.cmake"
    @ONLY ESCAPE_QUOTES)
  ADD_TEST(CTestTestResourceLock ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestResourceLock/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestResourceLock/output.log"
    )

  CONFIGURE_FILE(
    "${CMake_SOURCE_DIR}/Tests/CTestTestScheduler/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestScheduler/test.cmake"
    @ONLY ESCAPE_QUOTES)
  ADD_TEST(CTestTestScheduler ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestScheduler/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestScheduler/testOutput.log"
    )
  SET_TESTS_PROPERTIES(CTestTestScheduler PROPERTIES
    PASS_REGULAR_EXPRESSION "Start 1.*Start 2.*Start 3.*Start 4.*Start 4.*Start 3.*Start 2.*Start 1")

  CONFIGURE_FILE(
    "${CMake_SOURCE_DIR}/Tests/CTestTestStopTime/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestStopTime/test.cmake"
    @ONLY ESCAPE_QUOTES)
  CONFIGURE_FILE(
    "${CMake_SOURCE_DIR}/Tests/CTestTestStopTime/GetDate.cmake"
    "${CMake_BINARY_DIR}/Tests/CTestTestStopTime/GetDate.cmake"
    COPYONLY)
  ADD_TEST(CTestTestStopTime ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestStopTime/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestStopTime/testOutput.log"
    )
  SET_TESTS_PROPERTIES(CTestTestStopTime PROPERTIES
    PASS_REGULAR_EXPRESSION "The stop time has been passed")

  CONFIGURE_FILE(
    "${CMake_SOURCE_DIR}/Tests/CTestTestSubdir/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestSubdir/test.cmake"
    @ONLY ESCAPE_QUOTES)
  ADD_TEST(CTestTestSubdir ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestSubdir/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestSubdir/testOutput.log"
    )
  #make sure all 3 subdirs were added
  SET_TESTS_PROPERTIES(CTestTestSubdir PROPERTIES
    PASS_REGULAR_EXPRESSION "0 tests failed out of 3")
    
  CONFIGURE_FILE(
    "${CMake_SOURCE_DIR}/Tests/CTestTestTimeout/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestTimeout/test.cmake"
    @ONLY ESCAPE_QUOTES)
  ADD_TEST(CTestTestTimeout ${CMAKE_CTEST_COMMAND}
    -C "\${CTestTest_CONFIG}"
    -S "${CMake_BINARY_DIR}/Tests/CTestTestTimeout/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestTimeout/testOutput.log"
    )
  SET_TESTS_PROPERTIES(CTestTestTimeout PROPERTIES
    PASS_REGULAR_EXPRESSION "TestTimeout *\\.+ *\\*\\*\\*Timeout.*CheckChild *\\.+ *Passed")
  
  CONFIGURE_FILE(
    "${CMake_SOURCE_DIR}/Tests/CTestTestRunScript/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestRunScript/test.cmake"
    @ONLY ESCAPE_QUOTES)
  CONFIGURE_FILE(
    "${CMake_SOURCE_DIR}/Tests/CTestTestRunScript/hello.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestRunScript/hello.cmake"
    @ONLY ESCAPE_QUOTES)
  ADD_TEST(CTestTestRunScript ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestRunScript/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestRunScript/testOutput.log"
    )

  ADD_TEST(CTestTestShowOnly ${CMAKE_CTEST_COMMAND} -N)

  ADD_TEST(CTestBatchTest ${CMAKE_CTEST_COMMAND} -B)

  # Use macro, not function so that build can still be driven by CMake 2.4.
  # After 2.6 is required, this could be a function without the extra 'set'
  # calls.
  #
  macro(add_failed_submit_test name source build in out log regex)
    # Have variables named source, build and drop_method because the
    # configure_file call expects those variables to be defined.
    #
    set(source "${source}")
    set(build "${build}")
    configure_file("${in}" "${out}" @ONLY)
    add_test(${name} ${CMAKE_CTEST_COMMAND} -S "${out}" -V --output-log "${log}")
    set_tests_properties(${name} PROPERTIES PASS_REGULAR_EXPRESSION "${regex}")
  endmacro()

  set(regex "(Problems when submitting via S*CP")
  set(regex "${regex}|Error message was: ")
  set(regex "${regex}([Cc]ould *n.t resolve host")
  set(regex "${regex}|[Cc]ould *n.t connect to host")
  set(regex "${regex}|The requested URL returned error")
  set(regex "${regex}|libcurl was built with SSL disabled. https: not supported)")
  set(regex "${regex}|Submission method .xmlrpc. not compiled into CTest")
  set(regex "${regex}|Submission successful)")

  set(ctest_coverage_labels_args "")

  foreach(drop_method cp ftp http https scp xmlrpc)
    # Cycle through these values each time through the loop:
    if(ctest_coverage_labels_args STREQUAL "")
      set(ctest_coverage_labels_args "LABELS Everything")
    elseif(ctest_coverage_labels_args STREQUAL "LABELS Everything")
      set(ctest_coverage_labels_args "LABELS 0ArgTest")
    else()
      set(ctest_coverage_labels_args "")
    endif()

    add_failed_submit_test(CTestTestFailedSubmit-${drop_method}
      "${CMake_SOURCE_DIR}/Tests/CTestTest/SmallAndFast"
      "${CMake_BINARY_DIR}/Tests/CTestTestFailedSubmits/${drop_method}"
      "${CMake_SOURCE_DIR}/Tests/CTestTestFailedSubmits/test.cmake.in"
      "${CMake_BINARY_DIR}/Tests/CTestTestFailedSubmits/test-${drop_method}.cmake"
      "${CMake_BINARY_DIR}/Tests/CTestTestFailedSubmits/test-${drop_method}.log"
      "${regex}"
      )
  endforeach()


  IF (CMAKE_TESTS_CDASH_SERVER)
    SET(regex "^([^:]+)://([^/]+)(/.*)$")

    IF ("${CMAKE_TESTS_CDASH_SERVER}" MATCHES "${regex}")
      SET(protocol "${CMAKE_MATCH_1}")
      SET(server "${CMAKE_MATCH_2}")
      SET(path "${CMAKE_MATCH_3}")
    ELSE ("${CMAKE_TESTS_CDASH_SERVER}" MATCHES "${regex}")
      SET(protocol "http")
      SET(server "www.cdash.org")
      SET(path "/CDash")
      MESSAGE("warning: CMAKE_TESTS_CDASH_SERVER does not match expected regex...")
      MESSAGE("         ...using default url='${protocol}://${server}${path}' for CTestTest[23]")
    ENDIF ("${CMAKE_TESTS_CDASH_SERVER}" MATCHES "${regex}")
  ENDIF (CMAKE_TESTS_CDASH_SERVER)


  IF (CTEST_TEST_CTEST AND CMAKE_RUN_LONG_TESTS AND CMAKE_TESTS_CDASH_SERVER)
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

    # these tests take a long time, make sure they have it
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
  ENDIF (CTEST_TEST_CTEST AND CMAKE_RUN_LONG_TESTS AND CMAKE_TESTS_CDASH_SERVER)

  IF(NOT DEFINED CTEST_RUN_CTestSubmitLargeOutput)
    SET(CTEST_RUN_CTestSubmitLargeOutput OFF)

    IF (CMAKE_TESTS_CDASH_SERVER AND "${server}" STREQUAL "localhost")
      SET(CTEST_RUN_CTestSubmitLargeOutput ON)
    ENDIF (CMAKE_TESTS_CDASH_SERVER AND "${server}" STREQUAL "localhost")
  ENDIF(NOT DEFINED CTEST_RUN_CTestSubmitLargeOutput)

  IF(CTEST_RUN_CTestSubmitLargeOutput)
    CONFIGURE_FILE(
      "${CMake_SOURCE_DIR}/Tests/CTestSubmitLargeOutput/test.cmake.in"
      "${CMake_BINARY_DIR}/Tests/CTestSubmitLargeOutput/test.cmake"
      @ONLY ESCAPE_QUOTES
      )
    ADD_TEST(CTestSubmitLargeOutput ${CMAKE_CTEST_COMMAND}
      -S "${CMake_BINARY_DIR}/Tests/CTestSubmitLargeOutput/test.cmake" -V
      --output-log "${CMake_BINARY_DIR}/Tests/CTestSubmitLargeOutput/testOutput.log"
      )

    # This test only passes if it fails to submit properly...
    #
    SET_TESTS_PROPERTIES(CTestSubmitLargeOutput PROPERTIES
      PASS_REGULAR_EXPRESSION "Errors occurred during submission")

    # Give this test plenty of time to run on slower machines:
    #
    SET_TESTS_PROPERTIES(CTestSubmitLargeOutput PROPERTIES
      TIMEOUT ${CMAKE_LONG_TEST_TIMEOUT})
  ENDIF(CTEST_RUN_CTestSubmitLargeOutput)

  IF (CMAKE_RUN_LONG_TESTS AND TEST_KDE4_STABLE_BRANCH)
    IF(UNIX)
      IF(NOT QT4_FOUND)
         FIND_PACKAGE(Qt4)
      ENDIF(NOT QT4_FOUND)

      SET(TRY_BUILD_KDE4 TRUE)
      IF(QT4_FOUND)
        # check whether it's Qt 4.5 in a cmake 2.4. compatible way:
        IF(NOT EXISTS "${QT_QTNETWORK_INCLUDE_DIR}/QAbstractNetworkCache")
          SET(TRY_BUILD_KDE4 FALSE)
        ENDIF(NOT EXISTS "${QT_QTNETWORK_INCLUDE_DIR}/QAbstractNetworkCache")
      ELSE(QT4_FOUND)
        SET(TRY_BUILD_KDE4 FALSE)
      ENDIF(QT4_FOUND)

      FIND_PACKAGE(Perl)
      IF(NOT PERL_FOUND)
        SET(TRY_BUILD_KDE4 FALSE)
      ENDIF(NOT PERL_FOUND)

      FIND_PACKAGE(ZLIB)
      IF(NOT ZLIB_FOUND)
        SET(TRY_BUILD_KDE4 FALSE)
      ENDIF(NOT ZLIB_FOUND)

      IF(TRY_BUILD_KDE4)
        FILE(MAKE_DIRECTORY ${CMake_BINARY_DIR}/Tests/KDE4StableBranchTest)
        SET(TEST_KDE4_BASE_DIR ${CMake_BINARY_DIR}/Tests/KDE4StableBranchTest)
        CONFIGURE_FILE(${CMake_SOURCE_DIR}/Tests/KDE4StableBranchTest/test_kde4.sh.in ${CMake_BINARY_DIR}/Tests/KDE4StableBranchTest/test_kde4.sh @ONLY)
        EXECUTE_PROCESS(COMMAND chmod 755 ${CMake_BINARY_DIR}/Tests/KDE4StableBranchTest/test_kde4.sh )
        ADD_TEST(KDE4StableBranchTest ${CMake_BINARY_DIR}/Tests/KDE4StableBranchTest/test_kde4.sh)
      ENDIF(TRY_BUILD_KDE4)

    ENDIF(UNIX)
  ENDIF (CMAKE_RUN_LONG_TESTS AND TEST_KDE4_STABLE_BRANCH)
  
  IF("${CMAKE_TEST_GENERATOR}" MATCHES Xcode)
    SET(CMAKE_SKIP_BOOTSTRAP_TEST 1)
  ENDIF("${CMAKE_TEST_GENERATOR}" MATCHES Xcode)
  IF(EXISTS "${CMAKE_BINARY_DIR}/CMakeLists.txt")
    # If there is CMakeLists.txt in the binary tree, assume in-source build
    SET(CMAKE_SKIP_BOOTSTRAP_TEST 1)
  ENDIF(EXISTS "${CMAKE_BINARY_DIR}/CMakeLists.txt")
  SET(bootstrap "")
  IF(CMAKE_RUN_LONG_TESTS AND NOT CMAKE_SKIP_BOOTSTRAP_TEST)
    IF(UNIX)
      SET(bootstrap ${CMake_SOURCE_DIR}/bootstrap)
    ELSEIF(MSYS)
      CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/bootstrap.bat.in
                     ${CMAKE_CURRENT_BINARY_DIR}/bootstrap.bat @ONLY)
      SET(bootstrap ${CMAKE_CURRENT_BINARY_DIR}/bootstrap.bat)
    ENDIF()
  ENDIF()
  IF(bootstrap)
    ADD_TEST(BootstrapTest ${CMAKE_CTEST_COMMAND}
      --build-and-test
      ${CMake_SOURCE_DIR}
      ${CMake_BINARY_DIR}/Tests/BootstrapTest
      --build-nocmake
      --build-noclean
      --build-makeprogram ${bootstrap}
      --build-generator "${CMAKE_TEST_GENERATOR}"
      --test-command 
      ${CMake_BINARY_DIR}/Tests/BootstrapTest/Bootstrap.cmk/cmake)
    LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/BootstrapTest")
    # Make this test run early during parallel execution
    SET_TESTS_PROPERTIES(BootstrapTest PROPERTIES COST 5000)

    # provide more time for the bootstrap test
    GET_TEST_PROPERTY(BootstrapTest TIMEOUT PREVIOUS_TIMEOUT)
    IF ("${PREVIOUS_TIMEOUT}" MATCHES NOTFOUND)
      SET_TESTS_PROPERTIES ( BootstrapTest
        PROPERTIES TIMEOUT 5400)
    ENDIF ("${PREVIOUS_TIMEOUT}" MATCHES NOTFOUND)
  ENDIF()

  # fortran does not work for IDE builds because
  # CMAKE_STANDARD_LIBRARIES needs to be per language
  IF(CMAKE_TEST_GENERATOR MATCHES "Make|KDevelop")
    INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/CheckFortran.cmake)
    IF(CMAKE_Fortran_COMPILER)
      ADD_TEST(Fortran ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/Fortran"
        "${CMake_BINARY_DIR}/Tests/Fortran"
        --build-generator ${CMAKE_TEST_GENERATOR}
        --build-project testf
        --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
        --build-two-config
        --test-command testf)
      LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Fortran")

      # FortranCInterface tests.
      IF(UNIX)
        CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/FortranC/Flags.cmake.in
                       ${CMAKE_CURRENT_BINARY_DIR}/FortranC/Flags.cmake @ONLY)
        ADD_TEST(FortranC.Flags ${CMAKE_CMAKE_COMMAND} -P
          ${CMAKE_CURRENT_BINARY_DIR}/FortranC/Flags.cmake)
        LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/FortranC/Flags")
      ENDIF()
    ENDIF()
  ENDIF()

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
          LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Java")
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
      LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/SimpleCOnly_sdcc")
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
      LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Simple_Mingw_Linux2Win")
    ENDIF(MINGW_CC_LINUX2WIN_EXECUTABLE  AND  MINGW_CXX_LINUX2WIN_EXECUTABLE)
    
    
  ENDIF(CMAKE_TEST_GENERATOR MATCHES "Makefiles" OR CMAKE_TEST_GENERATOR MATCHES "KDevelop")

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

  IF(CMAKE_TEST_PROJECT_CSE_DIR)
    SET(script "${CMAKE_TEST_PROJECT_CSE_DIR}/BuildProjectCSE.cmake")
    IF(NOT EXISTS "${script}")
      SET(script "${CMAKE_TEST_PROJECT_CSE_DIR}/cse_build.cmake")
    ENDIF(NOT EXISTS "${script}")
    IF(NOT EXISTS "${script}")
      MESSAGE("warning: CMAKE_TEST_PROJECT_CSE_DIR set, but no build script found...")
    ENDIF(NOT EXISTS "${script}")

    ADD_TEST(BuildCSE ${CMAKE_CTEST_COMMAND} -V -S "${script}")
    SET_TESTS_PROPERTIES(BuildCSE PROPERTIES TIMEOUT 5400)
  ENDIF(CMAKE_TEST_PROJECT_CSE_DIR)

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

  # If this is not an in-source build, provide a target to wipe out
  # all the test build directories.
  IF(NOT EXISTS "${CMAKE_BINARY_DIR}/CMakeLists.txt")
    CONFIGURE_FILE(${CMake_SOURCE_DIR}/Tests/test_clean.cmake.in
                   ${CMake_BINARY_DIR}/Tests/test_clean.cmake @ONLY)
    ADD_CUSTOM_TARGET(test_clean
      COMMAND ${CMAKE_COMMAND} -P ${CMake_BINARY_DIR}/Tests/test_clean.cmake
      COMMENT "Removing test build directories."
      )
  ENDIF(NOT EXISTS "${CMAKE_BINARY_DIR}/CMakeLists.txt")

  ADD_TEST(CMakeWizardTest ${CMAKE_CMAKE_COMMAND}
    -D build_dir:STRING=${CMAKE_CURRENT_BINARY_DIR}/CMakeWizardTest
    -D source_dir:STRING=${CMAKE_CURRENT_SOURCE_DIR}/Tutorial/Step3
    -D CMAKE_CTEST_COMMAND:STRING=${CMAKE_CTEST_COMMAND}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/CMakeWizardTest.cmake)
ENDIF(BUILD_TESTING)

SUBDIRS(CMakeTests)
