cmake_minimum_required(VERSION 3.19)  # Must be older than 3.20 in order to also not set CMP0118!
cmake_policy(SET CMP0115 NEW)  # CMP0115 is correct here (for reducing error-output)!
project(${RunCMake_TEST} NONE)
include(${RunCMake_TEST}.cmake NO_POLICY_SCOPE)
