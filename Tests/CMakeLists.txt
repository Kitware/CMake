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
    ${build_generator_args}
    --build-project ${proj}
    ${${NAME}_EXTRA_OPTIONS}
    --test-command ${COMMAND} ${ARGN})
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${dir}")
endmacro()

macro(REGEX_ESCAPE_STRING _OUT _IN)
    # Escape special regex metacharacters with a backslash
    string(REGEX REPLACE "([$^.[|*+?()]|])" "\\\\\\1" ${_OUT} "${_IN}")
endmacro()

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
  if("${CMAKE_TEST_GENERATOR}" MATCHES "Unix Makefiles" OR ("${CMAKE_TEST_GENERATOR}" MATCHES Ninja AND NOT WIN32))
    set(TEST_CompileCommandOutput 1)
  endif()

  set(MAKE_IS_GNU )
  if(${CMAKE_TEST_MAKEPROGRAM} MATCHES make)
    execute_process(COMMAND ${CMAKE_TEST_MAKEPROGRAM} no_such_target --version
      RESULT_VARIABLE res OUTPUT_VARIABLE out ERROR_VARIABLE out)
    if("${res}" STREQUAL "0")
      if("${out}" MATCHES "GNU")
        set(MAKE_IS_GNU 1)
      endif()
    endif()
  endif()

  # some old versions of make simply cannot handle spaces in paths
  if (MAKE_IS_GNU OR
      "${CMAKE_TEST_MAKEPROGRAM}" MATCHES "nmake|gmake|wmake" OR
      "${CMAKE_TEST_GENERATOR}" MATCHES "Visual Studio|XCode|Borland")
    set(MAKE_SUPPORTS_SPACES 1)
  else()
    set(MAKE_SUPPORTS_SPACES 0)
  endif()

  set(build_generator_args
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    )
  if(CMAKE_TEST_GENERATOR_TOOLSET)
    list(APPEND build_generator_args
      --build-generator-toolset ${CMAKE_TEST_GENERATOR_TOOLSET}
      )
  endif()

  add_subdirectory(CMakeLib)
  add_subdirectory(CMakeOnly)
  add_subdirectory(RunCMake)

  add_subdirectory(FindPackageModeMakefileTest)

  add_subdirectory(CTestTestMemcheck)

  # Collect a list of all test build directories.
  set(TEST_BUILD_DIRS)

  # Should the long tests be run?
  option(CMAKE_RUN_LONG_TESTS
    "Should the long tests be run (such as Bootstrap)." ON)
  mark_as_advanced(CMAKE_RUN_LONG_TESTS)

  if (CMAKE_RUN_LONG_TESTS)
    option(CTEST_TEST_CTEST
      "Should the tests that run a full sub ctest process be run?"
      OFF)
    mark_as_advanced(CTEST_TEST_CTEST)
  endif ()

  # Should tests that use CVS be run?
  #
  set(do_cvs_tests 0)

  if(EXISTS ${CMAKE_ROOT}/Modules/FindCVS.cmake)
    find_package(CVS QUIET)
  else()
    find_program(CVS_EXECUTABLE NAMES cvs)
  endif()

  if(CVS_EXECUTABLE)
    set(do_cvs_tests 1)
  endif()

  if(do_cvs_tests AND NOT UNIX)
    if("${CVS_EXECUTABLE}" MATCHES "cygwin")
      set(do_cvs_tests 0)
    endif()
  endif()

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
    else()
      set(CTEST_TEST_OSX_ARCH 1)
    endif()
  endif()

  # Use 1500 or CTEST_TEST_TIMEOUT for long test timeout value,
  # whichever is greater.
  set(CMAKE_LONG_TEST_TIMEOUT 1500)
  if(CTEST_TEST_TIMEOUT)
    set(CMAKE_LONG_TEST_TIMEOUT ${CTEST_TEST_TIMEOUT})
  endif()
  if(CMAKE_LONG_TEST_TIMEOUT LESS 1500)
    set(CMAKE_LONG_TEST_TIMEOUT 1500)
  endif()

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
  # test Visual Studio GNU Fortran mixing with cmake_add_fortran_subdirectory
  # run this project if we have a working fortran compiler or
  # the test is enabled with CMAKE_TEST_CMAKE_ADD_FORTRAN cache variable.
  # If you enable the test, CMake should find the MinGW fortran install,
  # or in some cases you might need to set the PATH so that cmake can find
  # the gfortran from mingw.
  if(CMAKE_Fortran_COMPILER OR CMAKE_TEST_CMAKE_ADD_FORTRAN)
    set(CMAKE_SKIP_VSGNUFortran FALSE)
    # disable test for apple builds using ifort if they are building
    # more than one architecture, as ifort does not support that.
    if(APPLE AND (CMAKE_Fortran_COMPILER MATCHES ifort))
      list(LENGTH CMAKE_OSX_ARCHITECTURES len)
      if("${len}" GREATER 1)
        message(STATUS "Skip VSGNUFortran for ifort dual cpu mac build")
        set(CMAKE_SKIP_VSGNUFortran TRUE)
      endif()
    endif()
    if((CMAKE_C_COMPILER MATCHES lsb)
        AND (CMAKE_Fortran_COMPILER MATCHES ifort))
      message(STATUS "Skip VSGNUFortran for ifort and lsb compilers")
      set(CMAKE_SKIP_VSGNUFortran TRUE)
    endif()
    if(NOT CMAKE_SKIP_VSGNUFortran)
      ADD_TEST_MACRO(VSGNUFortran ${CMAKE_COMMAND} -P runtest.cmake)
    endif()
  endif()
  ADD_TEST_MACRO(COnly COnly)
  ADD_TEST_MACRO(CxxOnly CxxOnly)
  ADD_TEST_MACRO(IPO COnly/COnly)
  ADD_TEST_MACRO(OutDir runtime/OutDir)
  ADD_TEST_MACRO(ObjectLibrary UseCshared)
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
  ADD_TEST_MACRO(CompileDefinitions CompileDefinitions)
  ADD_TEST_MACRO(CompatibleInterface CompatibleInterface)
  set_tests_properties(EmptyLibrary PROPERTIES
    PASS_REGULAR_EXPRESSION "CMake Error: CMake can not determine linker language for target:test")
  ADD_TEST_MACRO(CrossCompile CrossCompile)
  set_tests_properties(CrossCompile PROPERTIES
    PASS_REGULAR_EXPRESSION "TRY_RUN.. invoked in cross-compiling mode")
  if("${CMAKE_TEST_GENERATOR}" MATCHES "Make")
    ADD_TEST_MACRO(Policy0002 Policy0002)
  endif()
  if(CTEST_TEST_OSX_ARCH)
    ADD_TEST_MACRO(Architecture Architecture)
    set_tests_properties(Architecture PROPERTIES
      PASS_REGULAR_EXPRESSION "(file is not of required architecture|does not match cputype|not the architecture being linked)")
  endif()

  list(APPEND TEST_BUILD_DIRS ${CMake_TEST_INSTALL_PREFIX})

  if(NOT QT4_FOUND)
    find_package(Qt4)
  endif()

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
                              QT4_WORKS)

    set(CMAKE_REQUIRED_INCLUDES "${_save_CMAKE_REQUIRED_INCLUDES}")
    set(CMAKE_REQUIRED_LIBRARIES "${_save_CMAKE_REQUIRED_LIBRARIES}")
  endif()

  # run test for BundleUtilities on supported platforms/compilers
  if(MSVC OR
     MINGW OR
     CMAKE_SYSTEM_NAME MATCHES "Linux" OR
     CMAKE_SYSTEM_NAME MATCHES "Darwin")
  if(NOT "${CMAKE_TEST_GENERATOR}" STREQUAL "Watcom WMake")

    add_test(BundleUtilities ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/BundleUtilities"
      "${CMake_BINARY_DIR}/Tests/BundleUtilities"
      ${build_generator_args}
      --build-project BundleUtilities
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/BundleUtilities")

    # run test for DeployQt4 on supported platforms/compilers (which depends on BundleUtilities)
    # this test also depends on the existence of the standard qtiff plugin
    if(QT4_WORKS AND QT_QTSQL_FOUND)
      add_test(Qt4Deploy ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/Qt4Deploy"
        "${CMake_BINARY_DIR}/Tests/Qt4Deploy"
        ${build_generator_args}
        --build-project Qt4Deploy
        --build-options
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
        )
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Qt4Deploy")
    endif()

  endif()
  endif()

  set(CMAKE_BUILD_TEST_SOURCE_DIR "${CMake_SOURCE_DIR}/Tests/COnly")
  set(CMAKE_BUILD_TEST_BINARY_DIR "${CMake_BINARY_DIR}/Tests/CMakeBuildCOnly")
  set(CMAKE_BUILD_TEST_EXE COnly)
  configure_file("${CMake_SOURCE_DIR}/Tests/CMakeBuildTest.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CMakeBuildTest.cmake" @ONLY)
  add_test(CMakeBuildTest ${CMAKE_CMAKE_COMMAND} -P
    "${CMake_BINARY_DIR}/Tests/CMakeBuildTest.cmake")
  list(APPEND TEST_BUILD_DIRS ${CMAKE_BUILD_TEST_BINARY_DIR})
  # now do it again for a project that has two project commands
  set(CMAKE_BUILD_TEST_SOURCE_DIR "${CMake_SOURCE_DIR}/Tests/DoubleProject")
  set(CMAKE_BUILD_TEST_BINARY_DIR "${CMake_BINARY_DIR}/Tests/DoubleProject")
  set(CMAKE_BUILD_TEST_EXE just_silly)
  configure_file("${CMake_SOURCE_DIR}/Tests/CMakeBuildTest.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CMakeBuildDoubleProjectTest.cmake" @ONLY)
  add_test(CMakeDoubleProject ${CMAKE_CMAKE_COMMAND} -P
    "${CMake_BINARY_DIR}/Tests/CMakeBuildDoubleProjectTest.cmake")
  list(APPEND TEST_BUILD_DIRS ${CMAKE_BUILD_TEST_BINARY_DIR})

  ADD_TEST_MACRO(Module.CheckTypeSize CheckTypeSize)

  add_test(Module.ExternalData ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Module/ExternalData"
    "${CMake_BINARY_DIR}/Tests/Module/ExternalData"
    ${build_generator_args}
    --build-project ExternalDataTest
    --build-noclean
    --force-new-ctest-process
    --build-options -DMAKE_SUPPORTS_SPACES=${MAKE_SUPPORTS_SPACES}
    --test-command ${CMAKE_CTEST_COMMAND} -C \${CTEST_CONFIGURATION_TYPE} -V
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Module/ExternalData")

  ADD_TEST_MACRO(Module.GenerateExportHeader GenerateExportHeader)

  if (APPLE OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(-fPIE run_pic_test)
  else()
    if (CMAKE_CXX_COMPILER_ID MATCHES "PGI"
        OR CMAKE_CXX_COMPILER_ID MATCHES "PathScale"
        OR CMAKE_SYSTEM_NAME MATCHES "IRIX64"
        OR CMAKE_CXX_COMPILER_ID MATCHES "Intel")
      set(run_pic_test 0)
    else()
      set(run_pic_test 1)
    endif()
  endif()

  if (run_pic_test)
    ADD_TEST_MACRO(PositionIndependentTargets PositionIndependentTargets)
  endif()

  add_test(LinkFlags-prepare
    ${CMAKE_CTEST_COMMAND} -C \${CTEST_CONFIGURATION_TYPE}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/LinkFlags"
    "${CMake_BINARY_DIR}/Tests/LinkFlags"
    ${build_generator_args}
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
    execute_process(COMMAND ${CMAKE_CMAKE_COMMAND} --help
      OUTPUT_VARIABLE cmakeOutput ERROR_VARIABLE cmakeOutput)
    # check for the Eclipse generator
    if ("${cmakeOutput}" MATCHES Eclipse)
      add_test(Simple_EclipseGenerator ${CMAKE_CTEST_COMMAND}
         --build-and-test
         "${CMake_SOURCE_DIR}/Tests/Simple"
         "${CMake_BINARY_DIR}/Tests/Simple_EclipseGenerator"
         --build-two-config
         --build-generator "Eclipse CDT4 - Unix Makefiles"
         --build-generator-toolset "${CMAKE_TEST_GENERATOR_TOOLSET}"
         --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
         --build-project Simple
         --test-command Simple)
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Simple_EclipseGenerator")
    endif ()

    # check for the CodeBlocks generator
    if ("${cmakeOutput}" MATCHES CodeBlocks)
      add_test(Simple_CodeBlocksGenerator ${CMAKE_CTEST_COMMAND}
         --build-and-test
         "${CMake_SOURCE_DIR}/Tests/Simple"
         "${CMake_BINARY_DIR}/Tests/Simple_CodeBlocksGenerator"
         --build-two-config
         --build-generator "CodeBlocks - Unix Makefiles"
         --build-generator-toolset "${CMAKE_TEST_GENERATOR_TOOLSET}"
         --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
         --build-project Simple
         --test-command Simple)
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Simple_CodeBlocksGenerator")
    endif ()
    # check for the KDevelop3 generator
    if ("${cmakeOutput}" MATCHES KDevelop3)
      add_test(Simple_KDevelop3Generator ${CMAKE_CTEST_COMMAND}
         --build-and-test
         "${CMake_SOURCE_DIR}/Tests/Simple"
         "${CMake_BINARY_DIR}/Tests/Simple_KDevelop3Generator"
         --build-two-config
         --build-generator "KDevelop3 - Unix Makefiles"
         --build-generator-toolset "${CMAKE_TEST_GENERATOR_TOOLSET}"
         --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
         --build-project Simple
         --test-command Simple)
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Simple_KDevelop3Generator")
    endif ()

  endif()

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
      ${build_generator_args}
      --build-target car
      --test-command car
      )

    if(${CMAKE_TEST_GENERATOR} MATCHES "Ninja")
      # The Ninja generator does not create a recursive build system.  Start
      # from the root directory.
      set(SubProject_SUBDIR)
    else()
      set(SubProject_SUBDIR "/foo")
    endif()

    # For stage 2, do not run cmake again.
    # Then build the foo sub project which should build
    # the bar library which should be referenced because
    # foo links to the static library bar, but bar is not
    # directly in the foo sub project
    add_test(SubProject-Stage2  ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/SubProject${SubProject_SUBDIR}"
      "${CMake_BINARY_DIR}/Tests/SubProject${SubProject_SUBDIR}"
      ${build_generator_args}
      --build-nocmake
      --build-project foo
      --build-target foo
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/SubProject/foo"
      --test-command foo
      )
    set_tests_properties ( SubProject-Stage2 PROPERTIES DEPENDS SubProject)
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/SubProject")
  endif()

  if (CMAKE_STRICT)
    ADD_TEST_MACRO(DocTest DocTest)
  endif ()
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
      set_tests_properties (${name} PROPERTIES TIMEOUT ${CMAKE_LONG_TEST_TIMEOUT})
    endif()
  endmacro()
  if(CMAKE_BUILD_NIGHTLY_RELEASES)
    ADD_NIGHTLY_BUILD_TEST(CMakeNightlyWindows
      dash2win64_release.cmake)
    ADD_NIGHTLY_BUILD_TEST(CMakeNightlyMac
      dashmacmini2_release.cmake)
    ADD_NIGHTLY_BUILD_TEST(CMakeNightlyMac64
      dashmacmini5_release.cmake)
    ADD_NIGHTLY_BUILD_TEST(CMakeNightlyLinux
      magrathea_release.cmake)
  endif()

  # add tests with more complex invocations
  add_test(Framework ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Framework"
    "${CMake_BINARY_DIR}/Tests/Framework"
    --build-two-config
    ${build_generator_args}
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
    ${build_generator_args}
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
    ${build_generator_args}
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
    ${build_generator_args}
    --build-project CustComDepend
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/CustComDepend/bin"
    --test-command foo bar.c
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CustComDepend")

  add_test(ArgumentExpansion  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/ArgumentExpansion"
    "${CMake_BINARY_DIR}/Tests/ArgumentExpansion"
    ${build_generator_args}
    --build-project ArgumentExpansion
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/ArgumentExpansion/bin"
    )
  set_tests_properties(ArgumentExpansion PROPERTIES
    FAIL_REGULAR_EXPRESSION "Unexpected: ")
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/ArgumentExpansion")

  add_test(GeneratorExpression  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/GeneratorExpression"
    "${CMake_BINARY_DIR}/Tests/GeneratorExpression"
    ${build_generator_args}
    --build-project GeneratorExpression
    --build-options -DCMAKE_BUILD_TYPE=\${CTEST_CONFIGURATION_TYPE}
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/GeneratorExpression")

  add_test(CustomCommand  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/CustomCommand"
    "${CMake_BINARY_DIR}/Tests/CustomCommand"
    --build-two-config
    ${build_generator_args}
    --build-project CustomCommand
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/CustomCommand/bin"
    --test-command CustomCommand
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CustomCommand")

  ADD_TEST_MACRO(EmptyDepends ${CMAKE_CTEST_COMMAND})

  add_test(CustomCommandWorkingDirectory  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/CustomCommandWorkingDirectory"
    "${CMake_BINARY_DIR}/Tests/CustomCommandWorkingDirectory"
    --build-two-config
    ${build_generator_args}
    --build-project TestWorkingDir
    --test-command working
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CustomCommandWorkingDirectory")

   #add_test(SimpleExclude ${CMAKE_CTEST_COMMAND}
   #  --build-and-test
   #  "${CMake_SOURCE_DIR}/Tests/SimpleExclude"
   #  "${CMake_BINARY_DIR}/Tests/SimpleExclude"
   #  ${build_generator_args}
   #  --build-project SimpleExclude
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
#    ${build_generator_args}
#    --build-project SameName
#    --build-two-config
#    --test-command
#    "${CMake_BINARY_DIR}/Tests/SameName/Exe1/mytest2")

  add_test(OutOfSource ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/OutOfSource"
    "${CMake_BINARY_DIR}/Tests/OutOfSource"
    ${build_generator_args}
    --build-project OutOfSource
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
    ${build_generator_args}
    --build-project BuildDepends
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/BuildDepends")

  set(SimpleInstallInstallDir
    "${CMake_BINARY_DIR}/Tests/SimpleInstall/InstallDirectory")
  add_test(SimpleInstall ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/SimpleInstall"
    "${CMake_BINARY_DIR}/Tests/SimpleInstall"
    ${build_generator_args}
    --build-project TestSimpleInstall
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
    ${build_generator_args}
    --build-project TestSimpleInstall
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

  if (CMAKE_SYSTEM_NAME MATCHES "Linux" AND NOT CMAKE_CURRENT_BINARY_DIR MATCHES ".* .*")
    find_program(RPMBUILD NAMES rpmbuild)
  endif()
  # Do not try to build RPM
  if (NOT RPMBUILD)
    set(CPACK_BINARY_RPM OFF)
  endif()

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
    else()
      set(CTEST_RUN_CPackComponents OFF)
      set(CTEST_package_X11_TEST OFF)
    endif()
  endif()

  # On Windows run the CPackWiXGenerator test
  # if the WiX Toolset seems to be available
  if(WIN32)
    file(TO_CMAKE_PATH "$ENV{WIX}" WIX_ROOT)

    find_program(WIX_LIGHT_EXECUTABLE light
      PATHS "${WIX_ROOT}/bin"
      DOC "WiX Toolset light.exe location")

    if(WIX_LIGHT_EXECUTABLE)
      add_test(CPackWiXGenerator ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/CPackWiXGenerator"
        "${CMake_BINARY_DIR}/Tests/CPackWiXGenerator"
        ${build_generator_args}
        --build-project CPackWiXGenerator
        --test-command ${CMAKE_CMAKE_COMMAND}
          "-DCPackWiXGenerator_BINARY_DIR:PATH=${CMake_BINARY_DIR}/Tests/CPackWiXGenerator"
          -P "${CMake_SOURCE_DIR}/Tests/CPackWiXGenerator/RunCPackVerifyResult.cmake")
    endif()
  endif()

  if(CTEST_RUN_CPackComponents)
    set(CPackComponents_EXTRA_OPTIONS)
    if(APPLE)
      set(CPackComponents_EXTRA_OPTIONS -DCPACK_BINARY_DRAGNDROP:BOOL=ON)
    endif()
    if(NSIS_MAKENSIS_EXECUTABLE)
      set(CPackComponents_EXTRA_OPTIONS ${CPackComponents_EXTRA_OPTIONS}
        -DCPACK_BINARY_NSIS:BOOL=ON)
    endif()

    add_test(CPackComponents ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/CPackComponents"
      "${CMake_BINARY_DIR}/Tests/CPackComponents"
      ${build_generator_args}
      --build-project CPackComponents
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
  endif()

  if(CTEST_RUN_CPackComponentsForAll)
    # Check whether if rpmbuild command is found
    # before adding RPM tests
    find_program(RPMBUILD_EXECUTABLE NAMES rpmbuild)
    if(RPMBUILD_EXECUTABLE)
      list(APPEND ACTIVE_CPACK_GENERATORS RPM)
    endif()
    # Check whether if dpkg command is found
    # before adding DEB tests
    find_program(DPKG_EXECUTABLE NAMES dpkg)
    if(DPKG_EXECUTABLE)
      list(APPEND ACTIVE_CPACK_GENERATORS DEB)
    endif()

    # ACTIVE_CPACK_GENERATORS variable
    # now contains the list of 'active generators'
    set(CPackComponentsForAll_EXTRA_OPTIONS)
    # set up list of CPack generators
    list(APPEND GENLST "ZIP")
    if(APPLE)
      list(APPEND GENLST "DragNDrop")
    endif()
    if (NOT CMAKE_CURRENT_BINARY_DIR MATCHES ".* .*")
      list(FIND ACTIVE_CPACK_GENERATORS "RPM" RPM_ACTIVE)
      if (NOT ${RPM_ACTIVE} EQUAL -1)
        list(APPEND GENLST "RPM")
      endif()
    endif()
    list(FIND ACTIVE_CPACK_GENERATORS "DEB" DEB_ACTIVE)
    if (NOT ${DEB_ACTIVE} EQUAL -1)
      list(APPEND GENLST "DEB")
    endif()

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
          ${build_generator_args}
          --build-project CPackComponentsForAll
          --build-options
             -DCPACK_BINARY_${CPackGen}:BOOL=ON
             ${CPackRun_CPackComponentWay}
             ${CPackComponentsForAll_EXTRA_OPTIONS}
             --graphviz=CPackComponentsForAll.dot
          --test-command ${CMAKE_CMAKE_COMMAND}
            "-DCPackComponentsForAll_BINARY_DIR:PATH=${CMake_BINARY_DIR}/Tests/CPackComponentsForAll/build${CPackGen}-${CPackComponentWay}"
            "${CPackRun_CPackGen}"
            "${CPackRun_CPackComponentWay}"
            -P "${CMake_SOURCE_DIR}/Tests/CPackComponentsForAll/RunCPackVerifyResult.cmake")
        list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CPackComponentsForAll/build${CPackGen}-${CPackComponentWay}")
      endforeach()
    endforeach()
  endif()

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
    endif()
  endif()

  if(CTEST_RUN_CPackTestAllGenerators)
    add_test(CPackTestAllGenerators ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/CPackTestAllGenerators"
      "${CMake_BINARY_DIR}/Tests/CPackTestAllGenerators"
      ${build_generator_args}
      --build-project CPackTestAllGenerators
      --test-command
      ${CMAKE_CMAKE_COMMAND}
        -D dir=${CMake_BINARY_DIR}/Tests/CPackTestAllGenerators
        -P ${CMake_SOURCE_DIR}/Tests/CPackTestAllGenerators/RunCPack.cmake
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CPackTestAllGenerators")
  endif()

  if(CTEST_package_X11_TEST)
    set(X11_build_target_arg --build-target package)
  else()
    set(X11_build_target_arg)
  endif()

  add_test(X11 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/X11"
    "${CMake_BINARY_DIR}/Tests/X11"
    ${build_generator_args}
    --build-project UseX11
    --build-two-config
    ${X11_build_target_arg}
    --test-command  UseX11)
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/X11")

  if(NOT DEFINED CTEST_RUN_CMakeTestAllGenerators)
    set(CTEST_RUN_CMakeTestAllGenerators ON)
  endif()

  if(CTEST_RUN_CMakeTestAllGenerators)
    add_test(CMakeTestAllGenerators ${CMAKE_CMAKE_COMMAND}
        -D dir=${CMake_BINARY_DIR}/Tests/CMakeTestAllGenerators
        -D CMake_SOURCE_DIR=${CMake_SOURCE_DIR}
        -P ${CMake_SOURCE_DIR}/Tests/CMakeTestAllGenerators/RunCMake.cmake
      )
    list(APPEND TEST_BUILD_DIRS
      "${CMake_BINARY_DIR}/Tests/CMakeTestAllGenerators")
  endif()

  if(NOT DEFINED CTEST_RUN_CMakeTestBadCommandLines)
    set(CTEST_RUN_CMakeTestBadCommandLines ON)
  endif()

  if(CTEST_RUN_CMakeTestBadCommandLines)
    add_test(CMakeTestBadCommandLines ${CMAKE_CMAKE_COMMAND}
        -D dir=${CMake_BINARY_DIR}/Tests/CMakeTestBadCommandLines
        -D gen=${CMAKE_TEST_GENERATOR}
        -D CMake_SOURCE_DIR=${CMake_SOURCE_DIR}
        -P ${CMake_SOURCE_DIR}/Tests/CMakeTestBadCommandLines/RunCMake.cmake
      )
    list(APPEND TEST_BUILD_DIRS
      "${CMake_BINARY_DIR}/Tests/CMakeTestBadCommandLines")
  endif()

  if(NOT DEFINED CTEST_RUN_CMakeTestMultipleConfigures)
    set(CTEST_RUN_CMakeTestMultipleConfigures ON)
  endif()

  if(CTEST_RUN_CMakeTestMultipleConfigures)
    add_test(CMakeTestMultipleConfigures ${CMAKE_CMAKE_COMMAND}
        -D dir=${CMake_BINARY_DIR}/Tests/CMakeTestMultipleConfigures
        -D gen=${CMAKE_TEST_GENERATOR}
        -D CMake_SOURCE_DIR=${CMake_SOURCE_DIR}
        -P ${CMake_SOURCE_DIR}/Tests/CMakeTestMultipleConfigures/RunCMake.cmake
      )
    list(APPEND TEST_BUILD_DIRS
      "${CMake_BINARY_DIR}/Tests/CMakeTestMultipleConfigures")
  endif()

  add_test(LoadedCommandOneConfig  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/LoadCommandOneConfig"
    "${CMake_BINARY_DIR}/Tests/LoadCommandOneConfig"
    ${build_generator_args}
    --build-project LoadCommand
    --test-command  LoadedCommand
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/LoadCommandOneConfig")

  add_test(complex  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Complex"
    "${CMake_BINARY_DIR}/Tests/Complex"
    --build-two-config
    --build-config-sample "${CMAKE_CTEST_COMMAND}"
    ${build_generator_args}
    --build-project Complex
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Complex/bin"
    --build-options
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    --test-command complex
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Complex")

  add_test(complexOneConfig  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/ComplexOneConfig"
    "${CMake_BINARY_DIR}/Tests/ComplexOneConfig"
    ${build_generator_args}
    --build-project Complex
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/ComplexOneConfig/bin"
    --build-options
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    --test-command complex)
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/ComplexOneConfig")
  # because of the registry write these tests depend on each other
  set_tests_properties ( complex PROPERTIES DEPENDS complexOneConfig)

  add_test(Example ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Example"
    "${CMake_BINARY_DIR}/Example"
    ${build_generator_args}
    --build-project HELLO
    --build-exe-dir "${CMake_BINARY_DIR}/Example/Demo"
    --test-command helloDemo
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Example")

  add_test(Environment ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Environment"
    "${CMake_BINARY_DIR}/Tests/Environment"
    ${build_generator_args}
    --build-project EnvironmentProj
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Environment"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Environment")

  add_test(QtAutomocNoQt  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/QtAutomocNoQt"
    "${CMake_BINARY_DIR}/Tests/QtAutomocNoQt"
    ${build_generator_args}
    --build-project QtAutomocNoQt
    --build-options -DCMAKE_BUILD_TYPE=\${CTEST_CONFIGURATION_TYPE}
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/QtAutomocNoQt")

  find_package(Qt5Widgets QUIET NO_MODULE)
  if(Qt5Widgets_FOUND)
    add_test(Qt5Automoc ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/QtAutomoc"
      "${CMake_BINARY_DIR}/Tests/Qt5Automoc"
      ${build_generator_args}
      --build-project QtAutomoc
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/Qt5Automoc"
      --force-new-ctest-process
      --build-options -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE} -DQT_TEST_VERSION=5
      --test-command ${CMAKE_CTEST_COMMAND} -V
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Qt5Automoc")
  endif()
  if(QT4_WORKS AND QT_QTGUI_FOUND)
    add_test(Qt4Automoc ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/QtAutomoc"
      "${CMake_BINARY_DIR}/Tests/Qt4Automoc"
      ${build_generator_args}
      --build-project QtAutomoc
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/Qt4Automoc"
      --force-new-ctest-process
      --build-options -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE} -DQT_TEST_VERSION=4
      --test-command ${CMAKE_CTEST_COMMAND} -V
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Qt4Automoc")

    add_test(Qt4Targets ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/Qt4Targets"
      "${CMake_BINARY_DIR}/Tests/Qt4Targets"
      ${build_generator_args}
      --build-project Qt4Targets
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/Qt4Targets"
      --force-new-ctest-process
      --build-options -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
      --test-command ${CMAKE_CTEST_COMMAND} -V
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Qt4Targets")
  endif()

  add_test(ExternalProject ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/ExternalProject"
    "${CMake_BINARY_DIR}/Tests/ExternalProject"
    ${build_generator_args}
    --build-project ExternalProjectTest
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/ExternalProject"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/ExternalProject")
  set_tests_properties(ExternalProject PROPERTIES
    TIMEOUT ${CMAKE_LONG_TEST_TIMEOUT})

  add_test(ExternalProjectUpdateSetup ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/ExternalProjectUpdate"
    "${CMake_BINARY_DIR}/Tests/ExternalProjectUpdate"
    ${build_generator_args}
    --build-project ExternalProjectUpdateTest
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/ExternalProjectUpdate"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/ExternalProjectUpdate")
  set_tests_properties(ExternalProjectUpdateSetup PROPERTIES
    TIMEOUT ${CMAKE_LONG_TEST_TIMEOUT})

  add_test(NAME ExternalProjectUpdate
    COMMAND ${CMAKE_CMAKE_COMMAND}
    -DExternalProjectUpdate_SOURCE_DIR:PATH=${CMake_SOURCE_DIR}/Tests/ExternalProjectUpdate
    -DExternalProjectUpdate_BINARY_DIR:PATH=${CMake_BINARY_DIR}/Tests/ExternalProjectUpdate
    -DCMAKE_TEST_GENERATOR=${CMAKE_TEST_GENERATOR}
    -DCMAKE_TEST_GENERATOR_TOOLSET=${CMAKE_TEST_GENERATOR_TOOLSET}
    -DCMAKE_TEST_MAKEPROGRAM=${CMAKE_TEST_MAKEPROGRAM}
    -DCMAKE_CTEST_COMMAND=${CMAKE_CTEST_COMMAND}
    -P ${CMake_SOURCE_DIR}/Tests/ExternalProjectUpdate/ExternalProjectUpdateTest.cmake
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/ExternalProjectUpdate")
  set_tests_properties(ExternalProjectUpdate PROPERTIES
    TIMEOUT ${CMAKE_LONG_TEST_TIMEOUT}
    WORKING_DIRECTORY ${CMake_SOURCE_DIR}/Tests/ExternalProjectUpdate
    DEPENDS ExternalProjectUpdateSetup )

  # do each of the tutorial steps
  foreach(STP RANGE 1 7)
    add_test(TutorialStep${STP} ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/Tutorial/Step${STP}"
      "${CMake_BINARY_DIR}/Tests/Tutorial/Step${STP}"
      --build-two-config
      ${build_generator_args}
      --build-project Tutorial
      --test-command Tutorial 25.0)
  endforeach()
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Tutorial")

  add_test(testing ${CMAKE_CTEST_COMMAND} -C \${CTEST_CONFIGURATION_TYPE}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Testing"
    "${CMake_BINARY_DIR}/Tests/Testing"
    ${build_generator_args}
    --build-project Testing
    --test-command ${CMAKE_CTEST_COMMAND} -C \${CTEST_CONFIGURATION_TYPE}
    )
  set_tests_properties(testing PROPERTIES PASS_REGULAR_EXPRESSION "Passed")
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Testing")

  add_test(wrapping  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Wrapping"
    "${CMake_BINARY_DIR}/Tests/Wrapping"
    ${build_generator_args}
    --build-project Wrapping
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --test-command wrapping
    )
  add_test(qtwrapping  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Wrapping"
    "${CMake_BINARY_DIR}/Tests/Wrapping"
    ${build_generator_args}
    --build-project Wrapping
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
      --test-command qtwrapping
      )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Wrapping")

  add_test(testdriver1 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TestDriver"
    "${CMake_BINARY_DIR}/Tests/TestDriver"
    ${build_generator_args}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --build-project TestDriverTest
    --test-command TestDriverTest test1
    )

  add_test(testdriver2 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TestDriver"
    "${CMake_BINARY_DIR}/Tests/TestDriver"
    ${build_generator_args}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --build-project TestDriverTest
    --test-command TestDriverTest test2
    )

  add_test(testdriver3  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TestDriver"
    "${CMake_BINARY_DIR}/Tests/TestDriver"
    ${build_generator_args}
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
    ${build_generator_args}
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
    endif()
    if(NOT "${_DLL_PATH}" MATCHES "^(.*:)?\\@bindir\\@/\\.\\./lib(:.*)?$")
      message(FATAL_ERROR "In order to successfully run the CMake test suite on Syllable you need to add \"\\@bindir\\@/../lib\" to the DLL_PATH environment variable")
    endif()

  else()

    add_test(JumpWithLibOut  ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/Jump"
      "${CMake_BINARY_DIR}/Tests/Jump/WithLibOut"
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/Jump/WithLibOut/Executable"
      --build-project Jump
      ${build_generator_args}
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
      ${build_generator_args}
      --test-command jumpExecutable
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Jump")

    add_test(Plugin ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/Plugin"
      "${CMake_BINARY_DIR}/Tests/Plugin"
      ${build_generator_args}
      --build-project Plugin
      --build-two-config
      --test-command bin/example)
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Plugin")

    if(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG)
      ADD_TEST_MACRO(RuntimePath RuntimePath)
    endif()
  endif()

  add_test(linkorder1 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/LinkLineOrder"
    "${CMake_BINARY_DIR}/Tests/LinkLineOrder"
    ${build_generator_args}
    --build-project LinkLineOrder
    --test-command Exec1
    )

  add_test(linkorder2  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/LinkLineOrder"
    "${CMake_BINARY_DIR}/Tests/LinkLineOrder"
    ${build_generator_args}
    --build-project LinkLineOrder
    --test-command Exec2
    )
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/LinkLineOrder")
  set_tests_properties ( qtwrapping PROPERTIES DEPENDS wrapping)
  set_tests_properties ( testdriver1 PROPERTIES DEPENDS qtwrapping)
  set_tests_properties ( testdriver2 PROPERTIES DEPENDS testdriver1)
  set_tests_properties ( testdriver3 PROPERTIES DEPENDS testdriver2)
  set_tests_properties ( linkorder2 PROPERTIES DEPENDS linkorder1)
  set_tests_properties ( SimpleInstall-Stage2 PROPERTIES DEPENDS SimpleInstall)

  # Test static linking on toolchains known to support it.
  if("${CMAKE_C_COMPILER_ID}" MATCHES "^(GNU)$"
      AND NOT APPLE AND NOT WIN32 AND NOT CYGWIN
      AND EXISTS "/usr/lib/libm.a")
    add_test(LinkStatic  ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/LinkStatic"
      "${CMake_BINARY_DIR}/Tests/LinkStatic"
      ${build_generator_args}
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
      ${build_generator_args}
      --build-project kwsys
      --test-command kwsysTestsCxx testIOS
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/kwsys")
  endif()

  if(MAKE_SUPPORTS_SPACES)
    add_test(SubDirSpaces ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/SubDirSpaces"
      "${CMake_BINARY_DIR}/Tests/SubDirSpaces"
      --build-exe-dir
      "${CMake_BINARY_DIR}/Tests/SubDirSpaces/Executable Sources"
      ${build_generator_args}
      --build-project SUBDIR
      --test-command test
      "${CMake_BINARY_DIR}/Tests/SubDirSpaces/ShouldBeHere"
      "${CMake_BINARY_DIR}/Tests/SubDirSpaces/testfromsubdir.obj"
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/SubDirSpaces")
  endif ()

  if (WIN32)
    add_test(SubDir ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/SubDir"
      "${CMake_BINARY_DIR}/Tests/SubDir"
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/SubDir/Executable"
      ${build_generator_args}
      --build-project SUBDIR
      --test-command test
      "${CMake_BINARY_DIR}/Tests/SubDir/ShouldBeHere"
      "${CMake_BINARY_DIR}/Tests/SubDir/testfromsubdir.obj"
      )
  else ()
    add_test(SubDir ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/SubDir"
      "${CMake_BINARY_DIR}/Tests/SubDir"
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/SubDir/Executable"
      ${build_generator_args}
      --build-project SUBDIR
      --test-command test
      "${CMake_BINARY_DIR}/Tests/SubDir/ShouldBeHere"
      "${CMake_BINARY_DIR}/Tests/SubDir/testfromsubdir.o"
      )
  endif ()
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/SubDir")

  if(CMAKE_TEST_MSVC)
    ADD_TEST_MACRO(ForceInclude foo)
    ADD_TEST_MACRO(PDBDirectoryAndName myexe)
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
      ${build_generator_args}
      --build-project MakeClean
      --build-exe-dir "${CMake_BINARY_DIR}/MakeClean"
      --test-command check_clean
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/MakeClean")
  endif()

  if(NOT DEFINED CTEST_RUN_MFC)
    set(CTEST_RUN_MFC OFF)

    if(MSVC)
      set(CTEST_RUN_MFC ON)

      # Look for evidence that this is a VCExpress build. If so, avoid
      # the MFC test by default.
      string(TOLOWER "${CMAKE_TEST_MAKEPROGRAM}" mkprog)
      if(mkprog MATCHES "vcexpress")
        message(STATUS
          "CMAKE_TEST_MAKEPROGRAM indicates vcexpress, avoiding MFC test")
        set(CTEST_RUN_MFC OFF)
      endif()

      # Since MSBuild might also be the "makeprogram" for a VCExpress
      # build tree, use one more heuristic, too. The string representing
      # the .vcproj file type contains "VCExpress" on machines where an
      # express edition of VS was installed last:
      if(CTEST_RUN_MFC)
        execute_process(COMMAND cmd /c assoc .vcproj
          OUTPUT_STRIP_TRAILING_WHITESPACE
          OUTPUT_VARIABLE ov)
        if(ov MATCHES "VCExpress")
          message(STATUS
            ".vcproj file association indicates VCExpress, avoiding MFC test")
          set(CTEST_RUN_MFC OFF)
        endif()
      endif()

      # For the Watcom WMake generator, avoid the MFC test by default.
      if(CTEST_RUN_MFC)
        if("${CMAKE_TEST_GENERATOR}" MATCHES "WMake")
          message(STATUS
            "using the Watcom WMake generator, avoiding MFC test")
          set(CTEST_RUN_MFC OFF)
        endif()
      endif()

      # Last resort, after quick checks are done. Do a try_compile, and avoid
      # the MFC test if the simplest possible MFC app cannot be compiled.
      if(CTEST_RUN_MFC AND NOT DEFINED HAVE_MFC)
        configure_file(
          ${CMAKE_CURRENT_SOURCE_DIR}/MFC/try_compile/CMakeLists.txt
          ${CMAKE_CURRENT_BINARY_DIR}/MFC/try_compile/CMakeLists.txt
          COPYONLY
          )
        configure_file(
          ${CMAKE_CURRENT_SOURCE_DIR}/MFC/mfc1/stdafx.cpp
          ${CMAKE_CURRENT_BINARY_DIR}/MFC/try_compile/stdafx.cpp
          COPYONLY
          )
        configure_file(
          ${CMAKE_CURRENT_SOURCE_DIR}/MFC/mfc1/stdafx.h
          ${CMAKE_CURRENT_BINARY_DIR}/MFC/try_compile/stdafx.h
          COPYONLY
          )

        message(STATUS "Looking for MFC")

        try_compile(HAVE_MFC
          ${CMAKE_CURRENT_BINARY_DIR}/MFC/try_compile/build
          ${CMAKE_CURRENT_BINARY_DIR}/MFC/try_compile
          try_compile_mfc
          OUTPUT_VARIABLE HAVE_MFC_OUTPUT)

        if(HAVE_MFC)
          message(STATUS "Looking for MFC - found")
          set(HAVE_MFC 1 CACHE INTERNAL "Have MFC")
          file(APPEND
            ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
            "Determining if MFC exists passed with the following output:\n"
            "${HAVE_MFC_OUTPUT}\n\n")
        else()
          message(STATUS "Looking for MFC - not found")
          set(HAVE_MFC "" CACHE INTERNAL "Have MFC")
          file(APPEND
            ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
            "Determining if MFC exists failed with the following output:\n"
            "${HAVE_MFC_OUTPUT}\n\n")
        endif()
      endif()

      if(CTEST_RUN_MFC AND NOT HAVE_MFC)
        message(STATUS
          "cannot compile simplest ever MFC app, avoiding MFC test")
        set(CTEST_RUN_MFC OFF)
      endif()
    endif()
  endif()

  if(CTEST_RUN_MFC)
    add_test(MFC ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/MFC"
      "${CMake_BINARY_DIR}/Tests/MFC"
      --build-two-config
      ${build_generator_args}
      --build-project mfc_driver
      --test-command ${CMAKE_CTEST_COMMAND}
        -C \${CTEST_CONFIGURATION_TYPE} -VV)
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/MFC")
  endif()

  if(${CMAKE_TEST_GENERATOR} MATCHES "Visual Studio")
    if(NOT MSVC60)
      ADD_TEST_MACRO(SBCS SBCS)
    endif()

    if(NOT "${CMAKE_TEST_GENERATOR}" MATCHES "Visual Studio [6789]( |$)"
        AND NOT CMAKE_TEST_GENERATOR_TOOLSET)
      ADD_TEST_MACRO(VSWindowsFormsResx VSWindowsFormsResx)
    endif()

    add_test(VSExternalInclude ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/VSExternalInclude"
      "${CMake_BINARY_DIR}/Tests/VSExternalInclude"
      --build-two-config
      ${build_generator_args}
      --build-project VSExternalInclude
      --test-command VSExternalInclude)
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/VSExternalInclude")

    add_test(VSMidl ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/VSMidl"
      "${CMake_BINARY_DIR}/Tests/VSMidl"
      --build-two-config
      ${build_generator_args}
      --build-project VSMidl
      --test-command VSMidl)
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/VSMidl")

    if(NOT MSVC60 AND NOT CMAKE_TEST_MAKEPROGRAM MATCHES "[mM][sS][bB][uU][iI][lL][dD]\\.[eE][xX][eE]")
      # The test (and tested property) works with .sln files, so it's skipped when:
      # * Using VS6, which doesn't use .sln files
      # * cmake --build is set up to use MSBuild, since the MSBuild invocation does not use the .sln file
      set(_last_test "")
      foreach(config ${CMAKE_CONFIGURATION_TYPES})
        add_test(NAME VSExcludeFromDefaultBuild-${config} COMMAND ${CMAKE_CTEST_COMMAND}
          --build-and-test
          "${CMake_SOURCE_DIR}/Tests/VSExcludeFromDefaultBuild"
          "${CMake_BINARY_DIR}/Tests/VSExcludeFromDefaultBuild"
          --build-config ${config}
          --build-two-config
          ${build_generator_args}
          --build-project VSExcludeFromDefaultBuild
          --test-command ${CMAKE_COMMAND}
             -D "activeConfig=${config}"
             -D "allConfigs=${CMAKE_CONFIGURATION_TYPES}"
             -D "dir=${CMake_BINARY_DIR}/Tests/VSExcludeFromDefaultBuild"
             -P "${CMake_SOURCE_DIR}/Tests/VSExcludeFromDefaultBuild/ResultTest.cmake")
        if(_last_test)
          set_property(TEST VSExcludeFromDefaultBuild-${config} PROPERTY DEPENDS ${_last_test})
        endif()
        set(_last_test "VSExcludeFromDefaultBuild-${config}")
      endforeach()
      unset(_last_test)
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/VSExcludeFromDefaultBuild")
    endif()

    if(CMAKE_TEST_GENERATOR MATCHES "Visual Studio ([0-5]|[6-9][0-9])")
      if(CMAKE_TEST_MAKEPROGRAM MATCHES "[mM][sS][bB][uU][iI][lL][dD]\\.[eE][xX][eE]")
        set(MSBUILD_EXECUTABLE "${CMAKE_TEST_MAKEPROGRAM}")
      else()
        set(_FDIR "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VC7;FrameworkDir32]")
        set(_FVER "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VC7;FrameworkVer32]")
        find_program(MSBUILD_EXECUTABLE NAMES msbuild HINTS ${_FDIR}/${_FVER})
      endif()
      if(MSBUILD_EXECUTABLE)
        add_test(NAME VSProjectInSubdir COMMAND ${CMAKE_CTEST_COMMAND}
          --build-and-test
          "${CMake_SOURCE_DIR}/Tests/VSProjectInSubdir"
          "${CMake_BINARY_DIR}/Tests/VSProjectInSubdir"
          --build-two-config
          --build-generator ${CMAKE_TEST_GENERATOR}
          --build-generator-toolset "${CMAKE_TEST_GENERATOR_TOOLSET}"
          --build-makeprogram "${MSBUILD_EXECUTABLE}"
          --build-project VSProjectInSubdir
          --build-target test)
        list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/VSProjectInSubdir")
      endif()
    endif()
  endif()

  if (APPLE)
    if (CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
      set(BundleTestInstallDir
        "${CMake_BINARY_DIR}/Tests/BundleTest/InstallDirectory")
      add_test(BundleTest ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/BundleTest"
        "${CMake_BINARY_DIR}/Tests/BundleTest"
        --build-two-config
        ${build_generator_args}
        --build-project BundleTest
        --build-target install
#       --build-target package
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
        ${build_generator_args}
        --build-project CFBundleTest
        --test-command
        ${CMAKE_CMAKE_COMMAND} -DCTEST_CONFIGURATION_TYPE=\${CTEST_CONFIGURATION_TYPE}
        -Ddir=${CMake_BINARY_DIR}/Tests/CFBundleTest
        -Dgen=${CMAKE_TEST_GENERATOR}
        -P ${CMake_SOURCE_DIR}/Tests/CFBundleTest/VerifyResult.cmake)
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CFBundleTest")

      ADD_TEST_MACRO(ObjC++ ObjC++)
    endif ()
  endif ()

  if(APPLE AND CTEST_TEST_CPACK)
    add_test(BundleGeneratorTest ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/BundleGeneratorTest"
      "${CMake_BINARY_DIR}/Tests/BundleGeneratorTest"
      --build-two-config
      ${build_generator_args}
      --build-project BundleGeneratorTest
      --build-target package
      --build-options "-DCMAKE_INSTALL_PREFIX:PATH=${CMake_BINARY_DIR}/Tests/BundleGeneratorTest/InstallDirectory"
      )
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/BundleGeneratorTest")
  endif()

  add_test(WarnUnusedUnusedViaSet ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/VariableUnusedViaSet"
    "${CMake_BINARY_DIR}/Tests/WarnUnusedUnusedViaSet"
    ${build_generator_args}
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
    ${build_generator_args}
    --build-noclean
    --build-project WarnUnusedUnusedViaUnset
    --build-options "--warn-unused-vars")
  set_tests_properties(WarnUnusedUnusedViaUnset PROPERTIES
    PASS_REGULAR_EXPRESSION "CMake Warning .*VariableUnusedViaUnset.CMakeLists.txt:7 \\(set\\):")
  set_tests_properties(WarnUnusedUnusedViaUnset PROPERTIES
    FAIL_REGULAR_EXPRESSION "CMake Warning .*VariableUnusedViaUnset.CMakeLists.txt:5 \\(set\\):")
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/WarnUnusedUnusedViaUnset")

  if("${CMAKE_TEST_GENERATOR}" MATCHES "Makefile" AND NOT WIN32)
    # Ninja does not support ADDITIONAL_MAKE_CLEAN_FILES and therefore fails
    # this test. (See #13371)
    # Apparently Visual Studio does not support it either. As the MakeClean
    # test above is only run with the Makefiles generator, only run this
    # test with the Makefiles generator also.
    add_test(WarnUnusedCliUnused ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/WarnUnusedCliUnused"
      "${CMake_BINARY_DIR}/Tests/WarnUnusedCliUnused"
      ${build_generator_args}
      --build-project WarnUnusedCliUnused
      --build-options "-DUNUSED_CLI_VARIABLE=Unused")
    set_tests_properties(WarnUnusedCliUnused PROPERTIES
      PASS_REGULAR_EXPRESSION "CMake Warning:.*Manually-specified variables were not used by the project:.*  UNUSED_CLI_VARIABLE")
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/WarnUnusedCliUnused")
  endif()

  add_test(WarnUnusedCliUsed ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/VariableUsage"
    "${CMake_BINARY_DIR}/Tests/WarnUnusedCliUsed"
    ${build_generator_args}
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
    ${build_generator_args}
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
    ${build_generator_args}
    --build-project TestsWorkingDirectoryProj
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

  # A test for ctest_build() with targets in subdirectories
  if(CMAKE_TEST_GENERATOR_TOOLSET)
    set(CMAKE_TEST_GENERATOR_TOOLSET_SELECTION "-T;${CMAKE_TEST_GENERATOR_TOOLSET};")
  else()
    set(CMAKE_TEST_GENERATOR_TOOLSET_SELECTION)
  endif()
  configure_file("${CMake_SOURCE_DIR}/Tests/CTestBuildCommandProjectInSubdir/CTestBuildCommandProjectInSubdir.cmake.in"
                 "${CMake_BINARY_DIR}/Tests/CTestBuildCommandProjectInSubdir/CTestBuildCommandProjectInSubdir.cmake" @ONLY)
  unset(CMAKE_TEST_GENERATOR_TOOLSET_SELECTION)
  add_test(CTest.BuildCommand.ProjectInSubdir
    ${CMAKE_CTEST_COMMAND} -S "${CMake_BINARY_DIR}/Tests/CTestBuildCommandProjectInSubdir/CTestBuildCommandProjectInSubdir.cmake"
                           -DCMAKE_MAKE_PROGRAM:FILEPATH=${CMAKE_TEST_MAKEPROGRAM})
  list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CTestBuildCommandProjectInSubdir/Nested")

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
      endif()
    endif()
    if(Subversion_FOUND)
      set(CTestUpdateSVN_DIR "CTest UpdateSVN")
      configure_file("${CMake_SOURCE_DIR}/Tests/CTestUpdateSVN.cmake.in"
        "${CMake_BINARY_DIR}/Tests/CTestUpdateSVN.cmake" @ONLY)
      add_test(CTest.UpdateSVN ${CMAKE_CMAKE_COMMAND}
        -P "${CMake_BINARY_DIR}/Tests/CTestUpdateSVN.cmake"
        )
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${CTestUpdateSVN_DIR}")
    endif()

    # Test CTest Update with CVS
    if(EXISTS ${CMAKE_ROOT}/Modules/FindCVS.cmake)
      find_package(CVS QUIET)
    else()
      find_program(CVS_EXECUTABLE NAMES cvs)
      set(CVS_FOUND ${CVS_EXECUTABLE})
    endif()
    set(CTEST_TEST_UPDATE_CVS ${CVS_FOUND})
    if(CTEST_TEST_UPDATE_CVS AND NOT UNIX)
      if("${CVS_EXECUTABLE}" MATCHES "cygwin")
        message(STATUS "No CTest.UpdateCVS test with cygwin cvs.exe outside cygwin!")
        set(CTEST_TEST_UPDATE_CVS 0)
      endif()
    endif()
    if(CTEST_TEST_UPDATE_CVS)
      set(CTestUpdateCVS_DIR "CTest UpdateCVS")
      configure_file("${CMake_SOURCE_DIR}/Tests/CTestUpdateCVS.cmake.in"
        "${CMake_BINARY_DIR}/Tests/CTestUpdateCVS.cmake" @ONLY)
      add_test(CTest.UpdateCVS ${CMAKE_CMAKE_COMMAND}
        -P "${CMake_BINARY_DIR}/Tests/CTestUpdateCVS.cmake"
        )
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${CTestUpdateCVS_DIR}")
    endif()

    # Test CTest Update with BZR
    find_program(BZR_EXECUTABLE NAMES bzr)
    mark_as_advanced(BZR_EXECUTABLE)
    set(CTEST_TEST_UPDATE_BZR 0)
    if(BZR_EXECUTABLE)
      if(NOT "${BZR_EXECUTABLE}" MATCHES "cygwin" OR UNIX)
        set(CTEST_TEST_UPDATE_BZR 1)
      endif()
    endif()
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
      endif()
    endif()

    # Test CTest Update with GIT
    find_program(GIT_EXECUTABLE NAMES git)
    mark_as_advanced(GIT_EXECUTABLE)
    set(CTEST_TEST_UPDATE_GIT 0)
    if(GIT_EXECUTABLE)
      if(NOT "${GIT_EXECUTABLE}" MATCHES "cygwin" OR UNIX)
        set(CTEST_TEST_UPDATE_GIT 1)
      endif()
    endif()
    if(CTEST_TEST_UPDATE_GIT)
      set(CTestUpdateGIT_DIR "CTest UpdateGIT")
      configure_file("${CMake_SOURCE_DIR}/Tests/CTestUpdateGIT.cmake.in"
        "${CMake_BINARY_DIR}/Tests/CTestUpdateGIT.cmake" @ONLY)
      add_test(CTest.UpdateGIT ${CMAKE_CMAKE_COMMAND}
        -P "${CMake_BINARY_DIR}/Tests/CTestUpdateGIT.cmake"
        )
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${CTestUpdateGIT_DIR}")
    endif()

    # Test CTest Update with HG
    find_program(HG_EXECUTABLE NAMES hg)
    mark_as_advanced(HG_EXECUTABLE)
    set(CTEST_TEST_UPDATE_HG 0)
    if(HG_EXECUTABLE)
      if(NOT "${HG_EXECUTABLE}" MATCHES "cygwin" OR UNIX)
        set(CTEST_TEST_UPDATE_HG 1)
      endif()
    endif()
    if(CTEST_TEST_UPDATE_HG)
      set(CTestUpdateHG_DIR "CTest UpdateHG")
      configure_file("${CMake_SOURCE_DIR}/Tests/CTestUpdateHG.cmake.in"
        "${CMake_BINARY_DIR}/Tests/CTestUpdateHG.cmake" @ONLY)
      add_test(CTest.UpdateHG ${CMAKE_CMAKE_COMMAND}
        -P "${CMake_BINARY_DIR}/Tests/CTestUpdateHG.cmake"
        )
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${CTestUpdateHG_DIR}")
    endif()
  endif()

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

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestConfigFileInBuildDir/test1.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestConfigFileInBuildDir1/test1.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestConfigFileInBuildDir1 ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestConfigFileInBuildDir1/test1.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestConfigFileInBuildDir1/testOut1.log"
    )
  REGEX_ESCAPE_STRING(CTEST_TEST_ESCAPED_SOURCE_DIR "${CMake_SOURCE_DIR}")
  set_tests_properties(CTestTestConfigFileInBuildDir1 PROPERTIES DEPENDS CTestTestNoBuild
    PASS_REGULAR_EXPRESSION
      "Reading ctest configuration file: ${CTEST_TEST_ESCAPED_SOURCE_DIR}.Tests.CTestTestConfigFileInBuildDir.CTestConfig.cmake")

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestConfigFileInBuildDir/test2.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestConfigFileInBuildDir2/test2.cmake"
    @ONLY ESCAPE_QUOTES)
  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestConfigFileInBuildDir/CTestConfig.cmake"
    "${CMake_BINARY_DIR}/Tests/CTestTestConfigFileInBuildDir2/CTestConfig.cmake"
    @ONLY ESCAPE_QUOTES COPYONLY)
  add_test(CTestTestConfigFileInBuildDir2 ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestConfigFileInBuildDir2/test2.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestConfigFileInBuildDir2/testOut2.log"
    )
  REGEX_ESCAPE_STRING(CTEST_TEST_ESCAPED_BINARY_DIR "${CMake_BINARY_DIR}")
  set_tests_properties(CTestTestConfigFileInBuildDir2 PROPERTIES DEPENDS CTestTestNoBuild
    REQUIRED_FILES ${CMake_BINARY_DIR}/Tests/CTestTestConfigFileInBuildDir2/CTestConfig.cmake
    PASS_REGULAR_EXPRESSION
      "Reading ctest configuration file: ${CTEST_TEST_ESCAPED_BINARY_DIR}.Tests.CTestTestConfigFileInBuildDir2.CTestConfig.cmake")

  # test coverage for mumps
  # create a MumpsCoverage dir in the binary tree under Testing to
  # avoid the .NoDartCoverage files in the cmake testing tree
  configure_file(
     "${CMake_SOURCE_DIR}/Tests/MumpsCoverage/DartConfiguration.tcl.in"
     "${CMake_BINARY_DIR}/Testing/MumpsCoverage/DartConfiguration.tcl")
  configure_file(
     "${CMake_SOURCE_DIR}/Tests/MumpsCoverage/gtm_coverage.mcov.in"
     "${CMake_BINARY_DIR}/Testing/MumpsCoverage/gtm_coverage.mcov")
  file(COPY "${CMake_SOURCE_DIR}/Tests/MumpsCoverage/VistA-FOIA"
    DESTINATION "${CMake_BINARY_DIR}/Testing/MumpsCoverage")
  add_test(NAME CTestGTMCoverage
    COMMAND cmake -E chdir
    ${CMake_BINARY_DIR}/Testing/MumpsCoverage
    $<TARGET_FILE:ctest> -T Coverage --debug)
  set_tests_properties(CTestGTMCoverage PROPERTIES
      PASS_REGULAR_EXPRESSION
      "Process file.*XINDEX.m.*Total LOC:.*127.*Percentage Coverage: 85.83.*"
      ENVIRONMENT COVFILE=)

  configure_file(
     "${CMake_SOURCE_DIR}/Tests/MumpsCoverage/DartConfiguration.cache.tcl.in"
     "${CMake_BINARY_DIR}/Testing/MumpsCacheCoverage/DartConfiguration.tcl")
  configure_file(
    "${CMake_SOURCE_DIR}/Tests/MumpsCoverage/cache_coverage.cmcov.in"
    "${CMake_BINARY_DIR}/Testing/MumpsCacheCoverage/cache_coverage.cmcov")
  file(COPY "${CMake_SOURCE_DIR}/Tests/MumpsCoverage/VistA-FOIA"
    DESTINATION "${CMake_BINARY_DIR}/Testing/MumpsCacheCoverage")
  add_test(NAME CTestCacheCoverage
    COMMAND cmake -E chdir
    ${CMake_BINARY_DIR}/Testing/MumpsCacheCoverage
    $<TARGET_FILE:ctest> -T Coverage --debug)
  set_tests_properties(CTestCacheCoverage PROPERTIES
      PASS_REGULAR_EXPRESSION
      "Process file.*XINDEX.m.*Total LOC:.*125.*Percentage Coverage: 85.60.*"
      ENVIRONMENT COVFILE=)
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

  # Test -S script with some -D variable definition args to ctest:
  add_test(CTestConfig.ScriptWithArgs ${CMAKE_CTEST_COMMAND}
    -C "Release"
    -D arg1=this
    -D arg2=that
    -D "arg3=the other"
    "-Darg4=this is the fourth"
    -Darg5=the_fifth
    -Darg6:STRING=value-with-type
    -S "${CMake_SOURCE_DIR}/Tests/CTestConfig/ScriptWithArgs.cmake" -VV
    --output-log "${CMake_BINARY_DIR}/Tests/CTestConfig/ScriptWithArgs.log"
    )

  ADD_TEST_MACRO(CMakeCommands.target_link_libraries target_link_libraries)
  ADD_TEST_MACRO(CMakeCommands.target_include_directories target_include_directories)
  ADD_TEST_MACRO(CMakeCommands.target_compile_definitions target_compile_definitions)

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
  else()
    set_tests_properties(CTestTestCrash PROPERTIES
      PASS_REGULAR_EXPRESSION "(Illegal|SegFault)")
  endif()

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

  set(CTestLimitDashJ_EXTRA_OPTIONS --force-new-ctest-process)
  add_test_macro(CTestLimitDashJ ${CMAKE_CTEST_COMMAND} -j 4
    --output-on-failure -C "\${CTestTest_CONFIG}")

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

  configure_file(
    "${CMake_SOURCE_DIR}/Tests/CTestTestFdSetSize/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestFdSetSize/test.cmake"
    @ONLY ESCAPE_QUOTES)
  add_test(CTestTestFdSetSize ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestFdSetSize/test.cmake" -j20 -V --timeout 120
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestFdSetSize/testOutput.log"
    )

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


  if (CMAKE_TESTS_CDASH_SERVER)
    set(regex "^([^:]+)://([^/]+)(/.*)$")

    if ("${CMAKE_TESTS_CDASH_SERVER}" MATCHES "${regex}")
      set(protocol "${CMAKE_MATCH_1}")
      set(server "${CMAKE_MATCH_2}")
      set(path "${CMAKE_MATCH_3}")
    else ()
      set(protocol "http")
      set(server "www.cdash.org")
      set(path "/CDash")
      message("warning: CMAKE_TESTS_CDASH_SERVER does not match expected regex...")
      message("         ...using default url='${protocol}://${server}${path}' for CTestTest[23]")
    endif ()
  endif ()


  if (CTEST_TEST_CTEST AND CMAKE_RUN_LONG_TESTS AND CMAKE_TESTS_CDASH_SERVER)
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
    if ("${PREVIOUS_TIMEOUT}" MATCHES NOTFOUND)
      set_tests_properties ( CTestTest
        PROPERTIES TIMEOUT ${CMAKE_LONG_TEST_TIMEOUT})
    endif ()

    get_test_property(CTestTest2 TIMEOUT PREVIOUS_TIMEOUT)
    if ("${PREVIOUS_TIMEOUT}" MATCHES NOTFOUND)
      set_tests_properties ( CTestTest2
        PROPERTIES TIMEOUT ${CMAKE_LONG_TEST_TIMEOUT})
    endif ()
  endif ()

  if("${CMAKE_TEST_GENERATOR}" MATCHES Xcode)
    set(CMAKE_SKIP_BOOTSTRAP_TEST 1)
  endif()
  if(EXISTS "${CMake_BINARY_DIR}/CMakeLists.txt")
    # If there is CMakeLists.txt in the binary tree, assume in-source build
    set(CMAKE_SKIP_BOOTSTRAP_TEST 1)
  endif()
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
    if ("${PREVIOUS_TIMEOUT}" MATCHES NOTFOUND)
      set_tests_properties ( BootstrapTest
        PROPERTIES TIMEOUT 5400)
    endif ()
  endif()

  if(CMAKE_Fortran_COMPILER)
    add_test(Fortran ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/Fortran"
      "${CMake_BINARY_DIR}/Tests/Fortran"
      ${build_generator_args}
      --build-project testf
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
        ${build_generator_args}
        --build-project FortranC
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
          ${build_generator_args}
          --build-project hello
          --build-two-config
          --build-run-dir "${CMake_BINARY_DIR}/Tests/Java/"
          --test-command ${JAVA_RUNTIME} -classpath hello.jar HelloWorld)
        list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Java")
      endif()
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
        ${build_generator_args}
        --build-project SimpleC
        --build-options
        "-DCMAKE_SYSTEM_NAME=Generic"
        "-DCMAKE_C_COMPILER=${SDCC_EXECUTABLE}")
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/SimpleCOnly_sdcc")
    endif()

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
        ${build_generator_args}
        --build-project Simple
        --build-options
        "-DCMAKE_SYSTEM_NAME=Windows"
        "-DCMAKE_C_COMPILER=${MINGW_CC_LINUX2WIN_EXECUTABLE}"
        "-DCMAKE_CXX_COMPILER=${MINGW_CXX_LINUX2WIN_EXECUTABLE}"
        "-DCMAKE_RC_COMPILER=${MINGW_RC_LINUX2WIN_EXECUTABLE}"
        )
      list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Simple_Mingw_Linux2Win")
    endif()
  endif()

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
    endif()
  endif()

  if(CMAKE_TEST_PROJECT_CSE_DIR)
    set(script "${CMAKE_TEST_PROJECT_CSE_DIR}/BuildProjectCSE.cmake")
    if(NOT EXISTS "${script}")
      set(script "${CMAKE_TEST_PROJECT_CSE_DIR}/cse_build.cmake")
    endif()
    if(NOT EXISTS "${script}")
      message("warning: CMAKE_TEST_PROJECT_CSE_DIR set, but no build script found...")
    endif()

    add_test(BuildCSE ${CMAKE_CTEST_COMMAND} -V -S "${script}")
    set_tests_properties(BuildCSE PROPERTIES TIMEOUT 5400)
  endif()

  if(CMAKE_TEST_PLPLOT_DIR)
    add_test(plplot ${CMAKE_CTEST_COMMAND} -V -S ${CMAKE_TEST_PLPLOT_DIR}/../../EasyDashboardScripts/plplot.cmake )
    set_tests_properties ( plplot PROPERTIES TIMEOUT 5400)
  endif()

  if(CMAKE_TEST_CHICKEN_DIR)
    add_test(Chicken ${CMAKE_CTEST_COMMAND} -V -S ${CMAKE_TEST_CHICKEN_DIR}/../../EasyDashboardScripts/Chicken.cmake )
    set_tests_properties ( Chicken PROPERTIES TIMEOUT 5400)
  endif()

  if(CMAKE_TEST_KDELIBS_ALPHA_1_DIR)
    add_test(KDELibsAlpha1 ${CMAKE_CTEST_COMMAND} -V -S ${CMAKE_TEST_KDELIBS_ALPHA_1_DIR}/../../EasyDashboardScripts/kdelibs.cmake )
    set_tests_properties ( KDELibsAlpha1 PROPERTIES TIMEOUT 5400)
  endif()

  # If this is not an in-source build, provide a target to wipe out
  # all the test build directories.
  if(NOT EXISTS "${CMake_BINARY_DIR}/CMakeLists.txt")
    configure_file(${CMake_SOURCE_DIR}/Tests/test_clean.cmake.in
                   ${CMake_BINARY_DIR}/Tests/test_clean.cmake @ONLY)
    add_custom_target(test_clean
      COMMAND ${CMAKE_COMMAND} -P ${CMake_BINARY_DIR}/Tests/test_clean.cmake
      COMMENT "Removing test build directories."
      )
  endif()

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
      --build-options -DMAKE_SUPPORTS_SPACES=${MAKE_SUPPORTS_SPACES})
    ADD_TEST_MACRO(CompileCommandOutput
      "${CMake_BINARY_DIR}/Tests/CMakeLib/runcompilecommands")
  endif()

  add_test(IncludeDirectories ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/IncludeDirectories"
    "${CMake_BINARY_DIR}/Tests/IncludeDirectories"
    --build-two-config
    ${build_generator_args}
    --build-project IncludeDirectories
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

  add_subdirectory(CMakeTests)
endif()
