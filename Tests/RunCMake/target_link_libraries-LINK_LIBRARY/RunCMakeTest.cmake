
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

  run_cmake(LINK_LIBRARY)

  run_cmake_target(LINK_LIBRARY simple1 LinkLibrary_simple1)
  run_cmake_target(LINK_LIBRARY simple2 LinkLibrary_simple2)
  run_cmake_target(LINK_LIBRARY group1 LinkLibrary_group1)
  run_cmake_target(LINK_LIBRARY group2 LinkLibrary_group2)
  run_cmake_target(LINK_LIBRARY nested-feature1 LinkLibrary_nested_feature1)
  run_cmake_target(LINK_LIBRARY nested-feature2 LinkLibrary_nested_feature2)
  run_cmake_target(LINK_LIBRARY link-items1 LinkLibrary_link_items1)
  run_cmake_target(LINK_LIBRARY link-items2 LinkLibrary_link_items2)
  run_cmake_target(LINK_LIBRARY link-items3 LinkLibrary_link_items3)
  run_cmake_target(LINK_LIBRARY link-items4 LinkLibrary_link_items4)
  run_cmake_target(LINK_LIBRARY mix-features1 LinkLibrary_mix_features1)
  run_cmake_target(LINK_LIBRARY mix-features2 LinkLibrary_mix_features2)
  run_cmake_target(LINK_LIBRARY mix-features3 LinkLibrary_mix_features3)

  # testing target property LINK_LIBRARY_OVERRIDE
  run_cmake_target(LINK_LIBRARY override-features1 LinkLibrary_override_features1)
  run_cmake_target(LINK_LIBRARY override-features2 LinkLibrary_override_features2)
  run_cmake_target(LINK_LIBRARY override-with-DEFAULT LinkLibrary_override_with_default)
  # testing target property LINK_LIBRARY_OVERRIDE_<LIBRARY>
  run_cmake_target(LINK_LIBRARY override-features3 LinkLibrary_override_features3)
  run_cmake_target(LINK_LIBRARY override-features4 LinkLibrary_override_features4)

  run_cmake(imported-target)

  # tests using features as described in the documentation
  if(CMAKE_C_COMPILER_ID STREQUAL "AppleClang"
      OR (CMAKE_C_COMPILER_ID STREQUAL "MSVC" AND MSVC_VERSION GREATER "1900")
      OR (CMAKE_C_COMPILER_ID STREQUAL "GNU" AND CMAKE_SYSTEM_NAME STREQUAL "Linux"))
    run_cmake(load_archive)
    run_cmake_target(load_archive link-exe main)
  endif()
  if(CMAKE_C_COMPILER_ID STREQUAL "AppleClang")
    run_cmake(weak_library)
    run_cmake_target(weak_library link-exe main)
  endif()

  unset(RunCMake_TEST_OPTIONS)
  unset(RunCMake_TEST_OUTPUT_MERGE)

endif()

# Apple framework features
if(APPLE AND (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang"))
  run_cmake(apple_framework)
  run_cmake_target(apple_framework framework main-framework)
  run_cmake_target(apple_framework reexport_framework main-reexport_framework)
  run_cmake_target(apple_framework weak_framework main-weak_framework)

  run_cmake_target(apple_framework target-framework main-target-framework)
  run_cmake_target(apple_framework target-reexport_framework main-target-reexport_framework)
  run_cmake_target(apple_framework target-weak_framework main-target-weak_framework)

  if(RunCMake_GENERATOR_IS_MULTI_CONFIG AND (NOT XCODE OR XCODE_VERSION GREATER_EQUAL 13))
    run_cmake_target(apple_framework target-framework-postfix main-target-framework-postfix)
    run_cmake_target(apple_framework target-reexport_framework-postfix main-target-reexport_framework-postfix)
    run_cmake_target(apple_framework target-weak_framework-postfix main-target-weak_framework-postfix)
  endif()
endif()

if (CMAKE_C_COMPILER_ID STREQUAL "AppleClang" AND CMAKE_C_COMPILER_VERSION GREATER_EQUAL "12")
  run_cmake_target(apple_framework needed_framework main-needed_framework)

  run_cmake_target(apple_framework target-needed_framework main-target-needed_framework)

  if(RunCMake_GENERATOR_IS_MULTI_CONFIG AND (NOT XCODE OR XCODE_VERSION GREATER_EQUAL 13))
    run_cmake_target(apple_framework target-needed_framework-postfix main-target-needed_framework-postfix)
  endif()
endif()

# Apple library features
if(APPLE AND (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang"))
  run_cmake(apple_library_external)
  run_cmake_target(apple_library_external build external)
  run_cmake_with_options(apple_library "-DRunCMake_BINARY_DIR=${RunCMake_BINARY_DIR}")
  run_cmake_target(apple_library reexport_library main-reexport_library)
  run_cmake_target(apple_library weak_library main-weak_library)
endif()

if (CMAKE_C_COMPILER_ID STREQUAL "AppleClang" AND CMAKE_C_COMPILER_VERSION GREATER_EQUAL "12")
  run_cmake_target(apple_library needed_library main-needed_library)
endif()

# WHOLE_ARCHIVE feature
if ((CMAKE_SYSTEM_NAME STREQUAL "Windows" AND
      ((DEFINED MSVC_VERSION AND MSVC_VERSION GREATER "1900") OR (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang" AND NOT CMAKE_C_SIMULATE_ID STREQUAL "MSVC")))
    OR (CMAKE_SYSTEM_NAME STREQUAL "SunOS" AND
      (NOT CMAKE_C_COMPILER_ID STREQUAL "SunPro" OR CMAKE_C_COMPILER_VERSION GREATER "5.9"))
    OR CMAKE_SYSTEM_NAME MATCHES "Darwin|iOS|tvOS|visionOS|watchOS|Linux|BSD|MSYS|CYGWIN")
  run_cmake(feature-WHOLE_ARCHIVE)
  run_cmake_target(feature-WHOLE_ARCHIVE link-exe main)
endif()
