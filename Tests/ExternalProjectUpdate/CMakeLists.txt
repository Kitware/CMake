cmake_minimum_required(VERSION 2.8.12)
project(ExternalProjectUpdateTest NONE)
if(CMAKE_XCODE_BUILD_SYSTEM VERSION_GREATER_EQUAL 12)
  cmake_policy(SET CMP0114 NEW)
else()
  # This test is very noisy with warnings about this policy if we don't
  # explicitly set it. Projects shouldn't do this, but for test code this
  # is reasonable.
  cmake_policy(SET CMP0114 OLD)
endif()
cmake_policy(GET CMP0114 cmp0114)

include(ExternalProject)

find_package(Git)

option(ExternalProjectUpdateTest_USE_FOLDERS "Enable folder grouping in IDEs." ON)
if(ExternalProjectUpdateTest_USE_FOLDERS)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
else()
  set_property(GLOBAL PROPERTY USE_FOLDERS OFF)
endif()

set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER
  "CMakePredefinedTargets-in-ExternalProjectUpdateTest")

set(base "${CMAKE_BINARY_DIR}/CMakeExternals")
set(binary_base "${base}/Build")
set_property(DIRECTORY PROPERTY EP_BASE ${base})
if(cmp0114 STREQUAL "NEW")
  set_property(DIRECTORY PROPERTY EP_STEP_TARGETS configure build test update)
  set(TestUpdateCommand_STEP_TARGETS STEP_TARGETS update)
  set(TestUpdateCommand_INDEPENDENT_STEP_TARGETS)
else()
  set_property(DIRECTORY PROPERTY EP_STEP_TARGETS configure build test)
  set_property(DIRECTORY PROPERTY EP_INDEPENDENT_STEP_TARGETS update)
  set(TestUpdateCommand_STEP_TARGETS)
  set(TestUpdateCommand_INDEPENDENT_STEP_TARGETS INDEPENDENT_STEP_TARGETS update)
endif()

ExternalProject_Add(TestUpdateCommand
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  UPDATE_COMMAND ${CMAKE_COMMAND} -E echo update
  UPDATE_DISCONNECTED 1
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  ${TestUpdateCommand_STEP_TARGETS}
  ${TestUpdateCommand_INDEPENDENT_STEP_TARGETS}
  )
add_custom_target(TestUpdateCommandDriver ALL)
add_dependencies(TestUpdateCommandDriver TestUpdateCommand-update)

set(do_git_tests 0)

if(GIT_EXECUTABLE)
  set(do_git_tests 1)

  message(STATUS "GIT_VERSION_STRING='${GIT_VERSION_STRING}'")

  if("${GIT_VERSION_STRING}" VERSION_LESS 1.6.5)
    message(STATUS "No ExternalProject git tests with git client less than version 1.6.5")
    set(do_git_tests 0)
  endif()
endif()

# This should be specified from the command line.
if(NOT TEST_GIT_TAG)
  set(TEST_GIT_TAG origin/master)
endif()

if(do_git_tests)
  set(local_git_repo "../../LocalRepositories/GIT")

  # Unzip/untar the git repository in our source folder so that other
  # projects below may use it to test git args of ExternalProject_Add
  #
  set(proj SetupLocalGITRepository)
  ExternalProject_Add(${proj}
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/LocalRepositories/GIT
    URL ${CMAKE_CURRENT_SOURCE_DIR}/gitrepo.tgz
    BUILD_COMMAND ""
    CONFIGURE_COMMAND "${GIT_EXECUTABLE}" --version
    INSTALL_COMMAND ""
  )
  set_property(TARGET ${proj}
    PROPERTY FOLDER "SetupRepos/Local/Deeply/Nested/For/Testing")

  set(proj TutorialStep1-GIT)
  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${local_git_repo}"
    GIT_TAG ${TEST_GIT_TAG}
    GIT_CONFIG "user.email=testauthor@cmake.org"
               "user.name=testauthor"
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    INSTALL_COMMAND ""
  )
  ExternalProject_Add_StepDependencies(${proj} download SetupLocalGITRepository)
  set_property(TARGET ${proj} PROPERTY FOLDER "GIT")

  set(proj TutorialStep2-GIT)
  ExternalProject_Add(${proj}
    GIT_REPOSITORY "${local_git_repo}"
    GIT_TAG ${TEST_GIT_TAG}
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    INSTALL_COMMAND ""
    UPDATE_DISCONNECTED 1
  )
  ExternalProject_Add_StepDependencies(${proj} download SetupLocalGITRepository)
  set_property(TARGET ${proj} PROPERTY FOLDER "GIT")
endif()


# Test the testable built/installed products:
#
enable_testing()


# Do at least a smoke test of a built executable from each
# project's build directory...
#
# BuildTree tests:
#

if(do_git_tests)
  add_test(TutorialStep1-GIT
    "${binary_base}/TutorialStep1-GIT/Tutorial" 81)
endif()

message(STATUS "do_git_tests='${do_git_tests}'")
message(STATUS "GIT_EXECUTABLE='${GIT_EXECUTABLE}'")
