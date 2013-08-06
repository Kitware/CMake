# This directory contains tests that run CMake to configure a project
# but do not actually build anything.  To add a test:
#
# 1.) Add a subdirectory named for the test.
#
# 2.) Call add_RunCMake_test and pass the test directory name.
#
# 3.) Create a RunCMakeTest.cmake script in the directory containing
#   include(RunCMake)
#   run_cmake(SubTest1)
#   ...
#   run_cmake(SubTestN)
# where SubTest1..SubTestN are sub-test names each corresponding to
# an independent CMake run and project configuration.
#
# 3.) Create a CMakeLists.txt file in the directory containing
#   cmake_minimum_required(...)
#   project(${RunCMake_TEST} NONE) # or languages needed
#   include(${RunCMake_TEST}.cmake)
# where "${RunCMake_TEST}" is literal.  A value for RunCMake_TEST
# will be passed to CMake by the run_cmake macro when running each
# sub-test.
#
# 4.) Create a <SubTest>.cmake file for each sub-test named above
# containing the actual test code.  Optionally create files
# containing expected test results:
#   <SubTest>-result.txt  = Process result expected if not "0"
#   <SubTest>-stdout.txt  = Regex matching expected stdout content
#   <SubTest>-stderr.txt  = Regex matching expected stderr content
#   <SubTest>-check.cmake = Custom result check
# Note that trailing newlines will be stripped from actual and expected test
# output before matching against the stdout and stderr expressions.
# The code in <SubTest>-check.cmake may use variables
#   RunCMake_TEST_SOURCE_DIR = Top of test source tree
#   RunCMake_TEST_BINARY_DIR = Top of test binary tree
# and an failure must store a message in RunCMake_TEST_FAILED.

macro(add_RunCMake_test test)
  add_test(RunCMake.${test} ${CMAKE_CMAKE_COMMAND}
    -DCMAKE_MODULE_PATH=${CMAKE_CURRENT_SOURCE_DIR}
    -DRunCMake_GENERATOR=${CMAKE_TEST_GENERATOR}
    -DRunCMake_GENERATOR_TOOLSET=${CMAKE_TEST_GENERATOR_TOOLSET}
    -DRunCMake_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/${test}
    -DRunCMake_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}/${test}
    ${${test}_ARGS}
    -P "${CMAKE_CURRENT_SOURCE_DIR}/${test}/RunCMakeTest.cmake"
    )
endmacro()

if(XCODE_VERSION AND "${XCODE_VERSION}" VERSION_LESS 3)
  set(GeneratorToolset_ARGS -DXCODE_BELOW_3=1)
endif()

add_RunCMake_test(CMP0019)
add_RunCMake_test(CMP0022)
add_RunCMake_test(CTest)
if(UNIX AND "${CMAKE_TEST_GENERATOR}" MATCHES "Unix Makefiles")
  add_RunCMake_test(CompilerChange)
endif()
add_RunCMake_test(Configure)
add_RunCMake_test(ExternalData)
add_RunCMake_test(FPHSA)
add_RunCMake_test(GeneratorExpression)
add_RunCMake_test(GeneratorToolset)
add_RunCMake_test(TargetPropertyGeneratorExpressions)
add_RunCMake_test(Languages)
add_RunCMake_test(ObjectLibrary)
if(NOT WIN32)
  add_RunCMake_test(PositionIndependentCode)
  set(SKIP_VISIBILITY 0)
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" AND "${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS 4.2)
    set(SKIP_VISIBILITY 1)
  endif()

  if (CMAKE_CXX_COMPILER_ID MATCHES Watcom
      OR CMAKE_SYSTEM_NAME MATCHES IRIX64
      OR CMAKE_CXX_COMPILER_ID MATCHES HP
      OR CMAKE_CXX_COMPILER_ID MATCHES XL
      OR CMAKE_CXX_COMPILER_ID MATCHES SunPro)
    set(SKIP_VISIBILITY 1)
  endif()

  if (NOT SKIP_VISIBILITY)
    add_RunCMake_test(VisibilityPreset)
  endif()
endif()
add_RunCMake_test(CompatibleInterface)

add_RunCMake_test(add_dependencies)
add_RunCMake_test(build_command)
add_RunCMake_test(find_package)
add_RunCMake_test(get_filename_component)
add_RunCMake_test(if)
add_RunCMake_test(include)
add_RunCMake_test(include_directories)
add_RunCMake_test(list)
add_RunCMake_test(try_compile)
add_RunCMake_test(variable_watch)
add_RunCMake_test(CMP0004)
add_RunCMake_test(TargetPolicies)

find_package(Qt4 QUIET)
find_package(Qt5Core QUIET)
if (QT4_FOUND AND Qt5Core_FOUND AND NOT Qt5Core_VERSION VERSION_LESS 5.1.0)
  add_RunCMake_test(IncompatibleQt)
endif()
if (QT4_FOUND)
  set(ObsoleteQtMacros_ARGS -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE})
  add_RunCMake_test(ObsoleteQtMacros)
endif()

if("${CMAKE_TEST_GENERATOR}" MATCHES "Visual Studio [^6]")
  add_RunCMake_test(include_external_msproject)
  add_RunCMake_test(SolutionGlobalSections)
endif()

add_RunCMake_test(File_Generate)
add_RunCMake_test(ExportWithoutLanguage)
add_RunCMake_test(target_link_libraries)
add_RunCMake_test(WatchTwice)
