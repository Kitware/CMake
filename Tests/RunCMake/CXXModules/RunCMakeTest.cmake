include(RunCMake)

set(stdlib_custom_json)
if (CMake_TEST_CXX_STDLIB_MODULES_JSON)
  list(APPEND stdlib_custom_json
    -DCMAKE_CXX_STDLIB_MODULES_JSON=${CMake_TEST_CXX_STDLIB_MODULES_JSON})
endif ()

run_cmake(Inspect ${stdlib_custom_json})
include("${RunCMake_BINARY_DIR}/Inspect-build/info.cmake")

# Test negative cases where C++20 modules do not work.
run_cmake(NoCXX)
if ("cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
  # This test requires that the compiler be told to compile in an older-than-20
  # standard. If the compiler forces a standard to be used, skip it.
  if (NOT forced_cxx_standard)
    run_cmake(NoCXX20)
    if(CMAKE_CXX_STANDARD_DEFAULT AND CMAKE_CXX20_STANDARD_COMPILE_OPTION)
      run_cmake_with_options(ImplicitCXX20 -DCMAKE_CXX20_STANDARD_COMPILE_OPTION=${CMAKE_CXX20_STANDARD_COMPILE_OPTION})
    endif()
  endif ()

  run_cmake(NoScanningSourceFileProperty)
  run_cmake(NoScanningTargetProperty)
  run_cmake(NoScanningVariable)
  run_cmake(CMP0155-OLD)
  run_cmake(CMP0155-NEW)
  run_cmake(CMP0155-NEW-with-rule)
endif ()

if (RunCMake_GENERATOR MATCHES "Ninja")
  execute_process(
    COMMAND "${CMAKE_MAKE_PROGRAM}" --version
    RESULT_VARIABLE res
    OUTPUT_VARIABLE ninja_version
    ERROR_VARIABLE err
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE)

  if (res)
    message(WARNING
      "Failed to determine `ninja` version: ${err}")
    set(ninja_version "0")
  endif ()
endif ()

set(generator_supports_cxx_modules 0)
if (RunCMake_GENERATOR MATCHES "Ninja" AND
    ninja_version VERSION_GREATER_EQUAL "1.11" AND
    "cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
  set(generator_supports_cxx_modules 1)
endif ()

if (RunCMake_GENERATOR MATCHES "Visual Studio" AND
    CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.34")
  set(generator_supports_cxx_modules 1)
endif ()

# Test behavior when the generator does not support C++20 modules.
if (NOT generator_supports_cxx_modules)
  if ("cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    run_cmake(NoDyndepSupport)
  endif ()

  # Bail; the remaining tests require the generator to successfully generate
  # with C++20 modules in the source list.
  return ()
endif ()

set(fileset_types
  Modules)
set(scopes
  Interface
  Private
  Public)
set(target_types
  Interface
  Static
  )
foreach (fileset_type IN LISTS fileset_types)
  foreach (scope IN LISTS scopes)
    foreach (target_type IN LISTS target_types)
      run_cmake("FileSet${fileset_type}${scope}On${target_type}")
    endforeach ()
  endforeach ()
  run_cmake("FileSet${fileset_type}InterfaceImported")

  # Test the error messages when a non-C++ source file is found in the source
  # list.
  run_cmake("NotCompiledSource${fileset_type}")
  run_cmake("NotCXXSource${fileset_type}")
endforeach ()

if ("cxx_std_23" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
  run_cmake(CXXImportStdConfig)
  run_cmake(CXXImportStdHeadTarget)
  run_cmake(CXXImportStdLinkLanguage)
  run_cmake(CXXImportStdInvalidGenex)
endif ()

if ("cxx_std_23" IN_LIST CMAKE_CXX_COMPILE_FEATURES AND
    NOT have_cxx23_import_std)
  run_cmake(NoCXX23TargetUnset)
  run_cmake(NoCXX23TargetNotRequired)
  run_cmake(NoCXX23TargetRequired)
endif ()

if ("cxx_std_26" IN_LIST CMAKE_CXX_COMPILE_FEATURES AND
    NOT have_cxx26_import_std)
  run_cmake(NoCXX26TargetUnset)
  run_cmake(NoCXX26TargetNotRequired)
  run_cmake(NoCXX26TargetRequired)
endif ()

run_cmake(InstallBMI)
run_cmake(InstallBMIGenericArgs)
run_cmake(InstallBMIIgnore)

run_cmake(ExportBuildCxxModules)
run_cmake(ExportBuildCxxModulesTargets)
run_cmake(ExportInstallCxxModules)

# Generator-specific tests.
if (RunCMake_GENERATOR MATCHES "Ninja")
  run_cmake(NinjaDependInfoFileSet)
  run_cmake(NinjaDependInfoExport)
  run_cmake(NinjaDependInfoExportFilesystemSafe)
  run_cmake(NinjaDependInfoBMIInstall)
  run_cmake(NinjaForceResponseFile) # issue#25367
  run_cmake(NinjaDependInfoCompileDatabase)
elseif (RunCMake_GENERATOR MATCHES "Visual Studio")
  run_cmake(VisualStudioNoSyntheticTargets)
else ()
  message(FATAL_ERROR
    "Please add 'DependInfo' tests for the '${RunCMake_GENERATOR}' generator.")
endif ()
