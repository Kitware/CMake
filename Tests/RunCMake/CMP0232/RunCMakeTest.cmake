include(RunCMake)

# The policy only affects the MSVC compiler, which the test is only added for.
# Visual Studio 2017 15.7 (19.14) is the first to have /Zc:__cplusplus, so
# older compilers keep the legacy value even under the NEW behavior.
if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.14)
  set(new_mode UPDATED)
else()
  set(new_mode LEGACY)
endif()

# Configure and build a target with the given CMP0232 default and
# CXX_STANDARD.  The source compiles only if __cplusplus has the value
# that the compiler is expected to report.
#   <policy>  OLD, NEW, or empty to leave the policy unset
#   <std>     CXX_STANDARD value, or empty for the compiler default
#   <mode>    expected __cplusplus value: UPDATED or LEGACY
# Extra arguments are passed to the configure step.
function(run_cmp0232 name policy std mode)
  if(policy STREQUAL "NEW")
    set(try_compile_mode ${new_mode})
  else()
    set(try_compile_mode LEGACY)
  endif()
  set(RunCMake_TEST_OPTIONS
    -DCMP0232_STANDARD=${std}
    -DCMP0232_EXPECT=${mode}
    -DCMP0232_TRY_COMPILE_EXPECT=${try_compile_mode}
    ${ARGN})
  if(policy)
    list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_POLICY_DEFAULT_CMP0232=${policy})
  endif()
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${name}-build")
  run_cmake(${name})

  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${name}-build ${CMAKE_COMMAND} --build . --config Debug)
endfunction()

# OLD and the unset (WARN) policy keep the legacy value.
run_cmp0232(CMP0232-WARN "" "" LEGACY)
run_cmp0232(CMP0232-OLD "OLD" "" LEGACY)
run_cmp0232(CMP0232-NEW "NEW" "" ${new_mode})

# The flag does not depend on CMAKE_CXX_FLAGS.
run_cmp0232(CMP0232-NEW-custom-flags "NEW" "" ${new_mode} "-DCMAKE_CXX_FLAGS=/EHsc")

# A target can still opt out through its compile options.
if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.14)
  run_cmp0232(CMP0232-NEW-opt-out "NEW" "" LEGACY -DCMP0232_OPT_OUT=1)
endif()

# Visual Studio 2017 15.3 (19.11) is the first to have /std:c++17.
if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.11.25505)
  run_cmp0232(CMP0232-OLD-std17 "OLD" 17 LEGACY)
  run_cmp0232(CMP0232-NEW-std17 "NEW" 17 ${new_mode})
endif()
