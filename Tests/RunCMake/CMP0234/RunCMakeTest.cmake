include(RunCMake)

# The policy only affects the MSVC compiler.  Visual Studio 2019 16.5 (19.25)
# is the first to have /Zc:preprocessor.
if(CMAKE_C_COMPILER_VERSION VERSION_LESS 19.25 OR CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.25)
  return()
endif()

# Configure and build a mixed C and CXX target.
#   <policy_value>  OLD, NEW, or empty to leave the policy unset
#   <c_ext>, <cxx_ext>  C_EXTENSIONS and CXX_EXTENSIONS values
#   <c_mode>, <cxx_mode>  expected preprocessor: STRICT or EXTENSIONS
function(run_cmp0234 name policy_value c_ext cxx_ext c_mode cxx_mode)
  set(RunCMake_TEST_OPTIONS
    -DCMP0234_POLICY=${policy_value}
    -DCMP0234_C_EXTENSIONS=${c_ext}
    -DCMP0234_CXX_EXTENSIONS=${cxx_ext}
    -DCMP0234_EXPECT_C=${c_mode}
    -DCMP0234_EXPECT_CXX=${cxx_mode}
    ${ARGN})
  if(policy_value)
    list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_POLICY_DEFAULT_CMP0234=${policy_value})
  endif()
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${name}-build")
  run_cmake(${name})

  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(${name}-build ${CMAKE_COMMAND} --build . --config Debug)
endfunction()

# OLD and the unset (WARN) policy keep the traditional preprocessor.
run_cmp0234(CMP0234-WARN-both-off "" OFF OFF EXTENSIONS EXTENSIONS)
run_cmp0234(CMP0234-OLD-both-off "OLD" OFF OFF EXTENSIONS EXTENSIONS)

# NEW: extensions off for a language passes the flag for it, and a mixed
# target passes it for both languages if either disables extensions.
run_cmp0234(CMP0234-NEW-both-on "NEW" ON ON EXTENSIONS EXTENSIONS)
run_cmp0234(CMP0234-NEW-both-off "NEW" OFF OFF STRICT STRICT)
run_cmp0234(CMP0234-NEW-c-off "NEW" OFF ON STRICT STRICT)
run_cmp0234(CMP0234-NEW-cxx-off "NEW" ON OFF STRICT STRICT)

# The policy is per target.
run_cmp0234(CMP0234-per-target-NEW-then-OLD "NEW" OFF OFF STRICT STRICT
  -DCMP0234_LATE_POLICY=OLD -DCMP0234_LATE_EXPECT=EXTENSIONS)
run_cmp0234(CMP0234-per-target-OLD-then-NEW "OLD" OFF OFF EXTENSIONS EXTENSIONS
  -DCMP0234_LATE_POLICY=NEW -DCMP0234_LATE_EXPECT=STRICT)

# Standalone C and CXX projects depend only on their own language.
run_cmp0234(CMP0234-OLD-c-only-off "OLD" OFF ON EXTENSIONS EXTENSIONS
  -DCMP0234_LANGS=C)
run_cmp0234(CMP0234-NEW-c-only-off "NEW" OFF ON STRICT STRICT
  -DCMP0234_LANGS=C)
run_cmp0234(CMP0234-NEW-c-only-on "NEW" ON OFF EXTENSIONS EXTENSIONS
  -DCMP0234_LANGS=C)
run_cmp0234(CMP0234-OLD-cxx-only-off "OLD" ON OFF EXTENSIONS EXTENSIONS
  -DCMP0234_LANGS=CXX)
run_cmp0234(CMP0234-NEW-cxx-only-off "NEW" ON OFF STRICT STRICT
  -DCMP0234_LANGS=CXX)
run_cmp0234(CMP0234-NEW-cxx-only-on "NEW" OFF ON EXTENSIONS EXTENSIONS
  -DCMP0234_LANGS=CXX)

# Standard matrix.  The C standard options /std:c11 and later, available from
# Visual Studio 2019 16.8 (19.28), imply the conformant preprocessor for C
# sources, and the C++ standard options never do.  With the NEW behavior, the
# conformant preprocessor is used for both languages if extensions are off
# for either, and otherwise C_EXTENSIONS ON restores the traditional one with
# /Zc:preprocessor-.  All cases are targets of one project (Std/CMakeLists.txt)
# so that the matrix needs a single configure and build.
if(CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 19.28)
  set(RunCMake_TEST_SOURCE_DIR "${RunCMake_SOURCE_DIR}/Std")
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/CMP0234-std-build")
  run_cmake(CMP0234-std)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(CMP0234-std-build ${CMAKE_COMMAND} --build . --config Debug)
endif()
