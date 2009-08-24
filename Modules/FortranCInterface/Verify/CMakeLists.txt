cmake_minimum_required(VERSION 2.7)
project(VerifyFortranC C Fortran)

option(VERIFY_CXX "Whether to verify C++ and Fortran" OFF)
if(VERIFY_CXX)
  enable_language(CXX)
  set(VerifyCXX VerifyCXX.cxx)
  add_definitions(-DVERIFY_CXX)
endif()

include(FortranCInterface)

FortranCInterface_HEADER(VerifyFortran.h SYMBOLS VerifyFortran)
include_directories(${VerifyFortranC_BINARY_DIR})

add_executable(VerifyFortranC main.c VerifyC.c VerifyFortran.f ${VerifyCXX})
