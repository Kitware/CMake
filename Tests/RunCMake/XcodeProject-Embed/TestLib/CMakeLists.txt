cmake_minimum_required(VERSION 3.30)
project(TestLib C)
add_library(TestLib SHARED TestLib.c TestLib.h)
set_target_properties(TestLib PROPERTIES
  FRAMEWORK 1
  PUBLIC_HEADER TestLib.h
  )
