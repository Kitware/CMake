cmake_minimum_required(VERSION 3.1...3.20)

project(TestBasicPerl CXX)

include(CTest)

if(NOT DEFINED language)
  set(language "perl")
endif()

include (../BasicConfiguration.cmake)

add_test (NAME BasicPerl
  COMMAND "${PERL_EXECUTABLE}" "-I${CMAKE_CURRENT_BINARY_DIR}"
                               "-I$<TARGET_FILE_DIR:example>"
  "${CMAKE_CURRENT_SOURCE_DIR}/../runme.pl")
