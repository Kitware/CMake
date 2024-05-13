cmake_policy(SET CMP0157 NEW)
enable_language (Swift)
include(CheckCompilerFlag)

set(Swift 1)

# test that the check uses an isolated locale
set(_env_LC_ALL "${LC_ALL}")
set(ENV{LC_ALL} "BAD")

check_compiler_flag(Swift "-foo-as-blarpy" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "invalid Swift compile flag didn't fail.")
endif()

check_compiler_flag(Swift "-Onone" SHOULD_WORK)
if(NOT SHOULD_WORK)
  message(SEND_ERROR "Swift compiler flag '-Onone' check failed")
endif()

# Reset locale
set(ENV{LC_ALL} ${_env_LC_ALL})
