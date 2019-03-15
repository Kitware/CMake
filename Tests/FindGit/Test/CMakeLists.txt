cmake_minimum_required(VERSION 3.12)
project(TestFindGit NONE)
include(CTest)

find_package(Git REQUIRED)

add_test(NAME test_git
  COMMAND ${CMAKE_COMMAND}
  "-DGIT_EXECUTABLE=${GIT_EXECUTABLE}"
  "-DGIT_EXECUTABLE_TARGET=$<TARGET_FILE:Git::Git>"
  "-DGIT_VERSION_STRING=${GIT_VERSION_STRING}"
  -P "${CMAKE_CURRENT_LIST_DIR}/RunGit.cmake"
  )
