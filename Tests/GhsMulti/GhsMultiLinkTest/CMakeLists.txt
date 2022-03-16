# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(test C)

message("Copy project")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt.in
  ${CMAKE_CURRENT_BINARY_DIR}/link_src/CMakeLists.txt COPYONLY)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/exe1.c
  ${CMAKE_CURRENT_SOURCE_DIR}/exe1.h
  ${CMAKE_CURRENT_SOURCE_DIR}/func2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/func3.c
  ${CMAKE_CURRENT_SOURCE_DIR}/func4.c
  ${CMAKE_CURRENT_SOURCE_DIR}/func5.c
  ${CMAKE_CURRENT_SOURCE_DIR}/func6.c
  ${CMAKE_CURRENT_SOURCE_DIR}/func7.c
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/link_src
)

message("Building project")
try_compile(RESULT
  ${CMAKE_CURRENT_BINARY_DIR}/link_build
  ${CMAKE_CURRENT_BINARY_DIR}/link_src
  test
  CMAKE_FLAGS
    -DRUN_TEST=${RUN_TEST}
    -DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}
  OUTPUT_VARIABLE OUTPUT)

message("Output from build:\n${OUTPUT}")
if (RUN_TEST STREQUAL "NO_FLAGS")
  if(NOT RESULT)
    message(SEND_ERROR "Could not build test project (1)!")
  endif()
else()
  unset(fileName CACHE)
  find_file(fileName exe1.gpj
    ${CMAKE_CURRENT_BINARY_DIR}/link_build/exe1.dir
    )
  message("Parsing project file: ${fileName}")
  file(STRINGS ${fileName} fileText)
  set(expected_flags
    -add-link-options1 -add-link-options2
    link_directories_used1 link_directories_used2 "c:/absolute"
    link_libraries_used1 link_libraries_used2
    -lcsl1 csl2
    -clinkexe1 -clinkexe2
    -special-lib2-public-link)
  foreach(opt IN LISTS expected_flags)
    string(FIND "${fileText}" "${opt}" opt_found)
    if ( opt_found EQUAL -1 )
      message(SEND_ERROR "Could not find: ${opt}")
    else()
      message("located: ${opt}")
    endif()
  endforeach()

  unset(fileName CACHE)
  find_file (fileName lib1.gpj
    ${CMAKE_CURRENT_BINARY_DIR}/link_build/lib1.dir
    )
  message("Parsing project file: ${fileName}")
  file(STRINGS ${fileName} fileText)
  set(expected_flags
    -clinkexeA1 -clinkexeA2
    -static-lib-flags1 -static-lib-flags2)
  foreach(opt IN LISTS expected_flags)
    string(FIND "${fileText}" "${opt}" opt_found)
    if (opt_found EQUAL -1)
      message(SEND_ERROR "Could not find: ${opt}")
    else()
      message("located: ${opt}")
    endif()
  endforeach()

  unset(fileName CACHE)
  find_file (fileName lib2.gpj
    ${CMAKE_CURRENT_BINARY_DIR}/link_build/lib2.dir
    )
  message("Parsing project file: ${fileName}")
  file(STRINGS ${fileName} fileText)
  set(expected_flags
    -clinkexeA1 -clinkexeA2)
  foreach(opt IN LISTS expected_flags)
    string(FIND "${fileText}" "${opt}" opt_found)
    if ( opt_found EQUAL -1 )
      message(SEND_ERROR "Could not find: ${opt}")
    else()
      message("located: ${opt}")
    endif()
  endforeach()
endif()
