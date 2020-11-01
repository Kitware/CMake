cmake_minimum_required(VERSION 3.15)
project(objctest LANGUAGES C OBJC)

include(CheckOBJCCompilerFlag)
check_objc_compiler_flag(-fobjc-arc HAVE_OBJC_ARC)

if(HAVE_OBJC_ARC)
  add_compile_options(-fobjc-arc)
  add_compile_definitions(HAVE_OBJC_ARC)
endif()

add_library(myfuncs STATIC myfuncs.m)
