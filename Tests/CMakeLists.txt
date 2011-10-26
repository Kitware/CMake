# a macro for tests that have a simple format where the name matches the
# directory and project
macro(ADD_TEST_MACRO NAME COMMAND)
  string(REPLACE "." "/" dir "${NAME}")
  string(REGEX REPLACE "[^.]*\\." "" proj "${NAME}")
  add_test(${NAME} ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/${dir}"
    "${CMake_BINARY_DIR}/Tests/${dir}"
    --build-two-config
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-project ${proj}
    ${${NAME}_EXTRA_OPTIONS}
    --test-command ${COMMAND} ${ARGN})
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${dir}")
endmacro(ADD_TEST_MACRO)

include(${CMAKE_CURRENT_SOURCE_DIR}/CheckFortran.cmake)

# Fake a user home directory to avoid polluting the real one.
if(DEFINED ENV{HOME} AND NOT CTEST_NO_TEST_HOME)
  set(TEST_HOME "${CMake_BINARY_DIR}/Tests/CMakeFiles/TestHome")
  file(MAKE_DIRECTORY "${TEST_HOME}")
  file(WRITE "${TEST_HOME}/.cvspass" ":pserver:anoncvs@www.cmake.org:/cvsroot/KWSys A\n")
  set(TEST_HOME_ENV_CODE "# Fake a user home directory to avoid polluting the real one.
# But provide original ENV{HOME} value in ENV{CTEST_REAL_HOME} for tests that
# need access to the real HOME directory.
set(ENV{CTEST_REAL_HOME} \"\$ENV{HOME}\")
set(ENV{HOME} \"${TEST_HOME}\")
")
endif()

# Choose a default configuration for CTest tests.
set(CTestTest_CONFIG Debug)
if(NOT CMAKE_CONFIGURATION_TYPES AND CMAKE_BUILD_TYPE)
  set(CTestTest_CONFIG ${CMAKE_BUILD_TYPE})
endif()

configure_file(${CMake_SOURCE_DIR}/Tests/EnforceConfig.cmake.in
               ${CMake_BINARY_DIR}/Tests/EnforceConfig.cmake @ONLY)

# Testing
if(BUILD_TESTING)
  if("${CMAKE_TEST_GENERATOR}" MATCHES "Unix Makefiles")
    set(TEST_CompileCommandOutput 1)
  endif()

  add_subdirectory(CMakeLib)

  add_subdirectory(FindPackageModeMakefileTest)

  # Collect a list of all test build directories.
  set(TEST_BUILD_DIRS)

  # Should the long tests be run?
  option(CMAKE_RUN_LONG_TESTS
    "Should the long tests be run (such as Bootstrap)." ON)
  mark_as_advanced(CMAKE_RUN_LONG_TESTS)

  if(CMAKE_RUN_LONG_TESTS)
    option(CTEST_TEST_CTEST
      "Should the tests that run a full sub ctest process be run?"
      OFF)
    mark_as_advanced(CTEST_TEST_CTEST)

    option(TEST_KDE4_STABLE_BRANCH
      "Should the KDE4 stable branch test be run?"
      OFF)
    mark_as_advanced(TEST_KDE4_STABLE_BRANCH)
  endif(CMAKE_RUN_LONG_TESTS)

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
  option(CTEST_TEST_CPACK
    "Should the tests that use '--build-target package' be run?"
    ON)
  mark_as_advanced(CTEST_TEST_CPACK)
  set(CTEST_TEST_OSX_ARCH 0)
  if(APPLE)
    execute_process(
      COMMAND sw_vers -productVersion
      OUTPUT_VARIABLE OSX_VERSION
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
    if(OSX_VERSION MATCHES "^10\\.[0123]" OR OSX_VERSION MATCHES "ProductVersion:\t10\\.[0123]")
      message(STATUS "Forcing CTEST_TEST_CPACK=OFF on OSX < 10.4")
      message(STATUS "OSX_VERSION='${OSX_VERSION}'")
      set(CTEST_TEST_CPACK OFF)
    else(OSX_VERSION MATCHES "^10\\.[0123]" OR OSX_VERSION MATCHES "ProductVersion:\t10\\.[0123]")
      set(CTEST_TEST_OSX_ARCH 1)
    endif(OSX_VERSION MATCHES "^10\\.[0123]" OR OSX_VERSION MATCHES "ProductVersion:\t10\\.[0123]")
  endif(APPLE)

  # Use 1500 or CTEST_TEST_TIMEOUT for long test timeout value,
  # whichever is greater.
  set(CMAKE_LONG_TEST_TIMEOUT 1500)
  if(CTEST_TEST_TIMEOUT)
    set(CMAKE_LONG_TEST_TIMEOUT ${CTEST_TEST_TIMEOUT})
  endif(CTEST_TEST_TIMEOUT)
  if(CMAKE_LONG_TEST_TIMEOUT LESS 1500)
    set(CMAKE_LONG_TEST_TIMEOUT 1500)
  endif(CMAKE_LONG_TEST_TIMEOUT LESS 1500)

  # add a bunch of standard build-and-test style tests
  ADD_TEST_MACRO(CommandLineTest CommandLineTest)
  ADD_TEST_MACRO(FindPackageTest FindPackageTest)
  ADD_TEST_MACRO(FindModulesExecuteAll FindModulesExecuteAll)
  ADD_TEST_MACRO(StringFileTest StringFileTest)
  ADD_TEST_MACRO(TryCompile TryCompile)
  ADD_TEST_MACRO(TarTest TarTest)
  ADD_TEST_MACRO(SystemInformation SystemInformation)
  ADD_TEST_MACRO(MathTest MathTest)
  # assume no resources building to test
  set(TEST_RESOURCES FALSE)
  # for windows and cygwin assume we have resources
  if(WIN32 OR CYGWIN)
    set(TEST_RESOURCES TRUE)
  endif()
  # for borland and watcom there is no resource support
  if("${CMAKE_TEST_GENERATOR}" MATCHES "WMake" OR
      "${CMAKE_TEST_GENERATOR}" MATCHES "Borland")
    set(TEST_RESOURCES FALSE)
  endif()
  if(TEST_RESOURCES)
    ADD_TEST_MACRO(VSResource VSResource)
  endif()
  ADD_TEST_MACRO(Simple Simple)
  ADD_TEST_MACRO(PreOrder PreOrder)
  ADD_TEST_MACRO(MissingSourceFile MissingSourceFile)
  set_tests_properties(MissingSourceFile PROPERTIES
    PASS_REGULAR_EXPRESSION "CMake Error at CMakeLists.txt:3 \\(add_executable\\):[ \r\n]*Cannot find source file:[ \r\n]*DoesNotExist/MissingSourceFile.c")
  if(CMAKE_Fortran_COMPILER)
    ADD_TEST_MACRO(FortranOnly FortranOnly)
  endif()
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
  ADD_TEST_MACRO(EmptyLibrary EmptyLibrary)
  set_tests_properties(EmptyLibrary PROPERTIES
    PASS_REGULAR_EXPRESSION "CMake Error: CMake can not determine linker language for target:test")
  ADD_TEST_MACRO(CrossCompile CrossCompile)
  set_tests_properties(CrossCompile PROPERTIES
    PASS_REGULAR_EXPRESSION "TRY_RUN.. invoked in cross-compiling mode")
  if("${CMAKE_TEST_GENERATOR}" MATCHES "Make")
    ADD_TEST_MACRO(Policy0002 Policy0002)
  endif("${CMAKE_TEST_GENERATOR}" MATCHES "Make")
  if(CTEST_TEST_OSX_ARCH)
    ADD_TEST_MACRO(Architecture Architecture)
    set_tests_properties(Architecture PROPERTIES
      PASS_REGULAR_EXPRESSION "(file is not of required architecture|does not match cputype|not the architecture being linked)")
  endif(CTEST_TEST_OSX_ARCH)

  list(APPEND TEST_BUILD_DIRS ${CMake_TEST_INSTALL_PREFIX})


  # run test for BundleUtilities on supported platforms/compilers
  if(MSVC OR
     CMAKE_SYSTEM_NAME MATCHES "Linux" OR
     CMAKE_SYSTEM_NAME MATCHES "Darwin")
  if(NOT "${CMAKE_TEST_GENERATOR}" STREQUAL "Watcom WMake")
    add_test(BundleUtilities ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/BundleUtilities"
      "${CMake_BINARY_DIR}/Tests/BundleUtilities"
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-project BundleUtilities
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/BundleUtilities")
  endif()
  endif()

  set(CMAKE_BUILD_TEST_SOURCE_DIR "${CMake_SOURCE_DIR}/Tests/COnly")
  set(CMAKE_BUILD_TEST_BINARY_DIR "${CMake_BINARY_DIR}/Tests/CMakeBuildCOnly")
  configure_file("${CMake_SOURCE_DIR}/Tests/CMakeBuildTest.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CMakeBuildTest.cmake" @ONLY)
  add_test(CMakeBuildTest ${CMAKE_CMAKE_COMMAND} -P
    "${CMake_BINARY_DIR}/Tests/CMakeBuildTest.cmake")
  list(APPEND TEST_BUILD_DIRS ${CMAKE_BUILD_TEST_BINARY_DIR})

  ADD_TEST_MACRO(Module.CheckTypeSize CheckTypeSize)

  ADD_TEST_MACRO(Module.GenerateExportHeader GenerateExportHeader)

  ADD_TEST_MACRO(Module.CheckCXXCompilerFlag CheckCXXCompilerFlag)

  add_test(LinkFlags-prepare
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
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/LinkFlags")

  macro(ADD_LINK_FLAGS_TEST name depends)
    add_test(LinkFlags-${name}
      ${CMAKE_CMAKE_COMMAND} --build "${CMake_BINARY_DIR}/Tests/LinkFlags"
      --target LinkFlags_${name} --config \${CTEST_CONFIGURATION_TYPE}
      )
    set_tests_properties(LinkFlags-${name} PROPERTIES
      PASS_REGULAR_EXPRESSION "BADFLAG" DEPENDS LinkFlags-${depends})
  endmacro()
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
  if(${CMAKE_TEST_GENERATOR} MATCHES "Unix Makefiles"  OR  ${CMAKE_TEST_GENERATOR} MATCHES "KDevelop")
    # check which generators we have
    exec_program(${CMAKE_CMAKE_COMMAND} ARGS --help OUTPUT_VARIABLE cmakeOutput )
    # check for the Eclipse generator
    if("${cmakeOutput}" MATCHES Eclipse)
      add_test(Simple_EclipseGenerator ${CMAKE_CTEST_COMMAND}
         --build-and-test
         "${CMake_SOURCE_DIR}/Tests/Simple"
         "${CMake_BINARY_DIR}/Tests/Simple_EclipseGenerator"
         --build-two-config
         --build-generator "Eclipse CDT4 - Unix Makefiles"
         --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
         --build-project Simple
         --test-command Simple)
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Simple_EclipseGenerator")
    endif("${cmakeOutput}" MATCHES Eclipse)

    # check for the CodeBlocks generator
    if("${cmakeOutput}" MATCHES CodeBlocks)
      add_test(Simple_CodeBlocksGenerator ${CMAKE_CTEST_COMMAND}
         --build-and-test
         "${CMake_SOURCE_DIR}/Tests/Simple"
         "${CMake_BINARY_DIR}/Tests/Simple_CodeBlocksGenerator"
         --build-two-config
         --build-generator "CodeBlocks - Unix Makefiles"
         --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
         --build-project Simple
         --test-command Simple)
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Simple_CodeBlocksGenerator")
    endif("${cmakeOutput}" MATCHES CodeBlocks)
    # check for the KDevelop3 generator
    if("${cmakeOutput}" MATCHES KDevelop3)
      add_test(Simple_KDevelop3Generator ${CMAKE_CTEST_COMMAND}
         --build-and-test
         "${CMake_SOURCE_DIR}/Tests/Simple"
         "${CMake_BINARY_DIR}/Tests/Simple_KDevelop3Generator"
         --build-two-config
         --build-generator "KDevelop3 - Unix Makefiles"
         --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
         --build-project Simple
         --test-command Simple)
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Simple_KDevelop3Generator")
    endif("${cmakeOutput}" MATCHES KDevelop3)

  endif(${CMAKE_TEST_GENERATOR} MATCHES "Unix Makefiles"  OR  ${CMAKE_TEST_GENERATOR} MATCHES "KDevelop")

  # test for correct sub-project generation
  # not implemented in VS6 or Xcode
  if(NOT MSVC60 AND NOT XCODE AND NOT MSVC70)
    # run cmake and configure all of SubProject
    # but only build the independent executable car
    add_test(SubProject ${CMAKE_CTEST_COMMAND}
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
    add_test(SubProject-Stage2  ${CMAKE_CTEST_COMMAND}
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
    set_tests_properties( SubProject-Stage2 PROPERTIES DEPENDS SubProject)
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/SubProject")
  endif(NOT MSVC60 AND NOT XCODE AND NOT MSVC70)

  if(CMAKE_STRICT)
    ADD_TEST_MACRO(DocTest DocTest)
  endif(CMAKE_STRICT)
  # macro to add a test that will build a nightly release
  # of CMake for given platform using the release scripts
  macro(ADD_NIGHTLY_BUILD_TEST name script)
    set(_TEST_DIR "${CMake_BINARY_DIR}/Tests/${name}")
    file(MAKE_DIRECTORY "${_TEST_DIR}")
    file(WRITE "${_TEST_DIR}/nightly-cmake.sh"
      "cd ${_TEST_DIR}
${CMake_BINARY_DIR}/bin/cmake -DCMAKE_CREATE_VERSION=nightly -P ${CMake_SOURCE_DIR}/Utilities/Release/${script}
${CMake_BINARY_DIR}/bin/cmake -DVERSION=master -P ${CMake_SOURCE_DIR}/Utilities/Release/upload_release.cmake
    ")
    add_test(${name} /bin/sh ${_TEST_DIR}/nightly-cmake.sh)
    if(COMMAND SET_TESTS_PROPERTIES AND COMMAND GET_TEST_PROPERTY)
      set_tests_properties(${name} PROPERTIES TIMEOUT ${CMAKE_LONG_TEST_TIMEOUT})
    endif(COMMAND SET_TESTS_PROPERTIES AND COMMAND GET_TEST_PROPERTY)
  endmacro(ADD_NIGHTLY_BUILD_TEST)
  if(CMAKE_BUILD_NIGHTLY_RELEASES)
    ADD_NIGHTLY_BUILD_TEST(CMakeNightlyWindows
      dash2win64_release.cmake)
    ADD_NIGHTLY_BUILD_TEST(CMakeNightlyMac
      dashmacmini2_release.cmake)
    ADD_NIGHTLY_BUILD_TEST(CMakeNightlyMac64
      dashmacmini5_release.cmake)
    ADD_NIGHTLY_BUILD_TEST(CMakeNightlyLinux
      magrathea_release.cmake)
  endif(CMAKE_BUILD_NIGHTLY_RELEASES)

  # add tests with more complex invocations
  add_test(Framework ${CMAKE_CTEST_COMMAND}
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
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Framework")

  add_test(TargetName ${CMAKE_CTEST_COMMAND}
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
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/TargetName")

  add_test(LibName ${CMAKE_CTEST_COMMAND}
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
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/LibName")

  add_test(CustComDepend ${CMAKE_CTEST_COMMAND}
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
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CustComDepend")

  add_test(ArgumentExpansion  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/ArgumentExpansion"
    "${CMake_BINARY_DIR}/Tests/ArgumentExpansion"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project ArgumentExpansion
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/ArgumentExpansion/bin"
    )
  set_tests_properties(ArgumentExpansion PROPERTIES
    FAIL_REGULAR_EXPRESSION "Unexpected: ")
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/ArgumentExpansion")

  add_test(CustomCommand  ${CMAKE_CTEST_COMMAND}
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
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CustomCommand")

  add_test(CustomCommandWorkingDirectory  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/CustomCommandWorkingDirectory"
    "${CMake_BINARY_DIR}/Tests/CustomCommandWorkingDirectory"
    --build-two-config
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project TestWorkingDir
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --test-command working
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CustomCommandWorkingDirectory")

   #add_test(SimpleExclude ${CMAKE_CTEST_COMMAND}
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
   #-P "${CMake_BINARY_DIR}/Tests/SimpleExclude/run.cmake"
   #)

#  add_test(SameName  ${CMAKE_CTEST_COMMAND}
#    --build-and-test
#    "${CMake_SOURCE_DIR}/Tests/SameName"
#    "${CMake_BINARY_DIR}/Tests/SameName"
#    --build-generator ${CMAKE_TEST_GENERATOR}
#    --build-project SameName
#    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
#    --build-two-config
#    --test-command
#    "${CMake_BINARY_DIR}/Tests/SameName/Exe1/mytest2")

  add_test(OutOfSource ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/OutOfSource"
    "${CMake_BINARY_DIR}/Tests/OutOfSource"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project OutOfSource
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-two-config
    --test-command
    "${CMake_BINARY_DIR}/Tests/OutOfSource/SubDir/OutOfSourceSubdir/simple")
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/OutOfSource")
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/OutOfSourceDeep")
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/OutOfBinary")

  add_test(BuildDepends ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/BuildDepends"
    "${CMake_BINARY_DIR}/Tests/BuildDepends"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project BuildDepends
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/BuildDepends")

  set(SimpleInstallInstallDir
    "${CMake_BINARY_DIR}/Tests/SimpleInstall/InstallDirectory")
  add_test(SimpleInstall ${CMAKE_CTEST_COMMAND}
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
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/SimpleInstall")
  add_test(SimpleInstall-Stage2 ${CMAKE_CTEST_COMMAND}
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
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/SimpleInstallS2")

  # By default, run the CPackComponents test if the CTEST_TEST_CPACK
  # option is ON:
  #
  set(CTEST_RUN_CPackComponents ${CTEST_TEST_CPACK})
  set(CTEST_package_X11_TEST ${CTEST_TEST_CPACK})
  set(CTEST_RUN_CPackComponentsForAll ${CTEST_TEST_CPACK})

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

  if(CTEST_RUN_CPackComponents)
    set(CPackComponents_EXTRA_OPTIONS)
    if(APPLE)
      set(CPackComponents_EXTRA_OPTIONS -DCPACK_BINARY_DRAGNDROP:BOOL=ON)
    endif(APPLE)
    if(NSIS_MAKENSIS_EXECUTABLE)
      set(CPackComponents_EXTRA_OPTIONS ${CPackComponents_EXTRA_OPTIONS}
        -DCPACK_BINARY_NSIS:BOOL=ON)
    endif(NSIS_MAKENSIS_EXECUTABLE)

    add_test(CPackComponents ${CMAKE_CTEST_COMMAND}
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
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CPackComponents")
  endif(CTEST_RUN_CPackComponents)

  if(CTEST_RUN_CPackComponentsForAll)
    set(CPackComponentsForAll_EXTRA_OPTIONS)
    set(CPackRun_CPackCommand "-DCPackCommand=${CMAKE_CPACK_COMMAND}")
    # set up list of CPack generators
    list(APPEND GENLST "ZIP")
    if(APPLE)
      list(APPEND GENLST "DragNDrop")
    endif(APPLE)
    if (CMAKE_SYSTEM_NAME MATCHES "Linux" AND NOT CMAKE_CURRENT_BINARY_DIR MATCHES ".* .*")
        find_program(RPMBUILD NAMES rpmbuild)
    endif(CMAKE_SYSTEM_NAME MATCHES "Linux" AND NOT CMAKE_CURRENT_BINARY_DIR MATCHES ".* .*")
    if (RPMBUILD)
       list(APPEND GENLST "RPM")
    endif(RPMBUILD)
    if (CMAKE_SYSTEM_NAME MATCHES "Linux")
       find_program(DPKG NAMES dpkg)
       if (DPKG)
          list(APPEND GENLST "DEB")
       endif(DPKG)
    endif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    # set up list of component packaging ways
    list(APPEND CWAYLST "default")
    list(APPEND CWAYLST "OnePackPerGroup")
    list(APPEND CWAYLST "IgnoreGroup")
    list(APPEND CWAYLST "AllInOne")
    foreach(CPackGen ${GENLST})
      set(CPackRun_CPackGen  "-DCPackGen=${CPackGen}")
      foreach(CPackComponentWay ${CWAYLST})
        set(CPackRun_CPackComponentWay "-DCPackComponentWay=${CPackComponentWay}")
        add_test(CPackComponentsForAll-${CPackGen}-${CPackComponentWay} ${CMAKE_CTEST_COMMAND}
          --build-and-test
          "${CMake_SOURCE_DIR}/Tests/CPackComponentsForAll"
          "${CMake_BINARY_DIR}/Tests/CPackComponentsForAll/build${CPackGen}-${CPackComponentWay}"
          --build-generator ${CMAKE_TEST_GENERATOR}
          --build-project CPackComponentsForAll
          --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
          --build-options
             -DCPACK_BINARY_${CPackGen}:BOOL=ON
             ${CPackRun_CPackComponentWay}
             ${CPackComponentsForAll_EXTRA_OPTIONS}
             --graphviz=CPackComponentsForAll.dot
          --test-command ${CMAKE_CMAKE_COMMAND}
            "-DCPackComponentsForAll_BINARY_DIR:PATH=${CMake_BINARY_DIR}/Tests/CPackComponentsForAll/build${CPackGen}-${CPackComponentWay}"
            "${CPackRun_CPackCommand}"
            "${CPackRun_CPackGen}"
            "${CPackRun_CPackComponentWay}"
            -P "${CMake_SOURCE_DIR}/Tests/CPackComponentsForAll/RunCPackVerifyResult.cmake")
        list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CPackComponentsForAll/build${CPackGen}-${CPackComponentWay}")
      endforeach(CPackComponentWay)
    endforeach(CPackGen)
  endif(CTEST_RUN_CPackComponentsForAll)

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

  if(CTEST_RUN_CPackTestAllGenerators)
    add_test(CPackTestAllGenerators ${CMAKE_CTEST_COMMAND}
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
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CPackTestAllGenerators")
  endif(CTEST_RUN_CPackTestAllGenerators)

  if(CTEST_package_X11_TEST)
    set(X11_build_target_arg --build-target package)
  else(CTEST_package_X11_TEST)
    set(X11_build_target_arg)
  endif(CTEST_package_X11_TEST)

  add_test(X11 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/X11"
    "${CMake_BINARY_DIR}/Tests/X11"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project UseX11
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-two-config
    ${X11_build_target_arg}
    --test-command  UseX11)
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/X11")

  if(NOT DEFINED CTEST_RUN_CMakeTestAllGenerators)
    set(CTEST_RUN_CMakeTestAllGenerators ON)
  endif(NOT DEFINED CTEST_RUN_CMakeTestAllGenerators)

  if(CTEST_RUN_CMakeTestAllGenerators)
    add_test(CMakeTestAllGenerators ${CMAKE_CMAKE_COMMAND}
        -D dir=${CMake_BINARY_DIR}/Tests/CMakeTestAllGenerators
        -D CMake_SOURCE_DIR=${CMake_SOURCE_DIR}
        -P ${CMake_SOURCE_DIR}/Tests/CMakeTestAllGenerators/RunCMake.cmake
      )
    list(APPEND TEST_BUILD_DIRS
      "${CMake_BINARY_DIR}/Tests/CMakeTestAllGenerators")
  endif(CTEST_RUN_CMakeTestAllGenerators)

  if(NOT DEFINED CTEST_RUN_CMakeTestBadCommandLines)
    set(CTEST_RUN_CMakeTestBadCommandLines ON)
  endif(NOT DEFINED CTEST_RUN_CMakeTestBadCommandLines)

  if(CTEST_RUN_CMakeTestBadCommandLines)
    add_test(CMakeTestBadCommandLines ${CMAKE_CMAKE_COMMAND}
        -D dir=${CMake_BINARY_DIR}/Tests/CMakeTestBadCommandLines
        -D gen=${CMAKE_TEST_GENERATOR}
        -D CMake_SOURCE_DIR=${CMake_SOURCE_DIR}
        -P ${CMake_SOURCE_DIR}/Tests/CMakeTestBadCommandLines/RunCMake.cmake
      )
    list(APPEND TEST_BUILD_DIRS
      "${CMake_BINARY_DIR}/Tests/CMakeTestBadCommandLines")
  endif(CTEST_RUN_CMakeTestBadCommandLines)

  if(NOT DEFINED CTEST_RUN_CMakeTestMultipleConfigures)
    set(CTEST_RUN_CMakeTestMultipleConfigures ON)
  endif(NOT DEFINED CTEST_RUN_CMakeTestMultipleConfigures)

  if(CTEST_RUN_CMakeTestMultipleConfigures)
    add_test(CMakeTestMultipleConfigures ${CMAKE_CMAKE_COMMAND}
        -D dir=${CMake_BINARY_DIR}/Tests/CMakeTestMultipleConfigures
        -D gen=${CMAKE_TEST_GENERATOR}
        -D CMake_SOURCE_DIR=${CMake_SOURCE_DIR}
        -P ${CMake_SOURCE_DIR}/Tests/CMakeTestMultipleConfigures/RunCMake.cmake
      )
    list(APPEND TEST_BUILD_DIRS
      "${CMake_BINARY_DIR}/Tests/CMakeTestMultipleConfigures")
  endif(CTEST_RUN_CMakeTestMultipleConfigures)

  add_test(LoadedCommandOneConfig  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/LoadCommandOneConfig"
    "${CMake_BINARY_DIR}/Tests/LoadCommandOneConfig"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project LoadCommand
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --test-command  LoadedCommand
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/LoadCommandOneConfig")

  # Como does not seem to support shared libraries.
  get_filename_component(CMAKE_BASE_NAME ${CMAKE_CXX_COMPILER} NAME_WE)
  if(CMAKE_BASE_NAME MATCHES "^como$")
    set(COMPILER_IS_COMO 1)
  endif(CMAKE_BASE_NAME MATCHES "^como$")
  if(NOT COMPILER_IS_COMO)
    set(COMPLEX_TEST_CMAKELIB 1)
    if(CMAKE_TEST_DIFFERENT_GENERATOR OR CMAKE_TEST_SYSTEM_LIBRARIES)
      set(COMPLEX_TEST_CMAKELIB 0)
    endif(CMAKE_TEST_DIFFERENT_GENERATOR OR CMAKE_TEST_SYSTEM_LIBRARIES)
    if(BORLAND)
      set(COMPLEX_TEST_CMAKELIB 0)
    endif(BORLAND)
    add_test(complex  ${CMAKE_CTEST_COMMAND}
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
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Complex")

    add_test(complexOneConfig  ${CMAKE_CTEST_COMMAND}
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
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/ComplexOneConfig")
    # because of the registry write these tests depend on each other
    set_tests_properties( complex PROPERTIES DEPENDS complexOneConfig)

# This fails on VS 70
# works on Xcode and makefiles
#      add_test(ConvLibrary ${CMAKE_CTEST_COMMAND}
#        --build-and-test
#        "${CMake_SOURCE_DIR}/Tests/ConvLibrary"
#        "${CMake_BINARY_DIR}/Tests/ConvLibrary"
#        --build-two-config
#        --build-generator ${CMAKE_TEST_GENERATOR}
#        --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
#        --build-project ConvLibrary
#        --test-command bartest)


#    add_test(complexRelativePaths  ${CMAKE_CTEST_COMMAND}
#      --build-and-test
#      "${CMake_SOURCE_DIR}/Tests/ComplexRelativePaths"
#      "${CMake_BINARY_DIR}/Tests/ComplexRelativePaths"
#      --build-generator ${CMAKE_TEST_GENERATOR}
#      --build-project complex
#      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
#      --build-exe-dir "${CMake_BINARY_DIR}/Tests/ComplexRelativePaths/bin"
#      --build-options -DCMAKE_USE_RELATIVE_PATHS:BOOL=ON
#      --test-command complex)

  endif(NOT COMPILER_IS_COMO)

  add_test(Example ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Example"
    "${CMake_BINARY_DIR}/Example"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project HELLO
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Example/Demo"
    --test-command helloDemo
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Example")

  add_test(Environment ${CMAKE_CTEST_COMMAND}
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
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Environment")

  if(NOT QT4_FOUND)
    find_package(Qt4)
  endif(NOT QT4_FOUND)

  if(QT4_FOUND)
    # test whether the Qt4 which has been found works, on some machines
    # which run nightly builds there were errors like "wrong file format"
    # for libQtCore.so. So first check it works, and only if it does add
    # the automoc test.
    include(CheckCXXSourceCompiles)
    set(_save_CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES}")
    set(_save_CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES}")

    set(CMAKE_REQUIRED_INCLUDES ${QT_INCLUDES})
    set(CMAKE_REQUIRED_LIBRARIES ${QT_QTCORE_LIBRARIES})

    CHECK_CXX_SOURCE_COMPILES("#include <QCoreApplication>\n int main() {return (qApp == 0 ? 0 : 1); }\n"
                              QT4_WORKS_FOR_AUTOMOC_TEST)

    set(CMAKE_REQUIRED_INCLUDES "${_save_CMAKE_REQUIRED_INCLUDES}")
    set(CMAKE_REQUIRED_LIBRARIES "${_save_CMAKE_REQUIRED_LIBRARIES}")

    if(QT4_WORKS_FOR_AUTOMOC_TEST)
      add_test(QtAutomoc ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/QtAutomoc"
        "${CMake_BINARY_DIR}/Tests/QtAutomoc"
        --build-generator ${CMAKE_TEST_GENERATOR}
        --build-project QtAutomoc
        --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
        --build-exe-dir "${CMake_BINARY_DIR}/Tests/QtAutomoc"
        --force-new-ctest-process
        --build-options -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
        --test-command ${CMAKE_CTEST_COMMAND} -V
        )
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/QtAutomoc")
    endif()
  endif()

  add_test(ExternalProject ${CMAKE_CTEST_COMMAND}
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
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/ExternalProject")
  set_tests_properties(ExternalProject PROPERTIES
    TIMEOUT ${CMAKE_LONG_TEST_TIMEOUT})

  # do each of the tutorial steps
  foreach(STP RANGE 1 7)
    add_test(TutorialStep${STP} ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/Tutorial/Step${STP}"
      "${CMake_BINARY_DIR}/Tests/Tutorial/Step${STP}"
      --build-two-config
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-project Tutorial
      --test-command Tutorial 25.0)
  endforeach(STP)
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Tutorial")

  add_test(testing ${CMAKE_CTEST_COMMAND} -C \${CTEST_CONFIGURATION_TYPE}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Testing"
    "${CMake_BINARY_DIR}/Tests/Testing"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project Testing
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --test-command ${CMAKE_CTEST_COMMAND} -C \${CTEST_CONFIGURATION_TYPE}
    )
  set_tests_properties(testing PROPERTIES PASS_REGULAR_EXPRESSION "Passed")
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Testing")

  add_test(wrapping  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Wrapping"
    "${CMake_BINARY_DIR}/Tests/Wrapping"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project Wrapping
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --test-command wrapping
    )
  add_test(qtwrapping  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Wrapping"
    "${CMake_BINARY_DIR}/Tests/Wrapping"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project Wrapping
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
      --test-command qtwrapping
      )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Wrapping")

  add_test(testdriver1 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TestDriver"
    "${CMake_BINARY_DIR}/Tests/TestDriver"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --build-project TestDriverTest
    --test-command TestDriverTest test1
    )

  add_test(testdriver2 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TestDriver"
    "${CMake_BINARY_DIR}/Tests/TestDriver"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --build-project TestDriverTest
    --test-command TestDriverTest test2
    )

  add_test(testdriver3  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TestDriver"
    "${CMake_BINARY_DIR}/Tests/TestDriver"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --build-project TestDriverTest
    --test-command TestDriverTest subdir/test3
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/TestDriver")

  add_test(Dependency ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Dependency"
    "${CMake_BINARY_DIR}/Tests/Dependency"
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Dependency/Exec"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-project Dependency
    --test-command exec
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Dependency")

  if("${CMAKE_SYSTEM_NAME}" MATCHES syllable)

# RPATH isn't supported under Syllable, so the tests don't
# find their libraries. In order to fix that LIBRARY_OUTPUT_DIR
# in the tests would have to be adjusted to ${EXECUTABLE_OUTPUT_DIR}/lib .
# For now we just require on Syllable that the user adjusts the DLL_PATH
# environment variable, so except the two tests below all other tests will succeed.

    set(_DLL_PATH "$ENV{DLL_PATH}")
    if(NOT "${_DLL_PATH}" MATCHES "^(.*:)?\\@bindir\\@/\\.(:.*)?$")
      message(FATAL_ERROR "In order to successfully run the CMake test suite on Syllable you need to add \"\\@bindir\\@/.\" to the DLL_PATH environment variable")
    endif(NOT "${_DLL_PATH}" MATCHES "^(.*:)?\\@bindir\\@/\\.(:.*)?$")
    if(NOT "${_DLL_PATH}" MATCHES "^(.*:)?\\@bindir\\@/\\.\\./lib(:.*)?$")
      message(FATAL_ERROR "In order to successfully run the CMake test suite on Syllable you need to add \"\\@bindir\\@/../lib\" to the DLL_PATH environment variable")
    endif(NOT "${_DLL_PATH}" MATCHES "^(.*:)?\\@bindir\\@/\\.\\./lib(:.*)?$")

  else("${CMAKE_SYSTEM_NAME}" MATCHES syllable)

    add_test(JumpWithLibOut  ${CMAKE_CTEST_COMMAND}
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

    add_test(JumpNoLibOut  ${CMAKE_CTEST_COMMAND}
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
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Jump")

    add_test(Plugin ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/Plugin"
      "${CMake_BINARY_DIR}/Tests/Plugin"
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-project Plugin
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-two-config
      --test-command bin/example)
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Plugin")

    if(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG)
      ADD_TEST_MACRO(RuntimePath RuntimePath)
    endif(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG)
  endif("${CMAKE_SYSTEM_NAME}" MATCHES syllable)

  add_test(linkorder1 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/LinkLineOrder"
    "${CMake_BINARY_DIR}/Tests/LinkLineOrder"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-project LinkLineOrder
    --test-command Exec1
    )

  add_test(linkorder2  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/LinkLineOrder"
    "${CMake_BINARY_DIR}/Tests/LinkLineOrder"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-project LinkLineOrder
    --test-command Exec2
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/LinkLineOrder")
  set_tests_properties( qtwrapping PROPERTIES DEPENDS wrapping)
  set_tests_properties( testdriver1 PROPERTIES DEPENDS qtwrapping)
  set_tests_properties( testdriver2 PROPERTIES DEPENDS testdriver1)
  set_tests_properties( testdriver3 PROPERTIES DEPENDS testdriver2)
  set_tests_properties( linkorder2 PROPERTIES DEPENDS linkorder1)
  set_tests_properties( SimpleInstall-Stage2 PROPERTIES DEPENDS SimpleInstall)

  # Test static linking on toolchains known to support it.
  if("${CMAKE_C_COMPILER_ID}" MATCHES "^(GNU)$"
      AND NOT APPLE AND NOT WIN32 AND NOT CYGWIN
      AND EXISTS "/usr/lib/libm.a")
    add_test(LinkStatic  ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/LinkStatic"
      "${CMake_BINARY_DIR}/Tests/LinkStatic"
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-project LinkStatic
      --build-options -DMATH_LIBRARY:FILEPATH=/usr/lib/libm.a
      --test-command LinkStatic
      )
  endif()

  if(NOT CMAKE_TEST_DIFFERENT_GENERATOR)
    add_test(kwsys ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Source/kwsys"
      "${CMake_BINARY_DIR}/Tests/kwsys"
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-project kwsys
      --test-command kwsysTestsCxx testIOS
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/kwsys")
  endif(NOT CMAKE_TEST_DIFFERENT_GENERATOR)
  set(MAKE_IS_GNU )
  if(${CMAKE_TEST_MAKEPROGRAM} MATCHES make)
    exec_program(
      ${CMAKE_TEST_MAKEPROGRAM} ARGS no_such_target --version
      RETURN_VALUE res OUTPUT_VARIABLE out
      )
    if("${res}" EQUAL 0)
      if("${out}" MATCHES "GNU")
        set(MAKE_IS_GNU 1)
      endif("${out}" MATCHES "GNU")
    endif("${res}" EQUAL 0)
  endif(${CMAKE_TEST_MAKEPROGRAM} MATCHES make)

  # only add this test on platforms that support it
  # some old versions of make simply cannot handle spaces in paths
  if(MAKE_IS_GNU OR
      "${CMAKE_TEST_MAKEPROGRAM}" MATCHES "nmake|gmake|wmake" OR
      "${CMAKE_TEST_GENERATOR}" MATCHES "Visual Studio|XCode|Borland")
    add_test(SubDirSpaces ${CMAKE_CTEST_COMMAND}
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
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/SubDirSpaces")
  endif(MAKE_IS_GNU OR
    "${CMAKE_TEST_MAKEPROGRAM}" MATCHES "nmake|gmake|wmake" OR
    "${CMAKE_TEST_GENERATOR}" MATCHES "Visual Studio|XCode|Borland")

  if(WIN32)
    add_test(SubDir ${CMAKE_CTEST_COMMAND}
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
  else(WIN32)
    add_test(SubDir ${CMAKE_CTEST_COMMAND}
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
  endif(WIN32)
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/SubDir")

  if(CMAKE_TEST_MSVC)
    ADD_TEST_MACRO(ForceInclude foo)
    ADD_TEST_MACRO(PrecompiledHeader foo)
  endif()
  if(CMAKE_TEST_MSVC OR
      "${CMAKE_TEST_GENERATOR}" MATCHES "(MSYS|MinGW) Makefiles")
    ADD_TEST_MACRO(ModuleDefinition example_exe)
  endif()

  ADD_TEST_MACRO(CheckCompilerRelatedVariables CheckCompilerRelatedVariables)

  if("${CMAKE_TEST_GENERATOR}" MATCHES "Makefile")
    add_test(MakeClean ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/MakeClean"
      "${CMake_BINARY_DIR}/Tests/MakeClean"
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-project MakeClean
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-exe-dir "${CMake_BINARY_DIR}/MakeClean"
      --test-command check_clean
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/MakeClean")
  endif("${CMAKE_TEST_GENERATOR}" MATCHES "Makefile")

  if(${CMAKE_TEST_GENERATOR} MATCHES "Visual Studio")
    add_test(VSExternalInclude ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/VSExternalInclude"
      "${CMake_BINARY_DIR}/Tests/VSExternalInclude"
      --build-two-config
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-project VSExternalInclude
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --test-command VSExternalInclude)
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/VSExternalInclude")

    add_test(VSMidl ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/VSMidl"
      "${CMake_BINARY_DIR}/Tests/VSMidl"
      --build-two-config
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-project VSMidl
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --test-command VSMidl)
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/VSMidl")
  endif(${CMAKE_TEST_GENERATOR} MATCHES "Visual Studio")

  if(APPLE AND CMAKE_COMPILER_IS_GNUCXX)
    set(BundleTestInstallDir
      "${CMake_BINARY_DIR}/Tests/BundleTest/InstallDirectory")
    add_test(BundleTest ${CMAKE_CTEST_COMMAND}
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
       "-DCMake_SOURCE_DIR:PATH=${CMake_SOURCE_DIR}"
      --test-command
      ${BundleTestInstallDir}/Applications/SecondBundleExe.app/Contents/MacOS/SecondBundleExe)
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/BundleTest")

    add_test(CFBundleTest ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/CFBundleTest"
      "${CMake_BINARY_DIR}/Tests/CFBundleTest"
      --build-two-config
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-project CFBundleTest
      --test-command
      ${CMAKE_CMAKE_COMMAND} -DCTEST_CONFIGURATION_TYPE=\${CTEST_CONFIGURATION_TYPE}
        -Ddir=${CMake_BINARY_DIR}/Tests/CFBundleTest
        -Dgen=${CMAKE_TEST_GENERATOR}
        -P ${CMake_SOURCE_DIR}/Tests/CFBundleTest/VerifyResult.cmake)
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CFBundleTest")

    ADD_TEST_MACRO(ObjC++ ObjC++)
  endif(APPLE AND CMAKE_COMPILER_IS_GNUCXX)

  if(APPLE AND CTEST_TEST_CPACK)
    add_test(BundleGeneratorTest ${CMAKE_CTEST_COMMAND}
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
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/BundleGeneratorTest")
  endif(APPLE AND CTEST_TEST_CPACK)

  add_test(WarnUnusedUnusedViaSet ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/VariableUnusedViaSet"
    "${CMake_BINARY_DIR}/Tests/WarnUnusedUnusedViaSet"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-noclean
    --build-project WarnUnusedUnusedViaSet
    --build-options "--warn-unused-vars")
  set_tests_properties(WarnUnusedUnusedViaSet PROPERTIES
    PASS_REGULAR_EXPRESSION "unused variable \\(changing definition\\) 'UNUSED_VARIABLE'")
  set_tests_properties(WarnUnusedUnusedViaSet PROPERTIES
    FAIL_REGULAR_EXPRESSION "unused variable \\(unsetting\\) 'UNUSED_VARIABLE'")
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/WarnUnusedUnusedViaSet")

  add_test(WarnUnusedUnusedViaUnset ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/VariableUnusedViaUnset"
    "${CMake_BINARY_DIR}/Tests/WarnUnusedUnusedViaUnset"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-noclean
    --build-project WarnUnusedUnusedViaUnset
    --build-options "--warn-unused-vars")
  set_tests_properties(WarnUnusedUnusedViaUnset PROPERTIES
    PASS_REGULAR_EXPRESSION "CMake Warning .*:7 \\(set\\):")
  set_tests_properties(WarnUnusedUnusedViaUnset PROPERTIES
    FAIL_REGULAR_EXPRESSION "CMake Warning .*:5 \\(set\\):")
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/WarnUnusedUnusedViaUnset")

  add_test(WarnUnusedCliUnused ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/VariableUsage"
    "${CMake_BINARY_DIR}/Tests/WarnUnusedCliUnused"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-noclean
    --build-project WarnUnusedCliUnused
    --build-options "-DUNUSED_CLI_VARIABLE=Unused")
  set_tests_properties(WarnUnusedCliUnused PROPERTIES
    PASS_REGULAR_EXPRESSION "CMake Warning:.*Manually-specified variables were not used by the project:.*  UNUSED_CLI_VARIABLE")
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/WarnUnusedCliUnused")

  add_test(WarnUnusedCliUsed ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/VariableUsage"
    "${CMake_BINARY_DIR}/Tests/WarnUnusedCliUsed"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-noclean
    --build-project WarnUnusedCliUsed
    --build-options "-DUSED_VARIABLE=Usage proven")
  set_tests_properties(WarnUnusedCliUsed PROPERTIES
    PASS_REGULAR_EXPRESSION "Usage proven")
  set_tests_properties(WarnUnusedCliUsed PROPERTIES
    FAIL_REGULAR_EXPRESSION "CMake Warning: The variable, 'USED_VARIABLE'")
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/WarnUnusedCliUsed")

  add_test(WarnUninitialized ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/VariableUsage"
    "${CMake_BINARY_DIR}/Tests/WarnUninitialized"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-noclean
    --build-project WarnUninitialized
    --build-options "--warn-uninitialized")
  set_tests_properties(WarnUninitialized PROPERTIES
    PASS_REGULAR_EXPRESSION "uninitialized variable 'USED_VARIABLE'")
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/WarnUninitialized")

  add_test(TestsWorkingDirectory ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TestsWorkingDirectory"
    "${CMake_BINARY_DIR}/Tests/TestsWorkingDirectory"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project TestsWorkingDirectoryProj
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/TestsWorkingDirectory"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V -C \${CTEST_CONFIGURATION_TYPE}
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/TestsWorkingDirectory")

  # Make sure CTest can handle a test with no newline in output.
  add_test(CTest.NoNewline
    ${CMAKE_CMAKE_COMMAND} -E echo_append "This line has no newline!")

  # A simple test for ctest in script mode
  configure_file("${CMake_SOURCE_DIR}/Tests/CTestScriptMode/CTestTestScriptMode.cmake.in"
          "${CMake_BINARY_DIR}/Tests/CTestScriptMode/CTestTestScriptMode.cmake" @ONLY)
#  add_test(CTest.ScriptMode ${CMAKE_CTEST_COMMAND}
#        -S "${CMake_BINARY_DIR}/Tests/CTestScriptMode/CTestTestScriptMode.cmake"
#        )

  set(CTEST_TEST_UPDATE 1)
  if(CTEST_TEST_UPDATE)
    # Test CTest Update with Subversion
    find_package(Subversion QUIET)
    if(Subversion_FOUND)
      get_filename_component(_Subversion_BIN_DIR
        ${Subversion_SVN_EXECUTABLE} PATH)
      find_program(Subversion_SVNADMIN_EXECUTABLE svnadmin
        HINTS ${_Subversion_BIN_DIR}
        )
      mark_as_advanced(Subversion_SVNADMIN_EXECUTABLE)
      if(NOT Subversion_SVNADMIN_EXECUTABLE)
        set(Subversion_FOUND FALSE)
      endif(NOT Subversion_SVNADMIN_EXECUTABLE)
    endif(Subversion_FOUND)
    if(Subversion_FOUND)
      set(CTestUpdateSVN_DIR "CTest UpdateSVN")
      configure_file("${CMake_SOURCE_DIR}/Tests/CTestUpdateSVN.cmake.in"
        "${CMake_BINARY_DIR}/Tests/CTestUpdateSVN.cmake" @ONLY)
      add_test(CTest.UpdateSVN ${CMAKE_CMAKE_COMMAND}
        -P "${CMake_BINARY_DIR}/Tests/CTestUpdateSVN.cmake"
        )
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${CTestUpdateSVN_DIR}")
    endif(Subversion_FOUND)

    # Test CTest Update with CVS
    if(EXISTS ${CMAKE_ROOT}/Modules/FindCVS.cmake)
      find_package(CVS QUIET)
    else(EXISTS ${CMAKE_ROOT}/Modules/FindCVS.cmake)
      find_program(CVS_EXECUTABLE NAMES cvs)
      set(CVS_FOUND ${CVS_EXECUTABLE})
    endif(EXISTS ${CMAKE_ROOT}/Modules/FindCVS.cmake)
    set(CTEST_TEST_UPDATE_CVS ${CVS_FOUND})
    if(CTEST_TEST_UPDATE_CVS AND NOT UNIX)
      if("${CVS_EXECUTABLE}" MATCHES "cygwin")
        message(STATUS "No CTest.UpdateCVS test with cygwin cvs.exe outside cygwin!")
        set(CTEST_TEST_UPDATE_CVS 0)
      endif("${CVS_EXECUTABLE}" MATCHES "cygwin")
    endif(CTEST_TEST_UPDATE_CVS AND NOT UNIX)
    if(CTEST_TEST_UPDATE_CVS)
      set(CTestUpdateCVS_DIR "CTest UpdateCVS")
      configure_file("${CMake_SOURCE_DIR}/Tests/CTestUpdateCVS.cmake.in"
        "${CMake_BINARY_DIR}/Tests/CTestUpdateCVS.cmake" @ONLY)
      add_test(CTest.UpdateCVS ${CMAKE_CMAKE_COMMAND}
        -P "${CMake_BINARY_DIR}/Tests/CTestUpdateCVS.cmake"
        )
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${CTestUpdateCVS_DIR}")
    endif(CTEST_TEST_UPDATE_CVS)

    # Test CTest Update with BZR
    find_program(BZR_EXECUTABLE NAMES bzr)
    mark_as_advanced(BZR_EXECUTABLE)
    set(CTEST_TEST_UPDATE_BZR 0)
    if(BZR_EXECUTABLE)
      if(NOT "${BZR_EXECUTABLE}" MATCHES "cygwin" OR UNIX)
        set(CTEST_TEST_UPDATE_BZR 1)
      endif(NOT "${BZR_EXECUTABLE}" MATCHES "cygwin" OR UNIX)
    endif(BZR_EXECUTABLE)
    if(CTEST_TEST_UPDATE_BZR)
      # Check if xmloutput plugin is there
      execute_process(COMMAND ${BZR_EXECUTABLE} xmlplugins RESULT_VARIABLE xmlplugres
        OUTPUT_QUIET ERROR_QUIET)
      if( NOT ${xmlplugres} )
        set(CTestUpdateBZR_DIR "CTest UpdateBZR")
        configure_file("${CMake_SOURCE_DIR}/Tests/CTestUpdateBZR.cmake.in"
          "${CMake_BINARY_DIR}/Tests/CTestUpdateBZR.cmake" @ONLY)
        add_test(CTest.UpdateBZR ${CMAKE_CMAKE_COMMAND}
          -P "${CMake_BINARY_DIR}/Tests/CTestUpdateBZR.cmake"
          )
        list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${CTestUpdateBZR_DIR}")
        set(CTestUpdateBZR_DIR "CTest UpdateBZR_CLocale")
        configure_file("${CMake_SOURCE_DIR}/Tests/CTestUpdateBZR.cmake.in"
          "${CMake_BINARY_DIR}/Tests/CTestUpdateBZR_CLocale.cmake" @ONLY)
        add_test(CTest.UpdateBZR.CLocale ${CMAKE_CMAKE_COMMAND}
          -P "${CMake_BINARY_DIR}/Tests/CTestUpdateBZR_CLocale.cmake"
          )
        set_tests_properties(CTest.UpdateBZR.CLocale PROPERTIES ENVIRONMENT LC_ALL=C)
        list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${CTestUpdateBZR_DIR}")
      endif( NOT ${xmlplugres} )
    endif(CTEST_TEST_UPDATE_BZR)

    # Test CTest Update with GIT
    find_program(GIT_EXECUTABLE NAMES git)
    mark_as_advanced(GIT_EXECUTABLE)
    set(CTEST_TEST_UPDATE_GIT 0)
    if(GIT_EXECUTABLE)
      if(NOT "${GIT_EXECUTABLE}" MATCHES "cygwin" OR UNIX)
        set(CTEST_TEST_UPDATE_GIT 1)
      endif(NOT "${GIT_EXECUTABLE}" MATCHES "cygwin" OR UNIX)
    endif(GIT_EXECUTABLE)
    if(CTEST_TEST_UPDATE_GIT)
      set(CTestUpdateGIT_DIR "CTest UpdateGIT")
      configure_file("${CMake_SOURCE_DIR}/Tests/CTestUpdateGIT.cmake.in"
        "${CMake_BINARY_DIR}/Tests/CTestUpdateGIT.cmake" @ONLY)
      add_test(CTest.UpdateGIT ${CMAKE_CMAKE_COMMAND}
        -P "${CMake_BINARY_DIR}/Tests/CTestUpdateGIT.cmake"
        )
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${CTestUpdateGIT_DIR}")
    endif(CTEST_TEST_UPDATE_GIT)

    # Test CTest Update with HG
    find_program(HG_EXECUTABLE NAMES hg)
    mark_as_advanced(HG_EXECUTABLE)
    set(CTEST_TEST_UPDATE_HG 0)
    if(HG_EXECUTABLE)
      if(NOT "${HG_EXECUTABLE}" MATCHES "cygwin" OR UNIX)
        set(CTEST_TEST_UPDATE_HG 1)
      endif(NOT "${HG_EXECUTABLE}" MATCHES "cygwin" OR UNIX)
    endif(HG_EXECUTABLE)
    if(CTEST_TEST_UPDATE_HG)
      set(CTestUpdateHG_DIR "CTest UpdateHG")
      configure_file("${CMake_SOURCE_DIR}/Tests/CTestUpdateHG.cmake.in"
        "${CMake_BINARY_DIR}/Tests/CTestUpdateHG.cmake" @ONLY)
      add_test(CTest.UpdateHG ${CMAKE_CMAKE_COMMAND}
        -P "${CMake_BINARY_DIR}/Tests/CTestUpdateHG.cmake"
        )
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${CTestUpdateHG_DIR}")
    endif(CTEST_TEST_UPDATE_HG)
  endif(CTEST_TEST_UPDATE)

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestFailure/testNoBuild.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestFailure/testNoBuild.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestNoBuild ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestFailure/testNoBuild.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestFailure/testOut1.log"
    )
  set_tests_properties(CTestTestNoBuild PROPERTIES
    FAIL_REGULAR_EXPRESSION "Error" WILL_FAIL true)

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestFailure/testNoExe.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestFailure/testNoExe.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestNoExe ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestFailure/testNoExe.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestFailure/testOut2.log"
    )
  set_tests_properties(CTestTestNoExe PROPERTIES DEPENDS CTestTestNoBuild
    PASS_REGULAR_EXPRESSION "Could not find executable"
    FAIL_REGULAR_EXPRESSION "SegFault")

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestUpload/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestUpload/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestUpload ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestUpload/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestUpload/testOut.log"
    )
  set_tests_properties(CTestTestUpload PROPERTIES
    PASS_REGULAR_EXPRESSION "Upload\\.xml")

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

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestCrash/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestCrash/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestCrash ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestCrash/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestCrash/testOutput.log"
    )
  # with watcom the SEGFAULT is not found, it just fails
  if(CMAKE_TEST_GENERATOR MATCHES "Watcom WMake")
    set_tests_properties(CTestTestCrash PROPERTIES
      PASS_REGULAR_EXPRESSION "Failed")
  else(CMAKE_TEST_GENERATOR MATCHES "Watcom WMake")
    set_tests_properties(CTestTestCrash PROPERTIES
      PASS_REGULAR_EXPRESSION "(Illegal|SegFault)")
  endif(CMAKE_TEST_GENERATOR MATCHES "Watcom WMake")

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestBadExe/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestBadExe/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestBadExe ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestBadExe/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestBadExe/testOutput.log"
    )
  set(CTestTestBadExe_REGEX "BAD_COMMAND")
  # some cygwin can not be made to produce a BAD_COMMAND error
  # the best we can get from it is a failed test
  if(CYGWIN)
    set(CTestTestBadExe_REGEX "(\\*\\*\\*Failed)|BAD_COMMAND")
  endif()
  set_tests_properties(CTestTestBadExe PROPERTIES
    PASS_REGULAR_EXPRESSION "${CTestTestBadExe_REGEX}")

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestParallel/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestParallel/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestParallel ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestParallel/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestParallel/testOutput.log"
    )

  add_test(CTestTestPrintLabels ${CMAKE_CTEST_COMMAND} --print-labels)
  set_tests_properties(CTestTestPrintLabels PROPERTIES LABELS "Label1;Label2")
  set_tests_properties(CTestTestPrintLabels PROPERTIES PASS_REGULAR_EXPRESSION
    "All Labels:.*  Label1.*  Label2")

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestResourceLock/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestResourceLock/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestResourceLock ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestResourceLock/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestResourceLock/output.log"
    )

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestScheduler/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestScheduler/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestScheduler ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestScheduler/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestScheduler/testOutput.log"
    )
  set_tests_properties(CTestTestScheduler PROPERTIES
    PASS_REGULAR_EXPRESSION "Start 1.*Start 2.*Start 3.*Start 4.*Start 4.*Start 3.*Start 2.*Start 1"
    RESOURCE_LOCK "CostData")

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestCostSerial/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestCostSerial/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestCostSerial ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestCostSerial/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestCostSerial/testOutput.log"
    )
  set_tests_properties(CTestTestCostSerial PROPERTIES
    PASS_REGULAR_EXPRESSION "Start 2.*Start 3.*Start 1.*Start 2.*Start 3.*Start 1"
    RESOURCE_LOCK "CostData")

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestStopTime/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestStopTime/test.cmake"
    @ONLY ESCAPE_QUOTES)
  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestStopTime/GetDate.cmake"
    "${CMake_BINARY_DIR}/Tests/CTestTestStopTime/GetDate.cmake"
    COPYONLY)
  add_test(CTestTestStopTime ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestStopTime/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestStopTime/testOutput.log"
    )
  set_tests_properties(CTestTestStopTime PROPERTIES
    PASS_REGULAR_EXPRESSION "The stop time has been passed")

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestSubdir/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestSubdir/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestSubdir ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestSubdir/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestSubdir/testOutput.log"
    )
  #make sure all 3 subdirs were added
  set_tests_properties(CTestTestSubdir PROPERTIES
    PASS_REGULAR_EXPRESSION "0 tests failed out of 3")

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestTimeout/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestTimeout/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestTimeout ${CMAKE_CTEST_COMMAND}
    -C "\${CTestTest_CONFIG}"
    -S "${CMake_BINARY_DIR}/Tests/CTestTestTimeout/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestTimeout/testOutput.log"
    )
  set_tests_properties(CTestTestTimeout PROPERTIES
    PASS_REGULAR_EXPRESSION "TestTimeout *\\.+ *\\*\\*\\*Timeout.*CheckChild *\\.+ *Passed")

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestZeroTimeout/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestZeroTimeout/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestZeroTimeout ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestZeroTimeout/test.cmake" -V
    --output-log
    "${CMake_BINARY_DIR}/Tests/CTestTestZeroTimeout/testOutput.log")
  set_tests_properties(CTestTestZeroTimeout PROPERTIES
    FAIL_REGULAR_EXPRESSION "\\*\\*\\*Timeout")

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestDepends/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestDepends/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestDepends ${CMAKE_CTEST_COMMAND}
    -C "\${CTestTest_CONFIG}"
    -S "${CMake_BINARY_DIR}/Tests/CTestTestDepends/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestDepends/testOutput.log"
    )

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestCycle/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestCycle/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestCycle ${CMAKE_CTEST_COMMAND}
    -C "\${CTestTest_CONFIG}"
    -S "${CMake_BINARY_DIR}/Tests/CTestTestCycle/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestCycle/testOutput.log"
    )
  set_tests_properties(CTestTestCycle PROPERTIES
    PASS_REGULAR_EXPRESSION "a cycle exists in the test dependency graph")

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestRunScript/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestRunScript/test.cmake"
    @ONLY ESCAPE_QUOTES)
  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestRunScript/hello.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestRunScript/hello.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestRunScript ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestRunScript/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestRunScript/testOutput.log"
    )

  add_test(CTestTestShowOnly ${CMAKE_CTEST_COMMAND} -N)

  add_test(CTestBatchTest ${CMAKE_CTEST_COMMAND} -B)

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
  set(regex "${regex}|Empty reply from server")
  set(regex "${regex}|The requested URL returned error")
  set(regex "${regex}|libcurl was built with SSL disabled. https: not supported)")
  set(regex "${regex}|Submission method .xmlrpc. not compiled into CTest")
  set(regex "${regex}|Submission problem")
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


  if(CMAKE_TESTS_CDASH_SERVER)
    set(regex "^([^:]+)://([^/]+)(/.*)$")

    if("${CMAKE_TESTS_CDASH_SERVER}" MATCHES "${regex}")
      set(protocol "${CMAKE_MATCH_1}")
      set(server "${CMAKE_MATCH_2}")
      set(path "${CMAKE_MATCH_3}")
    else("${CMAKE_TESTS_CDASH_SERVER}" MATCHES "${regex}")
      set(protocol "http")
      set(server "www.cdash.org")
      set(path "/CDash")
      message("warning: CMAKE_TESTS_CDASH_SERVER does not match expected regex...")
      message("         ...using default url='${protocol}://${server}${path}' for CTestTest[23]")
    endif("${CMAKE_TESTS_CDASH_SERVER}" MATCHES "${regex}")
  endif(CMAKE_TESTS_CDASH_SERVER)


  if(CTEST_TEST_CTEST AND CMAKE_RUN_LONG_TESTS AND CMAKE_TESTS_CDASH_SERVER)
    configure_file("${CMake_SOURCE_DIR}/Tests/CTestTest/test.cmake.in"
      "${CMake_BINARY_DIR}/Tests/CTestTest/test.cmake" @ONLY ESCAPE_QUOTES)
    add_test(CTestTest ${CMAKE_CTEST_COMMAND}
      -S "${CMake_BINARY_DIR}/Tests/CTestTest/test.cmake" -V
      --output-log "${CMake_BINARY_DIR}/Tests/CTestTest/testOutput.log"
      )

    configure_file("${CMake_SOURCE_DIR}/Tests/CTestTest2/test.cmake.in"
      "${CMake_BINARY_DIR}/Tests/CTestTest2/test.cmake" @ONLY ESCAPE_QUOTES)
    add_test(CTestTest2 ${CMAKE_CTEST_COMMAND}
      -S "${CMake_BINARY_DIR}/Tests/CTestTest2/test.cmake" -V
      --output-log "${CMake_BINARY_DIR}/Tests/CTestTest2/testOutput.log"
      )

    configure_file("${CMake_SOURCE_DIR}/Tests/CTestTestChecksum/test.cmake.in"
      "${CMake_BINARY_DIR}/Tests/CTestTestChecksum/test.cmake" @ONLY
      ESCAPE_QUOTES)
    add_test(CTestTestChecksum ${CMAKE_CTEST_COMMAND}
      -S "${CMake_BINARY_DIR}/Tests/CTestTestChecksum/test.cmake" -V
      --output-log
      "${CMake_BINARY_DIR}/Tests/CTestTestChecksum/testOutput.log"
      )
    set_tests_properties(CTestTestChecksum PROPERTIES PASS_REGULAR_EXPRESSION
      "Submission failed: Checksum failed for file")

    # these tests take a long time, make sure they have it
    # if timeouts have not already been set
    get_test_property(CTestTest TIMEOUT PREVIOUS_TIMEOUT)
    if("${PREVIOUS_TIMEOUT}" MATCHES NOTFOUND)
      set_tests_properties( CTestTest
        PROPERTIES TIMEOUT ${CMAKE_LONG_TEST_TIMEOUT})
    endif("${PREVIOUS_TIMEOUT}" MATCHES NOTFOUND)

    get_test_property(CTestTest2 TIMEOUT PREVIOUS_TIMEOUT)
    if("${PREVIOUS_TIMEOUT}" MATCHES NOTFOUND)
      set_tests_properties( CTestTest2
        PROPERTIES TIMEOUT ${CMAKE_LONG_TEST_TIMEOUT})
    endif("${PREVIOUS_TIMEOUT}" MATCHES NOTFOUND)
  endif(CTEST_TEST_CTEST AND CMAKE_RUN_LONG_TESTS AND CMAKE_TESTS_CDASH_SERVER)

  if(CMAKE_RUN_LONG_TESTS AND TEST_KDE4_STABLE_BRANCH)
    if(UNIX)
      if(NOT QT4_FOUND)
         find_package(Qt4)
      endif(NOT QT4_FOUND)

      set(TRY_BUILD_KDE4 TRUE)
      if(QT4_FOUND)
        # check whether it's Qt 4.5 in a cmake 2.4. compatible way:
        if(NOT EXISTS "${QT_QTNETWORK_INCLUDE_DIR}/QAbstractNetworkCache")
          set(TRY_BUILD_KDE4 FALSE)
        endif(NOT EXISTS "${QT_QTNETWORK_INCLUDE_DIR}/QAbstractNetworkCache")
      else(QT4_FOUND)
        set(TRY_BUILD_KDE4 FALSE)
      endif(QT4_FOUND)

      find_package(Perl)
      if(NOT PERL_FOUND)
        set(TRY_BUILD_KDE4 FALSE)
      endif(NOT PERL_FOUND)

      find_package(ZLIB)
      if(NOT ZLIB_FOUND)
        set(TRY_BUILD_KDE4 FALSE)
      endif(NOT ZLIB_FOUND)

      if(TRY_BUILD_KDE4)
        file(MAKE_DIRECTORY ${CMake_BINARY_DIR}/Tests/KDE4StableBranchTest)
        set(TEST_KDE4_BASE_DIR ${CMake_BINARY_DIR}/Tests/KDE4StableBranchTest)
        configure_file(${CMake_SOURCE_DIR}/Tests/KDE4StableBranchTest/test_kde4.sh.in ${CMake_BINARY_DIR}/Tests/KDE4StableBranchTest/test_kde4.sh @ONLY)
        execute_process(COMMAND chmod 755 ${CMake_BINARY_DIR}/Tests/KDE4StableBranchTest/test_kde4.sh )
        add_test(KDE4StableBranchTest ${CMake_BINARY_DIR}/Tests/KDE4StableBranchTest/test_kde4.sh)
      endif(TRY_BUILD_KDE4)

    endif(UNIX)
  endif(CMAKE_RUN_LONG_TESTS AND TEST_KDE4_STABLE_BRANCH)

  if("${CMAKE_TEST_GENERATOR}" MATCHES Xcode)
    set(CMAKE_SKIP_BOOTSTRAP_TEST 1)
  endif("${CMAKE_TEST_GENERATOR}" MATCHES Xcode)
  if(EXISTS "${CMake_BINARY_DIR}/CMakeLists.txt")
    # If there is CMakeLists.txt in the binary tree, assume in-source build
    set(CMAKE_SKIP_BOOTSTRAP_TEST 1)
  endif(EXISTS "${CMake_BINARY_DIR}/CMakeLists.txt")
  set(bootstrap "")
  if(CMAKE_RUN_LONG_TESTS AND NOT CMAKE_SKIP_BOOTSTRAP_TEST)
    if(UNIX)
      set(bootstrap ${CMake_SOURCE_DIR}/bootstrap)
    elseif(MSYS)
      configure_file(${CMAKE_CURRENT_SOURCE_DIR}/bootstrap.bat.in
                     ${CMAKE_CURRENT_BINARY_DIR}/bootstrap.bat @ONLY)
      set(bootstrap ${CMAKE_CURRENT_BINARY_DIR}/bootstrap.bat)
    endif()
  endif()
  if(bootstrap)
    add_test(BootstrapTest ${CMAKE_CTEST_COMMAND}
      --build-and-test
      ${CMake_SOURCE_DIR}
      ${CMake_BINARY_DIR}/Tests/BootstrapTest
      --build-nocmake
      --build-noclean
      --build-makeprogram ${bootstrap}
      --build-generator "${CMAKE_TEST_GENERATOR}"
      --test-command
      ${CMake_BINARY_DIR}/Tests/BootstrapTest/Bootstrap.cmk/cmake)
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/BootstrapTest")
    # Make this test run early during parallel execution
    set_tests_properties(BootstrapTest PROPERTIES COST 5000)

    # provide more time for the bootstrap test
    get_test_property(BootstrapTest TIMEOUT PREVIOUS_TIMEOUT)
    if("${PREVIOUS_TIMEOUT}" MATCHES NOTFOUND)
      set_tests_properties( BootstrapTest
        PROPERTIES TIMEOUT 5400)
    endif("${PREVIOUS_TIMEOUT}" MATCHES NOTFOUND)
  endif()

  if(CMAKE_Fortran_COMPILER)
    add_test(Fortran ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/Fortran"
      "${CMake_BINARY_DIR}/Tests/Fortran"
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-project testf
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-two-config
      --test-command testf)
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Fortran")

    # FortranCInterface tests.
    if(UNIX)
      configure_file(${CMAKE_CURRENT_SOURCE_DIR}/FortranC/Flags.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/FortranC/Flags.cmake @ONLY)
      add_test(FortranC.Flags ${CMAKE_CMAKE_COMMAND} -P
        ${CMAKE_CURRENT_BINARY_DIR}/FortranC/Flags.cmake)
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/FortranC/Flags")
    else()
      add_test(FortranC ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/FortranC"
        "${CMake_BINARY_DIR}/Tests/FortranC"
        --build-generator ${CMAKE_TEST_GENERATOR}
        --build-project FortranC
        --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
        --build-two-config
        --test-command CMakeFiles/FortranCInterface/FortranCInterface)
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/FortranC")
    endif()
  endif()

  find_package(Java COMPONENTS Development QUIET)
  if(Java_JAVA_EXECUTABLE AND Java_JAVAC_EXECUTABLE AND Java_JAR_EXECUTABLE AND NOT MINGW
      AND NOT "${CMAKE_TEST_GENERATOR}" MATCHES "Xcode")
    get_filename_component(JNIPATH ${JAVA_COMPILE} PATH)
    find_file(JNI_H jni.h
      "${JNIPATH}/../include"
      "${JNIPATH}/../java/include")
    if(JNI_H AND EXISTS "${JNI_H}") # in case jni.h is a broken symlink
      file(READ "${JNI_H}" JNI_FILE)
      if("${JNI_FILE}" MATCHES "JDK1_2")
        add_test(Java ${CMAKE_CTEST_COMMAND}
          --build-and-test
          "${CMake_SOURCE_DIR}/Tests/Java"
          "${CMake_BINARY_DIR}/Tests/Java"
          --build-generator ${CMAKE_TEST_GENERATOR}
          --build-project hello
          --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
          --build-two-config
          --build-run-dir "${CMake_BINARY_DIR}/Tests/Java/"
          --test-command ${JAVA_RUNTIME} -classpath hello.jar HelloWorld)
        list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Java")
      endif("${JNI_FILE}" MATCHES "JDK1_2")
    endif()
  endif()

  # add some cross compiler tests, for now only with makefile based generators
  if(CMAKE_TEST_GENERATOR MATCHES "Makefiles" OR CMAKE_TEST_GENERATOR MATCHES "KDevelop")

    # if sdcc is found, build the SimpleCOnly project with sdcc
    find_program(SDCC_EXECUTABLE sdcc)
    mark_as_advanced(SDCC_EXECUTABLE)
    if(SDCC_EXECUTABLE)
      add_test(SimpleCOnly_sdcc ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/SimpleCOnly"
        "${CMake_BINARY_DIR}/Tests/SimpleCOnly_sdcc"
        --build-generator ${CMAKE_TEST_GENERATOR}
        --build-project SimpleC
        --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
        --build-options
        "-DCMAKE_SYSTEM_NAME=Generic"
        "-DCMAKE_C_COMPILER=${SDCC_EXECUTABLE}")
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/SimpleCOnly_sdcc")
    endif(SDCC_EXECUTABLE)

    # If a Linux -> MinGW cross compiler is found then try it
    find_program(MINGW_CC_LINUX2WIN_EXECUTABLE i586-mingw32msvc-gcc)
    find_program(MINGW_CXX_LINUX2WIN_EXECUTABLE i586-mingw32msvc-g++)
    find_program(MINGW_RC_LINUX2WIN_EXECUTABLE i586-mingw32msvc-windres)
    mark_as_advanced(MINGW_CC_LINUX2WIN_EXECUTABLE MINGW_CXX_LINUX2WIN_EXECUTABLE MINGW_RC_LINUX2WIN_EXECUTABLE)
    if(MINGW_CC_LINUX2WIN_EXECUTABLE AND MINGW_CXX_LINUX2WIN_EXECUTABLE AND MINGW_RC_LINUX2WIN_EXECUTABLE)
      add_test(Simple_Mingw_Linux2Win ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/Simple"
        "${CMake_BINARY_DIR}/Tests/Simple_Mingw_Linux2Win"
        --build-generator ${CMAKE_TEST_GENERATOR}
        --build-project Simple
        --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
        --build-options
        "-DCMAKE_SYSTEM_NAME=Windows"
        "-DCMAKE_C_COMPILER=${MINGW_CC_LINUX2WIN_EXECUTABLE}"
        "-DCMAKE_CXX_COMPILER=${MINGW_CXX_LINUX2WIN_EXECUTABLE}"
        "-DCMAKE_RC_COMPILER=${MINGW_RC_LINUX2WIN_EXECUTABLE}"
        )
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Simple_Mingw_Linux2Win")
    endif()
  endif(CMAKE_TEST_GENERATOR MATCHES "Makefiles" OR CMAKE_TEST_GENERATOR MATCHES "KDevelop")

  if(UNIX)
    string(COMPARE EQUAL "${CMAKE_INSTALL_PREFIX}" "${CMake_BINARY_DIR}/Tests/TestShellInstall/Prefix"
      PREFIX_IS_FOR_TEST)
    if(PREFIX_IS_FOR_TEST)
      configure_file(
        ${CMake_SOURCE_DIR}/Tests/TestInstall.sh.in
        ${CMake_BINARY_DIR}/Tests/TestShellInstall/TestInstall.sh
        @ONLY IMMEDIATE
        )
      add_test(ShellInstall /bin/sh ${CMake_BINARY_DIR}/Tests/TestShellInstall/TestShellInstall.sh)
    endif(PREFIX_IS_FOR_TEST)
  endif(UNIX)

  if(CMAKE_TEST_PROJECT_CSE_DIR)
    set(script "${CMAKE_TEST_PROJECT_CSE_DIR}/BuildProjectCSE.cmake")
    if(NOT EXISTS "${script}")
      set(script "${CMAKE_TEST_PROJECT_CSE_DIR}/cse_build.cmake")
    endif(NOT EXISTS "${script}")
    if(NOT EXISTS "${script}")
      message("warning: CMAKE_TEST_PROJECT_CSE_DIR set, but no build script found...")
    endif(NOT EXISTS "${script}")

    add_test(BuildCSE ${CMAKE_CTEST_COMMAND} -V -S "${script}")
    set_tests_properties(BuildCSE PROPERTIES TIMEOUT 5400)
  endif(CMAKE_TEST_PROJECT_CSE_DIR)

  if(CMAKE_TEST_PLPLOT_DIR)
    add_test(plplot ${CMAKE_CTEST_COMMAND} -V -S ${CMAKE_TEST_PLPLOT_DIR}/../../EasyDashboardScripts/plplot.cmake )
    set_tests_properties( plplot PROPERTIES TIMEOUT 5400)
  endif(CMAKE_TEST_PLPLOT_DIR)

  if(CMAKE_TEST_CHICKEN_DIR)
    add_test(Chicken ${CMAKE_CTEST_COMMAND} -V -S ${CMAKE_TEST_CHICKEN_DIR}/../../EasyDashboardScripts/Chicken.cmake )
    set_tests_properties( Chicken PROPERTIES TIMEOUT 5400)
  endif(CMAKE_TEST_CHICKEN_DIR)

  if(CMAKE_TEST_KDELIBS_ALPHA_1_DIR)
    add_test(KDELibsAlpha1 ${CMAKE_CTEST_COMMAND} -V -S ${CMAKE_TEST_KDELIBS_ALPHA_1_DIR}/../../EasyDashboardScripts/kdelibs.cmake )
    set_tests_properties( KDELibsAlpha1 PROPERTIES TIMEOUT 5400)
  endif(CMAKE_TEST_KDELIBS_ALPHA_1_DIR)

  # If this is not an in-source build, provide a target to wipe out
  # all the test build directories.
  if(NOT EXISTS "${CMake_BINARY_DIR}/CMakeLists.txt")
    configure_file(${CMake_SOURCE_DIR}/Tests/test_clean.cmake.in
                   ${CMake_BINARY_DIR}/Tests/test_clean.cmake @ONLY)
    add_custom_target(test_clean
      COMMAND ${CMAKE_COMMAND} -P ${CMake_BINARY_DIR}/Tests/test_clean.cmake
      COMMENT "Removing test build directories."
      )
  endif(NOT EXISTS "${CMake_BINARY_DIR}/CMakeLists.txt")

  add_test(CMakeWizardTest ${CMAKE_CMAKE_COMMAND}
    -D build_dir:STRING=${CMAKE_CURRENT_BINARY_DIR}/CMakeWizardTest
    -D source_dir:STRING=${CMAKE_CURRENT_SOURCE_DIR}/Tutorial/Step3
    -D CMAKE_CTEST_COMMAND:STRING=${CMAKE_CTEST_COMMAND}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/CMakeWizardTest.cmake)
  # If the cache variable CMAKE_CONTRACT_PROJECTS is set
  # then the dashboard will run a contract with CMake test of that
  # name.  For example CMAKE_CONTRACT_PROJECTS = vtk542 would run
  # the vtk542 contract test.
  # For each Contract test, the project should provide a directory
  # with at least one CMakeLists.txt file that uses ExternalProject
  # to download and configure the project. The directory should also
  # contain a RunTest.cmake file that has a single set of the format:
  # set(project_RUN_TEST testToRun)
  # The testToRun should be a test executable that can be run to
  # smoke test the build.
  foreach(project ${CMAKE_CONTRACT_PROJECTS})
    include(Contracts/${project}/RunTest.cmake)
    ADD_TEST_MACRO(Contracts.${project}
      ${${project}_RUN_TEST})
    # Contract test timeout in seconds.
    # Default to 6 hours.
    if(DEFINED ${project}_TEST_TIMEOUT)
      set(timeout ${${project}_TEST_TIMEOUT})
    elseif(CMAKE_CONTRACT_TEST_TIMEOUT_DEFAULT)
      set(timeout ${CMAKE_CONTRACT_TEST_TIMEOUT_DEFAULT})
    else()
      set(timeout 21600)
    endif()
    set_tests_properties(Contracts.${project} PROPERTIES TIMEOUT ${timeout})
  endforeach()

  if(TEST_CompileCommandOutput)
    set(CompileCommandOutput_EXTRA_OPTIONS
      --build-options -DMAKE_SUPPORTS_SPACES=${MAKE_IS_GNU})
    ADD_TEST_MACRO(CompileCommandOutput
      "${CMake_BINARY_DIR}/Tests/CMakeLib/runcompilecommands")
  endif()

  add_test(IncludeDirectories ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/IncludeDirectories"
    "${CMake_BINARY_DIR}/Tests/IncludeDirectories"
    --build-two-config
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project IncludeDirectories
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --test-command IncludeDirectories)
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/IncludeDirectories")

  if(CMAKE_USE_KWSTYLE AND KWSTYLE_EXECUTABLE)
    # The "make StyleCheck" command line as a test. If the test fails, look
    # for lines like "Error #0 (624) Line length exceed 88 (max=79)" in the
    # output to find where the style errors are...
    add_test(KWStyle ${KWSTYLE_EXECUTABLE}
      -xml ${CMake_BINARY_DIR}/CMake.kws.xml
      -o ${CMake_SOURCE_DIR}/Utilities/KWStyle/CMakeOverwrite.txt
      -v
      -D ${CMake_BINARY_DIR}/CMakeKWSFiles.txt
      )
    set_tests_properties(KWStyle PROPERTIES
      WORKING_DIRECTORY ${CMake_BINARY_DIR}/Utilities/KWStyle)
  endif()
endif(BUILD_TESTING)

subdirs(CMakeTests)
