cmake_minimum_required(VERSION 3.5...3.25) # Enable CMP0091 and CMP0141.
project(FortranOnly Fortran)
message("CTEST_FULL_OUTPUT ")

if("${CMAKE_Fortran_COMPILER_ID};${CMAKE_Fortran_SIMULATE_ID}" MATCHES "^Intel(LLVM)?;MSVC$")
  string(APPEND CMAKE_Fortran_FLAGS_DEBUG " -Z7")
  string(APPEND CMAKE_Fortran_FLAGS_RELWITHDEBINFO " -Z7")
endif()

# create a library with hello and world functions
add_library(FortranOnlylib hello.f world.f)
set_property(TARGET FortranOnlylib PROPERTY Fortran_FORMAT FIXED)
set_property(SOURCE world.f PROPERTY Fortran_FORMAT FREE)

# create an executable that calls hello and world
add_executable(FortranOnly1 testf.f)
set_property(TARGET FortranOnly1 PROPERTY OUTPUT_NAME FortranOnly)
target_link_libraries(FortranOnly1 FortranOnlylib)

# Test that Fortran+RC work together.
# FIXME: Add .rc in more cases.
if(CMAKE_GENERATOR MATCHES "Visual Studio")
  set(test_rc testRC.rc)
endif()

# create a custom command that runs FortranOnly1 and puts
# the output into the file testfhello.txt
add_custom_command(OUTPUT ${FortranOnly_BINARY_DIR}/testfhello.txt
  COMMAND FortranOnly1
  > testfhello.txt)
# create a second executable FortranOnly2 that has
# testfhello.txt has an source file so that it will
# run the above custom command.
add_executable(FortranOnly2 testfhello.txt testf.f ${test_rc})
target_link_libraries(FortranOnly2 FortranOnlylib)
# create a custom target to check the content of testfhello.txt
# by running the cmake script checktestf2.cmake
add_custom_target(checktestf2 ALL
  COMMAND ${CMAKE_COMMAND}
  -P ${FortranOnly_SOURCE_DIR}/checktestf2.cmake)

# create a custom target that runs FortranOnly1 executable and creates
# a file out.txt that should have hello world in it.
add_custom_target(sayhello ALL
  COMMAND FortranOnly1 > out.txt
)
# make sure stuff is built in the right order
add_dependencies(checktestf2 FortranOnly2)
add_dependencies(sayhello FortranOnly1)
add_dependencies(FortranOnly2 FortranOnly1)

# add a custom target that checks that out.txt has the correct
# content
add_custom_target(checksayhello ALL
  COMMAND ${CMAKE_COMMAND} -P ${FortranOnly_SOURCE_DIR}/checksayhello.cmake
  )
add_dependencies(checksayhello sayhello)

include(CheckFortranSourceCompiles)
unset(HAVE_PRINT CACHE)
CHECK_Fortran_SOURCE_COMPILES([[
      PROGRAM TEST_HAVE_PRINT
        PRINT *, 'Hello'
      END
]] HAVE_PRINT)
if(NOT HAVE_PRINT)
  set(configure_log "${CMAKE_BINARY_DIR}/CMakeFiles/CMakeConfigureLog.yaml")
  if(EXISTS "${configure_log}")
    file(READ "${configure_log}" log_content)
  else()
    set(log_content "")
  endif()
  if(log_content MATCHES [[(  -
    kind: "try_compile-v1"(
+    [^
]+)+
    checks:
      - "Performing Test HAVE_PRINT"(
+    [^
]+)+)]])
    set(err "${CMAKE_MATCH_1}")
  else()
    set(err "")
  endif()
  message(SEND_ERROR "CHECK_Fortran_SOURCE_COMPILES for HAVE_PRINT failed:\n"
    "${err}")
endif()

unset(Fortran_BOGUS_FLAG CACHE)
include(CheckFortranCompilerFlag)
CHECK_Fortran_COMPILER_FLAG(-_this_is_not_a_flag_ Fortran_BOGUS_FLAG)
if (Fortran_BOGUS_FLAG)
  message(SEND_ERROR "CHECK_Fortran_COMPILER_FLAG() succeeded, but should have failed")
endif()

unset(Fortran_RUN_FLAG CACHE)
include(CheckFortranSourceRuns)
check_fortran_source_runs("program a; end program" Fortran_RUN_FLAG SRC_EXT F90)
if(NOT Fortran_RUN_FLAG)
  message(SEND_ERROR "CHECK_Fortran_SOURCE_RUNS() failed")
endif()

# Test generation of preprocessed sources.
if("${CMAKE_GENERATOR}" MATCHES "Makefile" AND CMAKE_MAKE_PROGRAM)
  if(CMAKE_Fortran_CREATE_PREPROCESSED_SOURCE)
    # Skip running this part of the test on certain platforms
    # until they are fixed.
    set(MAYBE_ALL ALL)
    list(LENGTH CMAKE_OSX_ARCHITECTURES ARCH_COUNT)
    if(ARCH_COUNT GREATER 1)
      # OSX does not support preprocessing more than one architecture.
      set(MAYBE_ALL)
    endif()

    add_executable(preprocess preprocess.F)

    # Custom target to try preprocessing invocation.
    add_custom_target(test_preprocess ${MAYBE_ALL}
      COMMAND ${CMAKE_COMMAND} -E rm -f CMakeFiles/preprocess.dir/preprocess.F.i
      COMMAND ${CMAKE_MAKE_PROGRAM} preprocess.i
      COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/test_preprocess.cmake
      # Remove bogus file some compilers leave behind.
      COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_CURRENT_SOURCE_DIR}/preprocess.s
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      )
  endif()
endif()

# Test that with Intel Fortran we always compile with preprocessor
# defines even if splitting the preprocessing and compilation steps.
if(CMAKE_Fortran_COMPILER_ID MATCHES "Intel")
  add_executable(IntelIfDef IntelIfDef.f)
  set_property(TARGET IntelIfDef PROPERTY Fortran_FORMAT FIXED)
  target_compile_definitions(IntelIfDef PRIVATE SOME_DEF)
endif()

# Skip these tests if compiler/version does enable and disable preprocessing
if(CMAKE_Fortran_COMPILE_OPTIONS_PREPROCESS_ON AND
   CMAKE_Fortran_COMPILE_OPTIONS_PREPROCESS_OFF)
  # Test that we can always preprocess a target
  add_executable(preprocess_target preprocess2.f)
  set_property(TARGET preprocess_target PROPERTY Fortran_PREPROCESS ON)

  if(CMAKE_Fortran_COMPILER_ID STREQUAL "GNU")
    # gfortran might report spurious warnings if we include the
    # preprocessing flags at the compilation stage
    target_compile_options(preprocess_target PRIVATE -Wall -Werror)
  endif()

  # Test that we can preprocess a single source file
  add_executable(preprocess_source preprocess3.f)
  set_property(SOURCE preprocess3.f PROPERTY Fortran_PREPROCESS ON)
endif()

# LCC < 1.24 has no way to disable Fortran preprocessor
if(NOT CMAKE_Fortran_COMPILER_ID STREQUAL "LCC" OR CMAKE_Fortran_COMPILER_VERSION VERSION_GREATER_EQUAL "1.24.00")
  # Test that neither the compiler nor CMake performs unnecessary preprocessing.
  add_library(no_preprocess_target_lower STATIC no_preprocess_target_lower.f)
  target_compile_options(no_preprocess_target_lower PRIVATE -DINTEGER=nonsense)
  set_property(TARGET no_preprocess_target_lower PROPERTY Fortran_PREPROCESS OFF)
  add_library(no_preprocess_source_lower STATIC no_preprocess_source_lower.f)
  target_compile_options(no_preprocess_source_lower PRIVATE -DINTEGER=nonsense)
  set_property(SOURCE no_preprocess_source_lower.f PROPERTY Fortran_PREPROCESS OFF)
endif()

# Test that we can explicitly not preprocess a target or source.
# This will not work on certain compilers due to either missing a
# "don't preprocess" flag, or due to the flags being ignored for
# extensions like '.F' and '.fpp'.
if(CMAKE_Fortran_COMPILE_OPTIONS_PREPROCESS_OFF AND
    NOT CMAKE_Fortran_COMPILER_ID MATCHES "(Flang|NAG|PGI|NVHPC|SunPro|XL)")

  set(CMAKE_Fortran_PREPROCESS OFF)
  add_library(no_preprocess_target STATIC no_preprocess_target_upper.F)
  target_compile_options(no_preprocess_target PRIVATE -DINTEGER=nonsense)
  unset(CMAKE_Fortran_PREPROCESS)

  add_library(no_preprocess_source STATIC no_preprocess_source_upper.F)
  target_compile_options(no_preprocess_source PRIVATE -DINTEGER=nonsense)

  if(NOT CMAKE_Fortran_COMPILER_ID STREQUAL "Cray"
      AND NOT "${CMAKE_Fortran_COMPILER_ID};${CMAKE_Fortran_SIMULATE_ID}" MATCHES "Intel(LLVM)?;MSVC")
    target_sources(no_preprocess_target PRIVATE no_preprocess_target_fpp.fpp)
    target_sources(no_preprocess_source PRIVATE no_preprocess_source_fpp.fpp)
  endif()

  set_property(SOURCE no_preprocess_source_upper.F no_preprocess_source_fpp.fpp PROPERTY Fortran_PREPROCESS OFF)
endif()
