cmake_minimum_required(VERSION 2.8.12)

project(CheckOBJCXXCompilerFlag)

include(CheckOBJCXXCompilerFlag)

if(CMAKE_COMPILER_IS_GNUOBJCXX)
  set(COMPILER_FLAG -fobjc-direct-dispatch)
else()
  set(COMPILER_FLAG -fobjc-gc)
endif()

CHECK_OBJCXX_COMPILER_FLAGS(${COMPILER_FLAG} HAS_COMPILER_FLAG)

if(NOT HAS_COMPILER_FLAG)
  message(SEND_ERROR "Test fail: HAS_COMPILER_FLAG: ${COMPILER_FLAG}")
endif()
