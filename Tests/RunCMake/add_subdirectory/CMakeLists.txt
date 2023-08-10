cmake_minimum_required(VERSION 3.5)

# Have to set policy here due to policy scope
if(DEFINED CMP0082_VALUE)
  cmake_policy(SET CMP0082 "${CMP0082_VALUE}")
endif()
set(CMAKE_POLICY_WARNING_CMP0082 ON)

project(${RunCMake_TEST} NONE)
include(${RunCMake_TEST}.cmake)
