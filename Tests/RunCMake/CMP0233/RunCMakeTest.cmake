include(RunCMake)

# The policy only affects the MSVC and clang-cl compilers, which the test is
# only added for.  Visual Studio 2022 17.2 (19.32) and clang-cl 19.1 are the
# first to have /Zc:__STDC__.
if(CMAKE_C_COMPILER_ID STREQUAL "MSVC" AND CMAKE_C_COMPILER_VERSION VERSION_LESS 19.32)
  return()
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang" AND CMAKE_C_COMPILER_VERSION VERSION_LESS 19.1)
  return()
endif()

# Configure and build a target with the given CMP0233 default, C_STANDARD
# and C_EXTENSIONS.  The source is compiled for the mode that the compiler
# is expected to end up in, and fails to compile in the other one.
#   <policy_value>  OLD, NEW, or empty to leave the policy unset
#   <std>           C_STANDARD value
#   <ext>           C_EXTENSIONS value
#   <mode>          expected compiler mode: STRICT or EXTENSIONS
# Extra arguments are passed to the configure step.
function(run_cmp0233 name policy_value std ext mode)
  set(RunCMake_TEST_OPTIONS
    -DCMP0233_POLICY=${policy_value}
    -DCMP0233_STANDARD=${std}
    -DCMP0233_EXTENSIONS=${ext}
    -DCMP0233_EXPECT=${mode}
    ${ARGN})
  if(policy_value)
    list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_POLICY_DEFAULT_CMP0233=${policy_value})
  endif()
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${name}-build")
  run_cmake(${name})

  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(${name}-build ${CMAKE_COMMAND} --build . --config Debug)
endfunction()

# OLD and the unset (WARN) policy keep the extensions.
run_cmp0233(CMP0233-WARN-ext-off-std11 "" 11 OFF EXTENSIONS)
run_cmp0233(CMP0233-OLD-ext-off-std11 "OLD" 11 OFF EXTENSIONS)
run_cmp0233(CMP0233-NEW-ext-off-std11 "NEW" 11 OFF STRICT)
run_cmp0233(CMP0233-NEW-ext-on-std11 "NEW" 11 ON EXTENSIONS)

run_cmp0233(CMP0233-OLD-ext-off-std17 "OLD" 17 OFF EXTENSIONS)
run_cmp0233(CMP0233-NEW-ext-off-std17 "NEW" 17 OFF STRICT)
run_cmp0233(CMP0233-NEW-ext-on-std17 "NEW" 17 ON EXTENSIONS)

# C23 is available from Visual Studio 2022 17.9 (19.39), but the Visual
# Studio generators need Visual Studio 2022 17.11 for it, and from
# clang-cl 18.0.  The compiler and generator that built CMake tell.
if(CMAKE_C_STANDARD_LATEST GREATER_EQUAL 23)
  set(has_c23 TRUE)
else()
  set(has_c23 FALSE)
endif()
if(has_c23)
  run_cmp0233(CMP0233-OLD-ext-off-std23 "OLD" 23 OFF EXTENSIONS)
  run_cmp0233(CMP0233-NEW-ext-off-std23 "NEW" 23 OFF STRICT)
  run_cmp0233(CMP0233-NEW-ext-on-std23 "NEW" 23 ON EXTENSIONS)
endif()

# The flag is only passed for the standards that support it.
run_cmp0233(CMP0233-NEW-ext-off-std99 "NEW" 99 OFF EXTENSIONS)

# The policy is per target: a change after a target is created does not
# affect that target, only the ones created later.
run_cmp0233(CMP0233-per-target-NEW-then-OLD "NEW" 17 OFF STRICT
  -DCMP0233_LATE_POLICY=OLD -DCMP0233_LATE_EXPECT=EXTENSIONS)
run_cmp0233(CMP0233-per-target-OLD-then-NEW "OLD" 17 OFF EXTENSIONS
  -DCMP0233_LATE_POLICY=NEW -DCMP0233_LATE_EXPECT=STRICT)

# The C++ sources of a target with C sources get the flag too, as with the
# Visual Studio generators, which compile both with one set of options.
set(RunCMake-check-file check-cxx-flag.cmake)
run_cmp0233(CMP0233-NEW-mixed-ext-off-std11 "NEW" 11 OFF STRICT
  -DCMP0233_EXTRA_SOURCES=other.cpp)
run_cmp0233(CMP0233-NEW-mixed-ext-on-std11 "NEW" 11 ON EXTENSIONS
  -DCMP0233_EXTRA_SOURCES=other.cpp)
run_cmp0233(CMP0233-OLD-mixed-ext-off-std11 "OLD" 11 OFF EXTENSIONS
  -DCMP0233_EXTRA_SOURCES=other.cpp)
if(has_c23)
  # The Visual Studio generators map C23 to LanguageStandard_C even in a
  # .vcxproj file that also has C++ sources.
  run_cmp0233(CMP0233-NEW-mixed-ext-off-std23 "NEW" 23 OFF STRICT
    -DCMP0233_EXTRA_SOURCES=other.cpp)
  run_cmp0233(CMP0233-NEW-mixed-ext-on-std23 "NEW" 23 ON EXTENSIONS
    -DCMP0233_EXTRA_SOURCES=other.cpp)
endif()
# C99 has no /Zc:__STDC__ support, so neither the C nor the C++ sources
# of a mixed target get the flag.
run_cmp0233(CMP0233-NEW-mixed-ext-off-std99 "NEW" 99 OFF EXTENSIONS
  -DCMP0233_EXTRA_SOURCES=other.cpp)
unset(RunCMake-check-file)
