include(RunCMake)

if(RunCMake_GENERATOR MATCHES "Ninja")
  # Detect ninja version so we know what tests can be supported.
  execute_process(
    COMMAND "${RunCMake_MAKE_PROGRAM}" --version
    OUTPUT_VARIABLE ninja_out
    ERROR_VARIABLE ninja_out
    RESULT_VARIABLE ninja_res
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  if(ninja_res EQUAL 0 AND "x${ninja_out}" MATCHES "^x[0-9]+\\.[0-9]+")
    set(ninja_version "${ninja_out}")
    message(STATUS "ninja version: ${ninja_version}")
  else()
    message(FATAL_ERROR "'ninja --version' reported:\n${ninja_out}")
  endif()
endif()

if(RunCMake_GENERATOR STREQUAL "Borland Makefiles" OR
   RunCMake_GENERATOR STREQUAL "Watcom WMake")
  set(fs_delay 3)
else()
  set(fs_delay 1.125)
endif()

function(run_BuildDepends CASE)
  # Use a single build tree for a few tests without cleaning.
  if(NOT RunCMake_TEST_BINARY_DIR)
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${CASE}-build)
  endif()
  set(RunCMake_TEST_NO_CLEAN 1)
  if(NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
  endif()
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  include(${RunCMake_SOURCE_DIR}/${CASE}.step1.cmake OPTIONAL)
  run_cmake(${CASE})
  set(RunCMake-check-file check.cmake)
  set(check_step 1)
  run_cmake_command(${CASE}-build1 ${CMAKE_COMMAND} --build . --config Debug)
  if(run_BuildDepends_skip_step_2)
    return()
  endif()
  execute_process(COMMAND ${CMAKE_COMMAND} -E sleep ${fs_delay}) # handle 1s resolution
  include(${RunCMake_SOURCE_DIR}/${CASE}.step2.cmake OPTIONAL)
  set(check_step 2)
  run_cmake_command(${CASE}-build2 ${CMAKE_COMMAND} --build . --config Debug)
  if(run_BuildDepends_skip_step_3)
    return()
  endif()
  execute_process(COMMAND ${CMAKE_COMMAND} -E sleep ${fs_delay}) # handle 1s resolution
  include(${RunCMake_SOURCE_DIR}/${CASE}.step3.cmake OPTIONAL)
  set(check_step 3)
  run_cmake_command(${CASE}-build3 ${CMAKE_COMMAND} --build . --config Debug)
endfunction()

set(run_BuildDepends_skip_step_3 1)

run_BuildDepends(C-Exe)
if(NOT RunCMake_GENERATOR STREQUAL "Xcode")
  if(RunCMake_GENERATOR_TOOLSET MATCHES "^(v80|v90|v100)$")
    # VS 10 forgets to re-link when a manifest changes
    set(run_BuildDepends_skip_step_2 1)
  endif()
  run_BuildDepends(C-Exe-Manifest)
  unset(run_BuildDepends_skip_step_2)
endif()

if(CMake_TEST_Fortran)
  run_BuildDepends(FortranInclude)
endif()

run_BuildDepends(Custom-Symbolic-and-Byproduct)
run_BuildDepends(Custom-Always)

set(RunCMake_TEST_OUTPUT_MERGE_save "${RunCMake_TEST_OUTPUT_MERGE}")
set(RunCMake_TEST_OUTPUT_MERGE 1)
run_BuildDepends(ExternalProjectCacheArgs)
set(RunCMake_TEST_OUTPUT_MERGE "${RunCMake_TEST_OUTPUT_MERGE_save}")

# Test header dependencies with a build tree underneath a source tree.
set(RunCMake_TEST_SOURCE_DIR "${RunCMake_BINARY_DIR}/BuildUnderSource")
set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/BuildUnderSource/build")
file(REMOVE_RECURSE "${RunCMake_TEST_SOURCE_DIR}")
file(MAKE_DIRECTORY "${RunCMake_TEST_SOURCE_DIR}/include")
foreach(f CMakeLists.txt BuildUnderSource.cmake BuildUnderSource.c)
  configure_file("${RunCMake_SOURCE_DIR}/${f}" "${RunCMake_TEST_SOURCE_DIR}/${f}" COPYONLY)
endforeach()
run_BuildDepends(BuildUnderSource)
unset(RunCMake_TEST_BINARY_DIR)
unset(RunCMake_TEST_SOURCE_DIR)

if(RunCMake_GENERATOR MATCHES "Make")
  run_BuildDepends(MakeCustomIncludes)
  if(NOT "${RunCMake_BINARY_DIR}" STREQUAL "${RunCMake_SOURCE_DIR}")
    run_BuildDepends(MakeInProjectOnly)
  endif()
endif()

function(run_RepeatCMake CASE)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${CASE}-build)
  if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake_TEST_OPTIONS -DCMAKE_CONFIGURATION_TYPES=Debug)
  else()
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
  endif()
  run_cmake(${CASE})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${CASE}-build1 ${CMAKE_COMMAND} --build . --config Debug)
  run_cmake_command(${CASE}-rerun1 ${CMAKE_COMMAND} .)
  file(WRITE ${RunCMake_TEST_BINARY_DIR}/exists-for-build2 "")
  run_cmake_command(${CASE}-build2 ${CMAKE_COMMAND} --build . --config Debug)
endfunction()

run_RepeatCMake(RepeatCMake-Custom)

function(run_ReGeneration)
  # test re-generation of project even if CMakeLists.txt files disappeared

  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/regenerate-project-build)
  set(RunCMake_TEST_SOURCE_DIR ${RunCMake_BINARY_DIR}/regenerate-project-source)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(REMOVE_RECURSE "${RunCMake_TEST_SOURCE_DIR}")
  set(ProjectHeader [=[
    cmake_minimum_required(VERSION 3.5)
    project(Regenerate-Project NONE)
  ]=])

  # create project with subdirectory
  file(WRITE "${RunCMake_TEST_SOURCE_DIR}/CMakeLists.txt" "${ProjectHeader}"
    "add_subdirectory(mysubdir)")
  file(MAKE_DIRECTORY "${RunCMake_TEST_SOURCE_DIR}/mysubdir")
  file(WRITE "${RunCMake_TEST_SOURCE_DIR}/mysubdir/CMakeLists.txt" "# empty")

  run_cmake(Regenerate-Project)
  execute_process(COMMAND ${CMAKE_COMMAND} -E sleep ${fs_delay})

  # now we delete the subdirectory and adjust the CMakeLists.txt
  file(REMOVE_RECURSE "${RunCMake_TEST_SOURCE_DIR}/mysubdir")
  file(WRITE "${RunCMake_TEST_SOURCE_DIR}/CMakeLists.txt" "${ProjectHeader}")

  run_cmake_command(Regenerate-Project-Directory-Removed
    ${CMAKE_COMMAND} --build "${RunCMake_TEST_BINARY_DIR}")

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_SOURCE_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
endfunction()

if(RunCMake_GENERATOR STREQUAL "Xcode")
  run_ReGeneration(regenerate-project)
endif()

if(CMake_TEST_BuildDepends_GNU_AS)
  set(ENV{ASM} "${CMake_TEST_BuildDepends_GNU_AS}")
  run_BuildDepends(GNU-AS)
endif()

if ((RunCMake_GENERATOR STREQUAL "Unix Makefiles"
      AND (CMAKE_C_COMPILER_ID STREQUAL "GNU"
        OR CMAKE_C_COMPILER_ID STREQUAL "LCC"
        OR CMAKE_C_COMPILER_ID STREQUAL "Clang"
        OR CMAKE_C_COMPILER_ID STREQUAL "AppleClang"))
    OR (RunCMake_GENERATOR STREQUAL "NMake Makefiles"
      AND MSVC_VERSION GREATER 1300
      AND CMAKE_C_COMPILER_ID STREQUAL "MSVC"))
  run_BuildDepends(CompilerDependencies)
  run_BuildDepends(CustomCommandDependencies)
endif()

if (RunCMake_GENERATOR MATCHES "Makefiles")
  run_cmake(CustomCommandDependencies-BadArgs)
  run_cmake_with_options(CustomCommandDependencies-compiler-deps-legacy -DCMAKE_DEPENDS_USE_COMPILER=FALSE)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(CustomCommandDependencies-compiler-deps-legacy ${CMAKE_COMMAND} --build . --config Debug)
  unset(RunCMake_TEST_NO_CLEAN)
endif()

if(RunCMake_GENERATOR MATCHES "Make|Ninja|Visual Studio|Xcode" AND
    NOT RunCMake_GENERATOR MATCHES "Visual Studio (9|10)( |$)")
  unset(run_BuildDepends_skip_step_3)
  run_BuildDepends(CustomCommandDepfile)
  set(run_BuildDepends_skip_step_3 1)
endif()

if(RunCMake_GENERATOR MATCHES "Make")
  run_BuildDepends(MakeDependencies)
endif()

if(RunCMake_GENERATOR MATCHES "^Visual Studio 9 " OR
   (RunCMake_GENERATOR MATCHES "Ninja" AND ninja_version VERSION_LESS 1.7))
  # This build tool misses the dependency.
  set(run_BuildDepends_skip_step_2 1)
endif()
run_BuildDepends(CustomCommandUnityBuild)
unset(run_BuildDepends_skip_step_2)

#if (RunCMake_GENERATOR MATCHES "Make|Ninja" AND CMAKE_C_LINK_DEPENDS_USE_LINKER)
if (RunCMake_GENERATOR MATCHES "Make|Ninja")
  set(run_BuildDepends_skip_step_2 1)
  run_BuildDepends(LinkDependsCheck)
  include("${RunCMake_BINARY_DIR}/LinkDependsCheck-build/LinkDependsUseLinker.cmake")
  if (CMAKE_C_LINK_DEPENDS_USE_LINKER)
    run_BuildDepends(LinkDependsExternalLibrary)
    unset(run_BuildDepends_skip_step_2)
    run_BuildDepends(LinkDepends)
  endif()
endif()
