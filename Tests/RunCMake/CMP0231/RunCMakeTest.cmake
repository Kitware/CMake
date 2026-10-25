include(RunCMake)

# The policy only affects the MSVC compiler, which the test is only added for.
# Compilers older than Visual Studio 2017 (19.10) have no permissive flags.
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.10)
  return()
endif()

# Configure and build a target with the given CMP0231 default, CXX_STANDARD
# and CXX_EXTENSIONS.  The source is compiled for the mode that the compiler
# is expected to end up in, and fails to compile in the other one.
#   <policy>  OLD, NEW, or empty to leave the policy unset
#   <std>     CXX_STANDARD value, or empty for the compiler default
#   <ext>     CXX_EXTENSIONS value
#   <mode>    expected compiler mode: PERMISSIVE or NON_PERMISSIVE
function(run_cmp0231 name policy std ext mode)
  set(RunCMake_TEST_OPTIONS
    -DCMP0231_STANDARD=${std}
    -DCMP0231_EXTENSIONS=${ext}
    -DCMP0231_EXPECT=${mode})
  if(policy)
    list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_POLICY_DEFAULT_CMP0231=${policy})
  endif()
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${name}-build")
  run_cmake(${name})

  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${name}-build ${CMAKE_COMMAND} --build . --config Debug)
endfunction()

# Compiler default standard (C++14 for MSVC).  CMP0128 leaves out its /std:
# flag, so /permissive- has to be added on its own.
if(CMAKE_CXX_STANDARD_DEFAULT)
  run_cmp0231(CMP0231-NEW-ext-off-default "NEW" ${CMAKE_CXX_STANDARD_DEFAULT} OFF NON_PERMISSIVE)
endif()

# C++14 and C++17 stay permissive until /permissive- is passed.
# OLD and the unset (WARN) policy keep the default mode of the compiler.
run_cmp0231(CMP0231-OLD-ext-off-std14 "OLD" 14 OFF PERMISSIVE)
run_cmp0231(CMP0231-NEW-ext-off-std14 "NEW" 14 OFF NON_PERMISSIVE)
run_cmp0231(CMP0231-NEW-ext-on-std14 "NEW" 14 ON PERMISSIVE)

run_cmp0231(CMP0231-OLD-ext-off-std17 "OLD" 17 OFF PERMISSIVE)
run_cmp0231(CMP0231-WARN-ext-off-std17 "" 17 OFF PERMISSIVE)
run_cmp0231(CMP0231-NEW-ext-off-std17 "NEW" 17 OFF NON_PERMISSIVE)
run_cmp0231(CMP0231-NEW-ext-on-std17 "NEW" 17 ON PERMISSIVE)

# Visual Studio 2017 15.5 (19.12) is the first to have a C++20 option.
# /std:c++latest implies /permissive- since Visual Studio 2019 16.8 (19.28).
if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.12)
  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.28)
    set(std20_default_mode NON_PERMISSIVE)
  else()
    set(std20_default_mode PERMISSIVE)
  endif()
  run_cmp0231(CMP0231-OLD-ext-on-std20 "OLD" 20 ON ${std20_default_mode})
  run_cmp0231(CMP0231-NEW-ext-off-std20 "NEW" 20 OFF NON_PERMISSIVE)
  run_cmp0231(CMP0231-NEW-ext-on-std20 "NEW" 20 ON PERMISSIVE)
endif()

# C++23 uses /std:c++latest, which implies /permissive-.
if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.29.30129)
  run_cmp0231(CMP0231-OLD-ext-on-std23 "OLD" 23 ON NON_PERMISSIVE)
  run_cmp0231(CMP0231-NEW-ext-off-std23 "NEW" 23 OFF NON_PERMISSIVE)
  run_cmp0231(CMP0231-NEW-ext-on-std23 "NEW" 23 ON PERMISSIVE)
endif()
