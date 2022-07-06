
include(RunCMake)

cmake_policy(SET CMP0054 NEW)

macro(run_cmake_target test subtest target)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${test}-${subtest} ${CMAKE_COMMAND} --build . --target ${target} --config Release --verbose ${ARGN})

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
endmacro()


# Some environments are excluded because they are not able to honor verbose mode
if ((RunCMake_GENERATOR MATCHES "Makefiles|Ninja|Xcode"
    OR (RunCMake_GENERATOR MATCHES "Visual Studio" AND MSVC_VERSION GREATER_EQUAL "1600"))
    AND NOT CMAKE_C_COMPILER_ID STREQUAL "Intel")

  set(RunCMake_TEST_OUTPUT_MERGE TRUE)
  if (NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Release)
  endif()

  if (CMAKE_SYSTEM_NAME STREQUAL "Windows"
      OR CMAKE_SYSTEM_NAME STREQUAL "CYGWIN"
      OR CMAKE_SYSTEM_NAME STREQUAL "MSYS")
    set(LINK_SHARED_LIBRARY_PREFIX ${CMAKE_IMPORT_LIBRARY_PREFIX})
    set(LINK_SHARED_LIBRARY_SUFFIX ${CMAKE_IMPORT_LIBRARY_SUFFIX})
  else()
    set(LINK_SHARED_LIBRARY_PREFIX ${CMAKE_SHARED_LIBRARY_PREFIX})
    set(LINK_SHARED_LIBRARY_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
  endif()
  if (MINGW OR MSYS OR CYGWIN)
    set(LINK_EXTERN_LIBRARY_SUFFIX "")
  else()
    set(LINK_EXTERN_LIBRARY_SUFFIX "${CMAKE_IMPORT_LIBRARY_SUFFIX}")
  endif()

  run_cmake(LINK_GROUP)

  run_cmake_target(LINK_GROUP simple1 LinkGroup_simple1)
  run_cmake_target(LINK_GROUP simple2 LinkGroup_simple2)
  run_cmake_target(LINK_GROUP multiple-definitions LinkGroup_multiple-definitions)
  run_cmake_target(LINK_GROUP multiple-groups LinkGroup_multiple-groups)
  run_cmake_target(LINK_GROUP group-and-single LinkGroup_group-and-single)
  run_cmake_target(LINK_GROUP with-LINK_LIBRARY LinkGroup_with-LINK_LIBRARY)
  run_cmake_target(LINK_GROUP with-LINK_LIBRARY2 LinkGroup_with-LINK_LIBRARY2)
  run_cmake_target(LINK_GROUP with-LINK_LIBRARY_OVERRIDE LinkGroup_with-LINK_LIBRARY_OVERRIDE)

  run_cmake(imported-target)

  # tests using features as described in the documentation
  if((CMAKE_C_COMPILER_ID STREQUAL "GNU" AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
      OR (CMAKE_C_COMPILER_ID STREQUAL "SunPro" AND CMAKE_C_COMPILER_VERSION VERSION_GREATER "5.9"
          AND CMAKE_SYSTEM_NAME STREQUAL "SunOS"))
    run_cmake(cross_refs)
    run_cmake_target(cross_refs link main)
  endif()

  unset(RunCMake_TEST_OPTIONS)
  unset(RunCMake_TEST_OUTPUT_MERGE)

endif()

# Feature RESCAN
if (CMAKE_SYSTEM_NAME MATCHES "Linux|BSD"
    OR (CMAKE_SYSTEM_NAME STREQUAL "SunOS" AND (NOT CMAKE_C_COMPILER_ID STREQUAL "SunPro" OR CMAKE_C_COMPILER_VERSION VERSION_GREATER "5.9"))
    OR (WIN32 AND CMAKE_C_COMPILER_ID STREQUAL "GNU"))
  run_cmake(rescan)
  run_cmake_target(rescan link main)
endif()
