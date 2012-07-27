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
    ${${NAME}_EXTRA_OPTIONS}
    --test-command ${COMMAND} ${ARGN})
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/${dir}")
ENDMACRO(ADD_TEST_MACRO)

MACRO(REGEX_ESCAPE_STRING _OUT _IN)
    # Escape special regex metacharacters with a backslash
    string(REGEX REPLACE "([$^.[|*+?()]|])" "\\\\\\1" ${_OUT} "${_IN}")
ENDMACRO(REGEX_ESCAPE_STRING _OUT _IN)

INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/CheckFortran.cmake)

# Fake a user home directory to avoid polluting the real one.
IF(DEFINED ENV{HOME} AND NOT CTEST_NO_TEST_HOME)
  SET(TEST_HOME "${CMake_BINARY_DIR}/Tests/CMakeFiles/TestHome")
  FILE(MAKE_DIRECTORY "${TEST_HOME}")
  FILE(WRITE "${TEST_HOME}/.cvspass" ":pserver:anoncvs@www.cmake.org:/cvsroot/KWSys A\n")
  SET(TEST_HOME_ENV_CODE "# Fake a user home directory to avoid polluting the real one.
# But provide original ENV{HOME} value in ENV{CTEST_REAL_HOME} for tests that
# need access to the real HOME directory.
SET(ENV{CTEST_REAL_HOME} \"\$ENV{HOME}\")
SET(ENV{HOME} \"${TEST_HOME}\")
")
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
  IF("${CMAKE_TEST_GENERATOR}" MATCHES "Unix Makefiles" OR ("${CMAKE_TEST_GENERATOR}" MATCHES Ninja AND NOT WIN32))
    SET(TEST_CompileCommandOutput 1)
  ENDIF()

  ADD_SUBDIRECTORY(CMakeLib)
  ADD_SUBDIRECTORY(CMakeOnly)
  ADD_SUBDIRECTORY(RunCMake)

  ADD_SUBDIRECTORY(FindPackageModeMakefileTest)

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
  # assume no resources building to test
  SET(TEST_RESOURCES FALSE)
  # for windows and cygwin assume we have resources
  IF(WIN32 OR CYGWIN)
    SET(TEST_RESOURCES TRUE)
  ENDIF()
  # for borland and watcom there is no resource support
  IF("${CMAKE_TEST_GENERATOR}" MATCHES "WMake" OR
      "${CMAKE_TEST_GENERATOR}" MATCHES "Borland")
    SET(TEST_RESOURCES FALSE)
  ENDIF()
  IF(TEST_RESOURCES)
    ADD_TEST_MACRO(VSResource VSResource)
  ENDIF()
  ADD_TEST_MACRO(Simple Simple)
  ADD_TEST_MACRO(PreOrder PreOrder)
  ADD_TEST_MACRO(MissingSourceFile MissingSourceFile)
  SET_TESTS_PROPERTIES(MissingSourceFile PROPERTIES
    PASS_REGULAR_EXPRESSION "CMake Error at CMakeLists.txt:3 \\(add_executable\\):[ \r\n]*Cannot find source file:[ \r\n]*DoesNotExist/MissingSourceFile.c")
  IF(CMAKE_Fortran_COMPILER)
    ADD_TEST_MACRO(FortranOnly FortranOnly)
  ENDIF()
  # test Visual Studio GNU Fortran mixing with cmake_add_fortran_subdirectory
  # run this project if we have a working fortran compiler or
  # the test is enabled with CMAKE_TEST_CMAKE_ADD_FORTRAN cache variable.
  # If you enable the test, CMake should find the MinGW fortran install,
  # or in some cases you might need to set the PATH so that cmake can find
  # the gfortran from mingw.
  IF(CMAKE_Fortran_COMPILER OR CMAKE_TEST_CMAKE_ADD_FORTRAN)
    SET(CMAKE_SKIP_VSGNUFortran FALSE)
    # disable test for apple builds using ifort if they are building
    # more than one architecture, as ifort does not support that.
    IF(APPLE AND (CMAKE_Fortran_COMPILER MATCHES ifort))
      LIST(LENGTH CMAKE_OSX_ARCHITECTURES len)
      IF("${len}" GREATER 1)
        MESSAGE(STATUS "Skip VSGNUFortran for ifort dual cpu mac build")
        SET(CMAKE_SKIP_VSGNUFortran TRUE)
      ENDIF()
    ENDIF()
    IF((CMAKE_C_COMPILER MATCHES lsb)
        AND (CMAKE_Fortran_COMPILER MATCHES ifort))
      MESSAGE(STATUS "Skip VSGNUFortran for ifort and lsb compilers")
      SET(CMAKE_SKIP_VSGNUFortran TRUE)
    ENDIF()
    IF(NOT CMAKE_SKIP_VSGNUFortran)
      ADD_TEST_MACRO(VSGNUFortran ${CMAKE_COMMAND} -P runtest.cmake)
    ENDIF()
  ENDIF()
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
  SET_TESTS_PROPERTIES(EmptyLibrary PROPERTIES
    PASS_REGULAR_EXPRESSION "CMake Error: CMake can not determine linker language for target:test")
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

  IF(NOT QT4_FOUND)
    FIND_PACKAGE(Qt4)
  ENDIF(NOT QT4_FOUND)

  IF(QT4_FOUND)
    # test whether the Qt4 which has been found works, on some machines
    # which run nightly builds there were errors like "wrong file format"
    # for libQtCore.so. So first check it works, and only if it does add
    # the automoc test.
    INCLUDE(CheckCXXSourceCompiles)
    SET(_save_CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES}")
    SET(_save_CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES}")

    SET(CMAKE_REQUIRED_INCLUDES ${QT_INCLUDES})
    SET(CMAKE_REQUIRED_LIBRARIES ${QT_QTCORE_LIBRARIES})

    CHECK_CXX_SOURCE_COMPILES("#include <QCoreApplication>\n int main() {return (qApp == 0 ? 0 : 1); }\n"
                              QT4_WORKS)

    SET(CMAKE_REQUIRED_INCLUDES "${_save_CMAKE_REQUIRED_INCLUDES}")
    SET(CMAKE_REQUIRED_LIBRARIES "${_save_CMAKE_REQUIRED_LIBRARIES}")
  ENDIF()

  # run test for BundleUtilities on supported platforms/compilers
  if(MSVC OR
     CMAKE_SYSTEM_NAME MATCHES "Linux" OR
     CMAKE_SYSTEM_NAME MATCHES "Darwin")
  if(NOT "${CMAKE_TEST_GENERATOR}" STREQUAL "Watcom WMake")

    ADD_TEST(BundleUtilities ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/BundleUtilities"
      "${CMake_BINARY_DIR}/Tests/BundleUtilities"
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-project BundleUtilities
      )
    LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/BundleUtilities")

    # run test for DeployQt4 on supported platforms/compilers (which depends on BundleUtilities)
    # this test also depends on the existence of the standard qtiff plugin
    if(QT4_WORKS AND QT_QTSQL_FOUND)
      ADD_TEST(Qt4Deploy ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/Qt4Deploy"
        "${CMake_BINARY_DIR}/Tests/Qt4Deploy"
        --build-generator ${CMAKE_TEST_GENERATOR}
        --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
        --build-project Qt4Deploy
        --build-options
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
        )
      LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Qt4Deploy")
    endif()

  endif()
  endif()

  SET(CMAKE_BUILD_TEST_SOURCE_DIR "${CMake_SOURCE_DIR}/Tests/COnly")
  SET(CMAKE_BUILD_TEST_BINARY_DIR "${CMake_BINARY_DIR}/Tests/CMakeBuildCOnly")
  SET(CMAKE_BUILD_TEST_EXE COnly)
  CONFIGURE_FILE("${CMake_SOURCE_DIR}/Tests/CMakeBuildTest.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CMakeBuildTest.cmake" @ONLY)
  ADD_TEST(CMakeBuildTest ${CMAKE_CMAKE_COMMAND} -P
    "${CMake_BINARY_DIR}/Tests/CMakeBuildTest.cmake")
  LIST(APPEND TEST_BUILD_DIRS ${CMAKE_BUILD_TEST_BINARY_DIR})
  # now do it again for a project that has two project commands
  SET(CMAKE_BUILD_TEST_SOURCE_DIR "${CMake_SOURCE_DIR}/Tests/DoubleProject")
  SET(CMAKE_BUILD_TEST_BINARY_DIR "${CMake_BINARY_DIR}/Tests/DoubleProject")
  SET(CMAKE_BUILD_TEST_EXE just_silly)
  CONFIGURE_FILE("${CMake_SOURCE_DIR}/Tests/CMakeBuildTest.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CMakeBuildDoubleProjectTest.cmake" @ONLY)
  ADD_TEST(CMakeDoubleProject ${CMAKE_CMAKE_COMMAND} -P
    "${CMake_BINARY_DIR}/Tests/CMakeBuildDoubleProjectTest.cmake")
  LIST(APPEND TEST_BUILD_DIRS ${CMAKE_BUILD_TEST_BINARY_DIR})

  ADD_TEST_MACRO(Module.CheckTypeSize CheckTypeSize)

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

    IF(${CMAKE_TEST_GENERATOR} MATCHES "Ninja")
      # The Ninja generator does not create a recursive build system.  Start
      # from the root directory.
      SET(SubProject_SUBDIR)
    ELSE()
      SET(SubProject_SUBDIR "/foo")
    ENDIF()

    # For stage 2, do not run cmake again.
    # Then build the foo sub project which should build
    # the bar library which should be referenced because
    # foo links to the static library bar, but bar is not
    # directly in the foo sub project
    ADD_TEST(SubProject-Stage2  ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/SubProject${SubProject_SUBDIR}"
      "${CMake_BINARY_DIR}/Tests/SubProject${SubProject_SUBDIR}"
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-nocmake
      --build-project foo
      --build-target foo
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/SubProject/foo"
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
${CMake_BINARY_DIR}/bin/cmake -DCMAKE_CREATE_VERSION=nightly -P ${CMake_SOURCE_DIR}/Utilities/Release/${script}
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
    ADD_NIGHTLY_BUILD_TEST(CMakeNightlyMac64
      dashmacmini5_release.cmake)
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

  ADD_TEST(ArgumentExpansion  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/ArgumentExpansion"
    "${CMake_BINARY_DIR}/Tests/ArgumentExpansion"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project ArgumentExpansion
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/ArgumentExpansion/bin"
    )
  SET_TESTS_PROPERTIES(ArgumentExpansion PROPERTIES
    FAIL_REGULAR_EXPRESSION "Unexpected: ")
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/ArgumentExpansion")

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
   #-P "${CMake_BINARY_DIR}/Tests/SimpleExclude/run.cmake"
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
  set(CTEST_RUN_CPackComponentsForAll ${CTEST_TEST_CPACK})

  if (CMAKE_SYSTEM_NAME MATCHES "Linux" AND NOT CMAKE_CURRENT_BINARY_DIR MATCHES ".* .*")
    find_program(RPMBUILD NAMES rpmbuild)
  endif(CMAKE_SYSTEM_NAME MATCHES "Linux" AND NOT CMAKE_CURRENT_BINARY_DIR MATCHES ".* .*")
  # Do not try to build RPM
  if (NOT RPMBUILD)
    set(CPACK_BINARY_RPM OFF)
  endif(NOT RPMBUILD)

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

  IF(CTEST_RUN_CPackComponentsForAll)
    # Check whether if rpmbuild command is found
    # before adding RPM tests
    find_program(RPMBUILD_EXECUTABLE NAMES rpmbuild)
    if(RPMBUILD_EXECUTABLE)
      list(APPEND ACTIVE_CPACK_GENERATORS RPM)
    endif(RPMBUILD_EXECUTABLE)
    # Check whether if dpkg command is found
    # before adding DEB tests
    find_program(DPKG_EXECUTABLE NAMES dpkg)
    if(DPKG_EXECUTABLE)
      list(APPEND ACTIVE_CPACK_GENERATORS DEB)
    endif(DPKG_EXECUTABLE)

    # ACTIVE_CPACK_GENERATORS variable
    # now contains the list of 'active generators'
    set(CPackComponentsForAll_EXTRA_OPTIONS)
    set(CPackRun_CPackCommand "-DCPackCommand=${CMAKE_CPACK_COMMAND}")
    # set up list of CPack generators
    list(APPEND GENLST "ZIP")
    if(APPLE)
      list(APPEND GENLST "DragNDrop")
    endif(APPLE)
    if (NOT CMAKE_CURRENT_BINARY_DIR MATCHES ".* .*")
      list(FIND ACTIVE_CPACK_GENERATORS "RPM" RPM_ACTIVE)
      if (NOT ${RPM_ACTIVE} EQUAL -1)
        list(APPEND GENLST "RPM")
      endif(NOT ${RPM_ACTIVE} EQUAL -1)
    endif(NOT CMAKE_CURRENT_BINARY_DIR MATCHES ".* .*")
    list(FIND ACTIVE_CPACK_GENERATORS "DEB" DEB_ACTIVE)
    if (NOT ${DEB_ACTIVE} EQUAL -1)
      list(APPEND GENLST "DEB")
    endif(NOT ${DEB_ACTIVE} EQUAL -1)

    # set up list of component packaging ways
    list(APPEND CWAYLST "default")
    list(APPEND CWAYLST "OnePackPerGroup")
    list(APPEND CWAYLST "IgnoreGroup")
    list(APPEND CWAYLST "AllInOne")
    foreach(CPackGen ${GENLST})
      set(CPackRun_CPackGen  "-DCPackGen=${CPackGen}")
      foreach(CPackComponentWay ${CWAYLST})
        set(CPackRun_CPackComponentWay "-DCPackComponentWay=${CPackComponentWay}")
        ADD_TEST(CPackComponentsForAll-${CPackGen}-${CPackComponentWay} ${CMAKE_CTEST_COMMAND}
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
        LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CPackComponentsForAll/build${CPackGen}-${CPackComponentWay}")
      endforeach(CPackComponentWay)
    endforeach(CPackGen)
  ENDIF(CTEST_RUN_CPackComponentsForAll)

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
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    --test-command complex)
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/ComplexOneConfig")
  # because of the registry write these tests depend on each other
  SET_TESTS_PROPERTIES ( complex PROPERTIES DEPENDS complexOneConfig)

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

  IF(QT4_WORKS AND QT_QTGUI_FOUND)
    ADD_TEST(QtAutomoc ${CMAKE_CTEST_COMMAND}
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
    LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/QtAutomoc")
  ENDIF()

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
  SET_TESTS_PROPERTIES(ExternalProject PROPERTIES
    TIMEOUT ${CMAKE_LONG_TEST_TIMEOUT})

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
  SET_TESTS_PROPERTIES(testing PROPERTIES PASS_REGULAR_EXPRESSION "Passed")
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

  # Test static linking on toolchains known to support it.
  IF("${CMAKE_C_COMPILER_ID}" MATCHES "^(GNU)$"
      AND NOT APPLE AND NOT WIN32 AND NOT CYGWIN
      AND EXISTS "/usr/lib/libm.a")
    ADD_TEST(LinkStatic  ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/LinkStatic"
      "${CMake_BINARY_DIR}/Tests/LinkStatic"
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --build-project LinkStatic
      --build-options -DMATH_LIBRARY:FILEPATH=/usr/lib/libm.a
      --test-command LinkStatic
      )
  ENDIF()

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
    ADD_TEST_MACRO(ForceInclude foo)
    ADD_TEST_MACRO(PrecompiledHeader foo)
  ENDIF()
  IF(CMAKE_TEST_MSVC OR
      "${CMAKE_TEST_GENERATOR}" MATCHES "(MSYS|MinGW) Makefiles")
    ADD_TEST_MACRO(ModuleDefinition example_exe)
  ENDIF()

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
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-project mfc_driver
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --test-command ${CMAKE_CTEST_COMMAND}
        -C \${CTEST_CONFIGURATION_TYPE} -VV)
    list(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/MFC")
  endif()

  IF(${CMAKE_TEST_GENERATOR} MATCHES "Visual Studio")
    IF(NOT MSVC60)
      ADD_TEST_MACRO(SBCS SBCS)
    ENDIF(NOT MSVC60)

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

    ADD_TEST(VSMidl ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/VSMidl"
      "${CMake_BINARY_DIR}/Tests/VSMidl"
      --build-two-config
      --build-generator ${CMAKE_TEST_GENERATOR}
      --build-project VSMidl
      --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
      --test-command VSMidl)
    LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/VSMidl")
  ENDIF(${CMAKE_TEST_GENERATOR} MATCHES "Visual Studio")

  IF (APPLE)
    if (CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
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
#       --build-target package
        --build-options "-DCMAKE_INSTALL_PREFIX:PATH=${BundleTestInstallDir}"
        "-DCMake_SOURCE_DIR:PATH=${CMake_SOURCE_DIR}"
        --test-command
        ${BundleTestInstallDir}/Applications/SecondBundleExe.app/Contents/MacOS/SecondBundleExe)
      LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/BundleTest")

      ADD_TEST(CFBundleTest ${CMAKE_CTEST_COMMAND}
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
      LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/CFBundleTest")

      ADD_TEST_MACRO(ObjC++ ObjC++)
    ENDIF (CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  ENDIF (APPLE)

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

  ADD_TEST(WarnUnusedUnusedViaSet ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/VariableUnusedViaSet"
    "${CMake_BINARY_DIR}/Tests/WarnUnusedUnusedViaSet"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-noclean
    --build-project WarnUnusedUnusedViaSet
    --build-options "--warn-unused-vars")
  SET_TESTS_PROPERTIES(WarnUnusedUnusedViaSet PROPERTIES
    PASS_REGULAR_EXPRESSION "unused variable \\(changing definition\\) 'UNUSED_VARIABLE'")
  SET_TESTS_PROPERTIES(WarnUnusedUnusedViaSet PROPERTIES
    FAIL_REGULAR_EXPRESSION "unused variable \\(unsetting\\) 'UNUSED_VARIABLE'")
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/WarnUnusedUnusedViaSet")

  ADD_TEST(WarnUnusedUnusedViaUnset ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/VariableUnusedViaUnset"
    "${CMake_BINARY_DIR}/Tests/WarnUnusedUnusedViaUnset"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-noclean
    --build-project WarnUnusedUnusedViaUnset
    --build-options "--warn-unused-vars")
  SET_TESTS_PROPERTIES(WarnUnusedUnusedViaUnset PROPERTIES
    PASS_REGULAR_EXPRESSION "CMake Warning .*:7 \\(set\\):")
  SET_TESTS_PROPERTIES(WarnUnusedUnusedViaUnset PROPERTIES
    FAIL_REGULAR_EXPRESSION "CMake Warning .*:5 \\(set\\):")
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/WarnUnusedUnusedViaUnset")

  ADD_TEST(WarnUnusedCliUnused ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/VariableUsage"
    "${CMake_BINARY_DIR}/Tests/WarnUnusedCliUnused"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-noclean
    --build-project WarnUnusedCliUnused
    --build-options "-DUNUSED_CLI_VARIABLE=Unused")
  SET_TESTS_PROPERTIES(WarnUnusedCliUnused PROPERTIES
    PASS_REGULAR_EXPRESSION "CMake Warning:.*Manually-specified variables were not used by the project:.*  UNUSED_CLI_VARIABLE")
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/WarnUnusedCliUnused")

  ADD_TEST(WarnUnusedCliUsed ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/VariableUsage"
    "${CMake_BINARY_DIR}/Tests/WarnUnusedCliUsed"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-noclean
    --build-project WarnUnusedCliUsed
    --build-options "-DUSED_VARIABLE=Usage proven")
  SET_TESTS_PROPERTIES(WarnUnusedCliUsed PROPERTIES
    PASS_REGULAR_EXPRESSION "Usage proven")
  SET_TESTS_PROPERTIES(WarnUnusedCliUsed PROPERTIES
    FAIL_REGULAR_EXPRESSION "CMake Warning: The variable, 'USED_VARIABLE'")
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/WarnUnusedCliUsed")

  ADD_TEST(WarnUninitialized ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/VariableUsage"
    "${CMake_BINARY_DIR}/Tests/WarnUninitialized"
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --build-noclean
    --build-project WarnUninitialized
    --build-options "--warn-uninitialized")
  SET_TESTS_PROPERTIES(WarnUninitialized PROPERTIES
    PASS_REGULAR_EXPRESSION "uninitialized variable 'USED_VARIABLE'")
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/WarnUninitialized")

  ADD_TEST(TestsWorkingDirectory ${CMAKE_CTEST_COMMAND}
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
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/TestsWorkingDirectory")

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

  CONFIGURE_FILE(
    "${CMake_SOURCE_DIR}/Tests/CTestTestUpload/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestUpload/test.cmake"
    @ONLY ESCAPE_QUOTES)
  ADD_TEST(CTestTestUpload ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestUpload/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestUpload/testOut.log"
    )
  SET_TESTS_PROPERTIES(CTestTestUpload PROPERTIES
    PASS_REGULAR_EXPRESSION "Upload\\.xml")

  CONFIGURE_FILE(
    "${CMake_SOURCE_DIR}/Tests/CTestTestConfigFileInBuildDir/test1.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestConfigFileInBuildDir1/test1.cmake"
    @ONLY ESCAPE_QUOTES)
  ADD_TEST(CTestTestConfigFileInBuildDir1 ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestConfigFileInBuildDir1/test1.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestConfigFileInBuildDir1/testOut1.log"
    )
  REGEX_ESCAPE_STRING(CTEST_TEST_ESCAPED_SOURCE_DIR "${CMake_SOURCE_DIR}")
  SET_TESTS_PROPERTIES(CTestTestConfigFileInBuildDir1 PROPERTIES DEPENDS CTestTestNoBuild
    PASS_REGULAR_EXPRESSION
      "Reading ctest configuration file: ${CTEST_TEST_ESCAPED_SOURCE_DIR}.Tests.CTestTestConfigFileInBuildDir.CTestConfig.cmake")

  CONFIGURE_FILE(
    "${CMake_SOURCE_DIR}/Tests/CTestTestConfigFileInBuildDir/test2.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestConfigFileInBuildDir2/test2.cmake"
    @ONLY ESCAPE_QUOTES)
  CONFIGURE_FILE(
    "${CMake_SOURCE_DIR}/Tests/CTestTestConfigFileInBuildDir/CTestConfig.cmake"
    "${CMake_BINARY_DIR}/Tests/CTestTestConfigFileInBuildDir2/CTestConfig.cmake"
    @ONLY ESCAPE_QUOTES COPYONLY)
  ADD_TEST(CTestTestConfigFileInBuildDir2 ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestConfigFileInBuildDir2/test2.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestConfigFileInBuildDir2/testOut2.log"
    )
  REGEX_ESCAPE_STRING(CTEST_TEST_ESCAPED_BINARY_DIR "${CMake_BINARY_DIR}")
  SET_TESTS_PROPERTIES(CTestTestConfigFileInBuildDir2 PROPERTIES DEPENDS CTestTestNoBuild
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
      PASS_REGULAR_EXPRESSION "(Illegal|SegFault)")
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

  ADD_TEST(CTestTestPrintLabels ${CMAKE_CTEST_COMMAND} --print-labels)
  SET_TESTS_PROPERTIES(CTestTestPrintLabels PROPERTIES LABELS "Label1;Label2")
  SET_TESTS_PROPERTIES(CTestTestPrintLabels PROPERTIES PASS_REGULAR_EXPRESSION
    "All Labels:.*  Label1.*  Label2")

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
    PASS_REGULAR_EXPRESSION "Start 1.*Start 2.*Start 3.*Start 4.*Start 4.*Start 3.*Start 2.*Start 1"
    RESOURCE_LOCK "CostData")

  CONFIGURE_FILE(
    "${CMake_SOURCE_DIR}/Tests/CTestTestCostSerial/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestCostSerial/test.cmake"
    @ONLY ESCAPE_QUOTES)
  ADD_TEST(CTestTestCostSerial ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestCostSerial/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestCostSerial/testOutput.log"
    )
  SET_TESTS_PROPERTIES(CTestTestCostSerial PROPERTIES
    PASS_REGULAR_EXPRESSION "Start 2.*Start 3.*Start 1.*Start 2.*Start 3.*Start 1"
    RESOURCE_LOCK "CostData")

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
    "${CMake_SOURCE_DIR}/Tests/CTestTestZeroTimeout/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestZeroTimeout/test.cmake"
    @ONLY ESCAPE_QUOTES)
  ADD_TEST(CTestTestZeroTimeout ${CMAKE_CTEST_COMMAND}
    -S "${CMake_BINARY_DIR}/Tests/CTestTestZeroTimeout/test.cmake" -V
    --output-log
    "${CMake_BINARY_DIR}/Tests/CTestTestZeroTimeout/testOutput.log")
  SET_TESTS_PROPERTIES(CTestTestZeroTimeout PROPERTIES
    FAIL_REGULAR_EXPRESSION "\\*\\*\\*Timeout")

  CONFIGURE_FILE(
    "${CMake_SOURCE_DIR}/Tests/CTestTestDepends/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestDepends/test.cmake"
    @ONLY ESCAPE_QUOTES)
  ADD_TEST(CTestTestDepends ${CMAKE_CTEST_COMMAND}
    -C "\${CTestTest_CONFIG}"
    -S "${CMake_BINARY_DIR}/Tests/CTestTestDepends/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestDepends/testOutput.log"
    )

  CONFIGURE_FILE(
    "${CMake_SOURCE_DIR}/Tests/CTestTestCycle/test.cmake.in"
    "${CMake_BINARY_DIR}/Tests/CTestTestCycle/test.cmake"
    @ONLY ESCAPE_QUOTES)
  ADD_TEST(CTestTestCycle ${CMAKE_CTEST_COMMAND}
    -C "\${CTestTest_CONFIG}"
    -S "${CMake_BINARY_DIR}/Tests/CTestTestCycle/test.cmake" -V
    --output-log "${CMake_BINARY_DIR}/Tests/CTestTestCycle/testOutput.log"
    )
  SET_TESTS_PROPERTIES(CTestTestCycle PROPERTIES
    PASS_REGULAR_EXPRESSION "a cycle exists in the test dependency graph")

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

    CONFIGURE_FILE("${CMake_SOURCE_DIR}/Tests/CTestTestChecksum/test.cmake.in"
      "${CMake_BINARY_DIR}/Tests/CTestTestChecksum/test.cmake" @ONLY
      ESCAPE_QUOTES)
    ADD_TEST(CTestTestChecksum ${CMAKE_CTEST_COMMAND}
      -S "${CMake_BINARY_DIR}/Tests/CTestTestChecksum/test.cmake" -V
      --output-log
      "${CMake_BINARY_DIR}/Tests/CTestTestChecksum/testOutput.log"
      )
    SET_TESTS_PROPERTIES(CTestTestChecksum PROPERTIES PASS_REGULAR_EXPRESSION
      "Submission failed: Checksum failed for file")

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
  IF(EXISTS "${CMake_BINARY_DIR}/CMakeLists.txt")
    # If there is CMakeLists.txt in the binary tree, assume in-source build
    SET(CMAKE_SKIP_BOOTSTRAP_TEST 1)
  ENDIF(EXISTS "${CMake_BINARY_DIR}/CMakeLists.txt")
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
    ELSE()
      ADD_TEST(FortranC ${CMAKE_CTEST_COMMAND}
        --build-and-test
        "${CMake_SOURCE_DIR}/Tests/FortranC"
        "${CMake_BINARY_DIR}/Tests/FortranC"
        --build-generator ${CMAKE_TEST_GENERATOR}
        --build-project FortranC
        --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
        --build-two-config
        --test-command CMakeFiles/FortranCInterface/FortranCInterface)
      LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/FortranC")
    ENDIF()
  ENDIF()

  find_package(Java COMPONENTS Development QUIET)
  IF(Java_JAVA_EXECUTABLE AND Java_JAVAC_EXECUTABLE AND Java_JAR_EXECUTABLE AND NOT MINGW
      AND NOT "${CMAKE_TEST_GENERATOR}" MATCHES "Xcode")
    GET_FILENAME_COMPONENT(JNIPATH ${JAVA_COMPILE} PATH)
    FIND_FILE(JNI_H jni.h
      "${JNIPATH}/../include"
      "${JNIPATH}/../java/include")
    IF(JNI_H AND EXISTS "${JNI_H}") # in case jni.h is a broken symlink
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
    ENDIF()
  ENDIF()

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

    # If a Linux -> MinGW cross compiler is found then try it
    FIND_PROGRAM(MINGW_CC_LINUX2WIN_EXECUTABLE i586-mingw32msvc-gcc)
    FIND_PROGRAM(MINGW_CXX_LINUX2WIN_EXECUTABLE i586-mingw32msvc-g++)
    FIND_PROGRAM(MINGW_RC_LINUX2WIN_EXECUTABLE i586-mingw32msvc-windres)
    MARK_AS_ADVANCED(MINGW_CC_LINUX2WIN_EXECUTABLE MINGW_CXX_LINUX2WIN_EXECUTABLE MINGW_RC_LINUX2WIN_EXECUTABLE)
    IF(MINGW_CC_LINUX2WIN_EXECUTABLE AND MINGW_CXX_LINUX2WIN_EXECUTABLE AND MINGW_RC_LINUX2WIN_EXECUTABLE)
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
        "-DCMAKE_CXX_COMPILER=${MINGW_CXX_LINUX2WIN_EXECUTABLE}"
        "-DCMAKE_RC_COMPILER=${MINGW_RC_LINUX2WIN_EXECUTABLE}"
        )
      LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/Simple_Mingw_Linux2Win")
    ENDIF()
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
  IF(NOT EXISTS "${CMake_BINARY_DIR}/CMakeLists.txt")
    CONFIGURE_FILE(${CMake_SOURCE_DIR}/Tests/test_clean.cmake.in
                   ${CMake_BINARY_DIR}/Tests/test_clean.cmake @ONLY)
    ADD_CUSTOM_TARGET(test_clean
      COMMAND ${CMAKE_COMMAND} -P ${CMake_BINARY_DIR}/Tests/test_clean.cmake
      COMMENT "Removing test build directories."
      )
  ENDIF(NOT EXISTS "${CMake_BINARY_DIR}/CMakeLists.txt")

  ADD_TEST(CMakeWizardTest ${CMAKE_CMAKE_COMMAND}
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
  # SET(project_RUN_TEST testToRun)
  # The testToRun should be a test executable that can be run to
  # smoke test the build.
  FOREACH(project ${CMAKE_CONTRACT_PROJECTS})
    INCLUDE(Contracts/${project}/RunTest.cmake)
    ADD_TEST_MACRO(Contracts.${project}
      ${${project}_RUN_TEST})
    # Contract test timeout in seconds.
    # Default to 6 hours.
    IF(DEFINED ${project}_TEST_TIMEOUT)
      SET(timeout ${${project}_TEST_TIMEOUT})
    ELSEIF(CMAKE_CONTRACT_TEST_TIMEOUT_DEFAULT)
      SET(timeout ${CMAKE_CONTRACT_TEST_TIMEOUT_DEFAULT})
    ELSE()
      SET(timeout 21600)
    ENDIF()
    SET_TESTS_PROPERTIES(Contracts.${project} PROPERTIES TIMEOUT ${timeout})
  ENDFOREACH()

  IF(TEST_CompileCommandOutput)
    SET(CompileCommandOutput_EXTRA_OPTIONS
      --build-options -DMAKE_SUPPORTS_SPACES=${MAKE_IS_GNU})
    ADD_TEST_MACRO(CompileCommandOutput
      "${CMake_BINARY_DIR}/Tests/CMakeLib/runcompilecommands")
  ENDIF()

  ADD_TEST(IncludeDirectories ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/IncludeDirectories"
    "${CMake_BINARY_DIR}/Tests/IncludeDirectories"
    --build-two-config
    --build-generator ${CMAKE_TEST_GENERATOR}
    --build-project IncludeDirectories
    --build-makeprogram ${CMAKE_TEST_MAKEPROGRAM}
    --test-command IncludeDirectories)
  LIST(APPEND TEST_BUILD_DIRS "${CMake_BINARY_DIR}/Tests/IncludeDirectories")

  IF(CMAKE_USE_KWSTYLE AND KWSTYLE_EXECUTABLE)
    # The "make StyleCheck" command line as a test. If the test fails, look
    # for lines like "Error #0 (624) Line length exceed 88 (max=79)" in the
    # output to find where the style errors are...
    ADD_TEST(KWStyle ${KWSTYLE_EXECUTABLE}
      -xml ${CMake_BINARY_DIR}/CMake.kws.xml
      -o ${CMake_SOURCE_DIR}/Utilities/KWStyle/CMakeOverwrite.txt
      -v
      -D ${CMake_BINARY_DIR}/CMakeKWSFiles.txt
      )
    SET_TESTS_PROPERTIES(KWStyle PROPERTIES
      WORKING_DIRECTORY ${CMake_BINARY_DIR}/Utilities/KWStyle)
  ENDIF()
ENDIF(BUILD_TESTING)

SUBDIRS(CMakeTests)
