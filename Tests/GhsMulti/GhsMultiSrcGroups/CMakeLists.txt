# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(test C)

#set(CMAKE_FOLDER ON)
add_executable(groups
  test1.c
  test1.h
  test2a.c
  test4.c
  test5.c
  test6.c
  test7.c
  standard.h
  testOBJ.c
  testOBJ.h
  sub/testOBJ.c
  sub/testOBJ.h
  textfile.txt
  textfile2.txt
  test3.c
  Atest3.c
#  object.o
  resource.pdf
  cmake.rule
  s5.h
  s2.h
  s4.h
  standard.h
  )

if(TEST_PROP)
  set_target_properties(groups PROPERTIES GHS_NO_SOURCE_GROUP_FILE ON)
endif()
if(CMAKE_C_COMPILER_ID MATCHES "GHS")
  target_link_options(groups PRIVATE "-non_shared")
endif()
source_group( gC FILES sub/testOBJ.h testOBJ.c testOBJ.h sub/testOBJ.c )
source_group( gA FILES test1.c test1.h)
source_group( gB test[65].c )
source_group( gC\\gD FILES test7.c )
source_group( docs FILES textfile.txt )
