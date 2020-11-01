cmake_minimum_required(VERSION 3.12...3.19)

project(TestSwigSrcOUTPUT_DIR CXX)

include(CTest)


set(CMAKE_CXX_STANDARD 11)


find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

set(UseSWIG_MODULE_VERSION 2)


file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/Foo"
                    "${CMAKE_CURRENT_BINARY_DIR}/FooSupport"
                    "${CMAKE_CURRENT_BINARY_DIR}/FooFile"
                    "${CMAKE_CURRENT_BINARY_DIR}/Bar"
                    "${CMAKE_CURRENT_BINARY_DIR}/BarSupport"
                    "${CMAKE_CURRENT_BINARY_DIR}/BarFile")


set_property (SOURCE foo.i PROPERTY COMPILE_OPTIONS -namespace Foo)
set_property (SOURCE foo.i PROPERTY OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/Foo")
set_property (SOURCE foo.i PROPERTY CPLUSPLUS ON)

set_property (SOURCE bar.i PROPERTY COMPILE_OPTIONS -namespace Bar)
set_property (SOURCE bar.i PROPERTY OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/Bar")
set_property (SOURCE bar.i PROPERTY CPLUSPLUS ON)

swig_add_library(outdir_test LANGUAGE csharp SOURCES foo.i bar.i cs.cpp
                             OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/NotUsed1"
                             OUTFILE_DIR "${CMAKE_CURRENT_BINARY_DIR}/NotUsed2")
target_include_directories(outdir_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
set_property(TARGET outdir_test PROPERTY USE_TARGET_INCLUDE_DIRECTORIES ON)


get_target_property(SUPPORT_FILES_DIRS outdir_test SWIG_SUPPORT_FILES_DIRECTORY)

add_test(NAME SwigSrcOUTPUT_DIR COMMAND "${CMAKE_COMMAND}" "-DBASE_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}" "-DSUPPORT_FILES_DIRECTORY=${SUPPORT_FILES_DIRS}" -P "${CMAKE_CURRENT_SOURCE_DIR}/ValidateSupportFiles.cmake")



set_property (SOURCE foo.i PROPERTY OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/FooSupport")
set_property (SOURCE foo.i PROPERTY OUTFILE_DIR "${CMAKE_CURRENT_BINARY_DIR}/FooFile")

set_property (SOURCE bar.i PROPERTY OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/BarSupport")
set_property (SOURCE bar.i PROPERTY OUTFILE_DIR "${CMAKE_CURRENT_BINARY_DIR}/BarFile")

swig_add_library(outfiledir_test LANGUAGE csharp SOURCES foo.i bar.i cs.cpp
                                 OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/NotUsed1"
                                 OUTFILE_DIR "${CMAKE_CURRENT_BINARY_DIR}/NotUsed2")
target_include_directories(outfiledir_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
set_property(TARGET outfiledir_test PROPERTY USE_TARGET_INCLUDE_DIRECTORIES ON)


get_target_property(SUPPORT_FILES_DIRS outfiledir_test SWIG_SUPPORT_FILES_DIRECTORY)

add_test(NAME SwigSrcOUTFILE_DIR COMMAND "${CMAKE_COMMAND}" "-DBASE_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}" "-DSUPPORT_FILES_DIRECTORY=${SUPPORT_FILES_DIRS}" -DOUTFILE_DIR=ON -P "${CMAKE_CURRENT_SOURCE_DIR}/ValidateSupportFiles.cmake")
