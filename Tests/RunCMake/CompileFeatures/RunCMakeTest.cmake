cmake_policy(SET CMP0057 NEW)
include(RunCMake)

run_cmake(NotAFeature)
run_cmake(NotAFeatureGenex)
run_cmake(NotAFeatureTransitive)
run_cmake(NotAFeature_OriginDebug)
run_cmake(NotAFeature_OriginDebugGenex)
run_cmake(NotAFeature_OriginDebugTransitive)
run_cmake(NotAFeature_OriginDebugCommand)

run_cmake(compiler_introspection)
include("${RunCMake_BINARY_DIR}/compiler_introspection-build/info.cmake")

if (NOT C_FEATURES)
  run_cmake(NoSupportedCFeatures)
  run_cmake(NoSupportedCFeaturesGenex)
endif()

if (NOT CXX_FEATURES)
  run_cmake(NoSupportedCxxFeatures)
  run_cmake(NoSupportedCxxFeaturesGenex)
elseif (cxx_std_98 IN_LIST CXX_FEATURES AND cxx_std_11 IN_LIST CXX_FEATURES)
  if(CXX_STANDARD_DEFAULT EQUAL 98)
    run_cmake(LinkImplementationFeatureCycle)
  endif()
  run_cmake(LinkImplementationFeatureCycleSolved)

  if (cxx_final IN_LIST CXX_FEATURES)
    set(RunCMake_TEST_OPTIONS "-DHAVE_FINAL=1")
  endif()
  run_cmake(NonValidTarget1)
  run_cmake(NonValidTarget2)
  unset(RunCMake_TEST_OPTIONS)
endif()

configure_file("${RunCMake_SOURCE_DIR}/CMakeLists.txt" "${RunCMake_BINARY_DIR}/CMakeLists.txt" COPYONLY)
file(READ "${RunCMake_SOURCE_DIR}/CMP0128Common.cmake" cmp0128_common)

function(test_build)
  set(test ${name}-${lang})

  file(READ "${RunCMake_SOURCE_DIR}/${name}.cmake" cmake)
  file(CONFIGURE OUTPUT "${RunCMake_BINARY_DIR}/${test}.cmake" CONTENT "${cmake}${cmp0128_common}" @ONLY)
  if(EXISTS "${RunCMake_SOURCE_DIR}/${name}-build-check.cmake")
    configure_file("${RunCMake_SOURCE_DIR}/${name}-build-check.cmake" "${RunCMake_BINARY_DIR}/${test}-build-check.cmake" @ONLY)
  endif()
  if(EXISTS "${RunCMake_SOURCE_DIR}/${name}-stderr.txt")
    configure_file("${RunCMake_SOURCE_DIR}/${name}-stderr.txt" "${RunCMake_BINARY_DIR}/${test}-stderr.txt" @ONLY)
  endif()

  set(RunCMake_SOURCE_DIR "${RunCMake_BINARY_DIR}")
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${test}-build")
  run_cmake(${test})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${test}-build ${CMAKE_COMMAND} --build . ${ARGN})
endfunction()

# Mangle flags such as they're in verbose build output.
macro(mangle_flags variable)
  set(result "${${variable}}")

  if(RunCMake_GENERATOR MATCHES "Visual Studio" AND MSVC_TOOLSET_VERSION GREATER_EQUAL 141)
    string(REPLACE "-" "/" result "${result}")
  elseif(RunCMake_GENERATOR STREQUAL "Xcode" AND CMAKE_XCODE_BUILD_SYSTEM GREATER_EQUAL 12)
    string(REPLACE "=" [[\\=]] result "${result}")
  endif()

  string(REPLACE ";" " " result "${result}")
  list(APPEND flags "${result}")
endmacro()

function(test_cmp0128_old_same_standard)
  set(flag "${${lang}${${lang}_STANDARD_DEFAULT}_EXT_FLAG}")

  if(NOT flag)
    return()
  endif()

  mangle_flags(flag)

  set(name CMP0128OldSameStandard)
  test_build(--verbose)
endfunction()

function(test_cmp0128_new_extensions_opposite)
  if(extensions_opposite)
    set(flag_ext "_EXT")
  endif()

  set(flag "${${lang}${${lang}_STANDARD_DEFAULT}${flag_ext}_FLAG}")

  if(NOT flag)
    return()
  endif()

  mangle_flags(flag)

  # Make sure we enable/disable extensions when:
  # 1. LANG_STANDARD is unset.
  set(name CMP0128NewExtensionsStandardUnset)
  set(RunCMake_TEST_OPTIONS -DCMAKE_POLICY_DEFAULT_CMP0128=NEW)
  test_build(--verbose)

  # 2. LANG_STANDARD matches CMAKE_LANG_STANDARD_DEFAULT.
  set(name CMP0128NewExtensionsStandardDefault)
  test_build(--verbose)
endfunction()

function(test_cmp0128_new_no_unnecessary_flag)
  set(standard_flag "${${lang}${${lang}_STANDARD_DEFAULT}_FLAG}")
  set(extension_flag "${${lang}${${lang}_STANDARD_DEFAULT}_EXT_FLAG}")

  if(NOT standard_flag AND NOT extension_flag)
    return()
  endif()

  mangle_flags(standard_flag)
  mangle_flags(extension_flag)

  set(name CMP0128NewNoUnnecessaryFlag)
  set(RunCMake_TEST_OPTIONS -DCMAKE_POLICY_DEFAULT_CMP0128=NEW)
  test_build(--verbose)
endfunction()

function(test_cmp0128_warn_match)
  set(name CMP0128WarnMatch)
  test_build()
endfunction()

function(test_cmp0128_warn_unset)
  # For compilers that had CMAKE_<LANG>_EXTENSION_COMPILE_OPTION (only IAR)
  # there is no behavioral change and thus no warning.
  if(NOT "${${lang}_EXT_FLAG}" STREQUAL "")
    return()
  endif()

  if(extensions_opposite)
    set(opposite "enabled")
  else()
    set(opposite "disabled")
  endif()

  set(name CMP0128WarnUnset)
  test_build()
endfunction()

function(test_lang lang ext)
  if(CMake_NO_${lang}_STANDARD)
    return()
  endif()

  set(extensions_default "${${lang}_EXTENSIONS_DEFAULT}")
  set(standard_default "${${lang}_STANDARD_DEFAULT}")

  if(extensions_default)
    set(extensions_opposite OFF)
  else()
    set(extensions_opposite ON)
  endif()

  test_cmp0128_new_extensions_opposite()
  test_cmp0128_new_no_unnecessary_flag()
  test_cmp0128_old_same_standard()
  test_cmp0128_warn_match()
  test_cmp0128_warn_unset()
endfunction()

if(C_STANDARD_DEFAULT)
  test_lang(C c)
endif()

if(CXX_STANDARD_DEFAULT)
  run_cmake(NotAStandard)

  foreach(standard 98 11)
    if (CXX${standard}_FLAG STREQUAL NOTFOUND)
      run_cmake(RequireCXX${standard})
      run_cmake(RequireCXX${standard}Variable)
    endif()
    if (CXX${standard}_EXT_FLAG STREQUAL NOTFOUND)
      run_cmake(RequireCXX${standard}Ext)
      run_cmake(RequireCXX${standard}ExtVariable)
    endif()
  endforeach()

  test_lang(CXX cpp)
endif()
