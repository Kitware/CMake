cmake_minimum_required(VERSION 3.1)

project(TestBasicPerl CXX)

include(CTest)

set(language "perl")

include (../BasicConfiguration.cmake)

if (WIN32)
  file (TO_CMAKE_PATH "$ENV{PATH}" perl_path)
  string (REPLACE ";" "$<SEMICOLON>" perl_path "${perl_path}")
  set (perl_env "PATH=$<TARGET_FILE_DIR:example>$<SEMICOLON>${perl_path}")
else()
  set (perl_env "LD_LIBRARY_PATH=$<TARGET_FILE_DIR:example>")
endif()

add_test (NAME BasicPerl
  COMMAND "${CMAKE_COMMAND}" -E env "${perl_env}"
  "${PERL_EXECUTABLE}" "-I$<TARGET_FILE_DIR:example>"
  "${CMAKE_CURRENT_SOURCE_DIR}/../runme.pl")
