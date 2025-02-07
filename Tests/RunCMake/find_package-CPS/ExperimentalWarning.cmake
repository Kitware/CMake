cmake_minimum_required(VERSION 4.0)

set(
  CMAKE_EXPERIMENTAL_FIND_CPS_PACKAGES
  "e82e467b-f997-4464-8ace-b00808fff261"
  )

include(Setup.cmake)

find_package(DoesNotExist)
