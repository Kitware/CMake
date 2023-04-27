cmake_minimum_required(VERSION 3.19) # CMP0053

include(RunCMake)

# Fix Visual Studio generator name
if(RunCMake_GENERATOR MATCHES "^(Visual Studio [0-9]+ [0-9]+) ")
  set(RunCMake_GENERATOR "${CMAKE_MATCH_1}")
endif()

set(RunCMake-check-file check.cmake)

include("${RunCMake_SOURCE_DIR}/validate_schema.cmake")

function(reset_cmake_presets_directory name)
  set(RunCMake_TEST_SOURCE_DIR "${RunCMake_BINARY_DIR}/${name}")
  file(REMOVE_RECURSE "${RunCMake_TEST_SOURCE_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_SOURCE_DIR}")
endfunction()

function(run_cmake_presets name)
  set(RunCMake_TEST_SOURCE_DIR "${RunCMake_BINARY_DIR}/${name}")
  set(_source_arg "${RunCMake_TEST_SOURCE_DIR}")
  if(CMakePresets_SOURCE_ARG)
    set(_source_arg "${CMakePresets_SOURCE_ARG}")
  endif()

  if(NOT RunCMake_TEST_SOURCE_DIR_NO_CLEAN)
    file(REMOVE_RECURSE "${RunCMake_TEST_SOURCE_DIR}")
    file(MAKE_DIRECTORY "${RunCMake_TEST_SOURCE_DIR}")
  endif()
  configure_file("${RunCMake_SOURCE_DIR}/CMakeLists.txt.in" "${RunCMake_TEST_SOURCE_DIR}/CMakeLists.txt" @ONLY)

  if(NOT CMakePresets_FILE)
    set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/${name}.json.in")
  endif()
  if(EXISTS "${CMakePresets_FILE}")
    configure_file("${CMakePresets_FILE}" "${RunCMake_TEST_SOURCE_DIR}/CMakePresets.json" @ONLY)
  endif()

  if(NOT CMakeUserPresets_FILE)
    set(CMakeUserPresets_FILE "${RunCMake_SOURCE_DIR}/${name}User.json.in")
  endif()
  if(EXISTS "${CMakeUserPresets_FILE}")
    configure_file("${CMakeUserPresets_FILE}" "${RunCMake_TEST_SOURCE_DIR}/CMakeUserPresets.json" @ONLY)
  endif()

  set(_CMakePresets_EXTRA_FILES_OUT)
  set(_CMakePresets_EXTRA_FILES_SCHEMA_EXPECTED_RESULTS)
  foreach(_extra_file IN LISTS CMakePresets_EXTRA_FILES)
    cmake_path(RELATIVE_PATH _extra_file
      BASE_DIRECTORY "${RunCMake_SOURCE_DIR}"
      OUTPUT_VARIABLE _extra_file_relative
      )
    string(REGEX REPLACE "\\.in$" "" _extra_file_out_relative "${_extra_file_relative}")
    set(_extra_file_out "${RunCMake_TEST_SOURCE_DIR}/${_extra_file_out_relative}")
    configure_file("${_extra_file}" "${_extra_file_out}" @ONLY)
    list(APPEND _CMakePresets_EXTRA_FILES_OUT "${_extra_file_out}")
    list(APPEND _CMakePresets_EXTRA_FILES_SCHEMA_EXPECTED_RESULTS 0)
  endforeach()

  set(_s_arg -S)
  if(CMakePresets_NO_S_ARG)
    set(_s_arg)
  endif()
  set(_source_args ${_s_arg} ${_source_arg})
  if(CMakePresets_NO_SOURCE_ARGS)
    set(_source_args)
  endif()
  set(_unused_cli --no-warn-unused-cli)
  if(CMakePresets_WARN_UNUSED_CLI)
    set(_unused_cli)
  endif()

  set(_preset "--preset=${name}")
  if(CMakePresets_NO_PRESET)
    set(_preset)
  endif()

  set(RunCMake_TEST_COMMAND ${CMAKE_COMMAND}
    ${_source_args}
    -DRunCMake_TEST=${name}
    -DRunCMake_GENERATOR=${RunCMake_GENERATOR}
    -DCMAKE_MAKE_PROGRAM=${RunCMake_MAKE_PROGRAM}
    ${_unused_cli}
    ${_preset}
    ${ARGN}
    )
  run_cmake(${name})
endfunction()

# Test CMakePresets.json errors
set(CMakePresets_SCHEMA_EXPECTED_RESULT 1)
run_cmake_presets(NoCMakePresets)
run_cmake_presets(Comment)
run_cmake_presets(JSONParseError)
run_cmake_presets(InvalidRoot)
run_cmake_presets(NoVersion)
run_cmake_presets(InvalidVersion)
run_cmake_presets(LowVersion)
run_cmake_presets(HighVersion)
run_cmake_presets(InvalidVendor)
set(CMakePresets_SCHEMA_EXPECTED_RESULT 0)
run_cmake_presets(NoPresets)
set(CMakePresets_SCHEMA_EXPECTED_RESULT 1)
run_cmake_presets(InvalidPresets)
run_cmake_presets(PresetNotObject)
run_cmake_presets(NoPresetName)
run_cmake_presets(InvalidPresetName)
run_cmake_presets(EmptyPresetName)
set(CMakePresets_SCHEMA_EXPECTED_RESULT 0)
run_cmake_presets(NoPresetGenerator)
set(CMakePresets_SCHEMA_EXPECTED_RESULT 1)
run_cmake_presets(InvalidPresetGenerator)
set(CMakePresets_SCHEMA_EXPECTED_RESULT 0)
run_cmake_presets(NoPresetBinaryDir)
set(CMakePresets_SCHEMA_EXPECTED_RESULT 1)
run_cmake_presets(InvalidPresetBinaryDir)
run_cmake_presets(InvalidVariables)
run_cmake_presets(VariableNotObject)
run_cmake_presets(NoVariableValue)
run_cmake_presets(InvalidVariableValue)
run_cmake_presets(ExtraRootField)
run_cmake_presets(ExtraPresetField)
run_cmake_presets(ExtraVariableField)
run_cmake_presets(FuturePresetInstallDirField)
run_cmake_presets(FuturePresetToolchainField)
run_cmake_presets(InvalidPresetVendor)
set(CMakePresets_SCHEMA_EXPECTED_RESULT 0)
run_cmake_presets(DuplicatePresets)
run_cmake_presets(CyclicInheritance0)
run_cmake_presets(CyclicInheritance1)
run_cmake_presets(CyclicInheritance2)
run_cmake_presets(InvalidInheritance)
run_cmake_presets(ErrorNoWarningDev)
run_cmake_presets(ErrorNoWarningDeprecated)
set(CMakePresets_SCHEMA_EXPECTED_RESULT 1)
run_cmake_presets(InvalidArchitectureStrategy)
run_cmake_presets(UnknownArchitectureStrategy)
run_cmake_presets(InvalidToolsetStrategy)
run_cmake_presets(UnknownToolsetStrategy)
run_cmake_presets(EmptyCacheKey)
run_cmake_presets(EmptyEnvKey)
set(CMakePresets_SCHEMA_EXPECTED_RESULT 0)
run_cmake_presets(UnclosedMacro)
run_cmake_presets(NoSuchMacro)
run_cmake_presets(EnvCycle)
run_cmake_presets(EmptyEnv)
run_cmake_presets(EmptyPenv)
run_cmake_presets(EmptyPenvInInclude)
run_cmake_presets(InvalidRegex)
set(CMakePresets_SCHEMA_EXPECTED_RESULT 1)
run_cmake_presets(ConditionFuture)
run_cmake_presets(SubConditionNull)
run_cmake_presets(TraceNotSupported)

# Test cmakeMinimumRequired field
run_cmake_presets(MinimumRequiredInvalid)
set(CMakePresets_SCHEMA_EXPECTED_RESULT 0)
run_cmake_presets(MinimumRequiredEmpty)
run_cmake_presets(MinimumRequiredMajor)
run_cmake_presets(MinimumRequiredMinor)
run_cmake_presets(MinimumRequiredPatch)

# Test properly working CMakePresets.json
set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/CMakePresets.json.in")
unset(ENV{TEST_ENV})
unset(ENV{TEST_ENV_REF})
unset(ENV{TEST_D_ENV_REF})
set(ENV{TEST_ENV_OVERRIDE} "This environment variable will be overridden")
set(ENV{TEST_PENV} "Process environment variable")
set(ENV{TEST_ENV_REF_PENV} "suffix")
run_cmake_presets(Good "-DTEST_OVERRIDE_1=Overridden value" "-DTEST_OVERRIDE_2:STRING=Overridden value" -C "${RunCMake_SOURCE_DIR}/CacheOverride.cmake" "-UTEST_UNDEF")
unset(ENV{TEST_ENV_OVERRIDE})
unset(ENV{TEST_PENV})
unset(ENV{TEST_ENV_REF_PENV})
run_cmake_presets(GoodNoArgs)
file(REMOVE_RECURSE ${RunCMake_BINARY_DIR}/GoodBinaryUp-build)
run_cmake_presets(GoodBinaryUp)
set(CMakePresets_SOURCE_ARG "../GoodBinaryRelative")
run_cmake_presets(GoodBinaryRelative)
unset(CMakePresets_SOURCE_ARG)
run_cmake_presets(GoodSpaces "--preset" "Good Spaces")
run_cmake_presets(GoodSpacesEq "--preset=Good Spaces")
if(WIN32)
  run_cmake_presets(GoodWindowsBackslash)
endif()
set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/GoodBOM.json.in")
run_cmake_presets(GoodBOM)
set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/CMakePresets.json.in")
file(REMOVE_RECURSE ${RunCMake_BINARY_DIR}/GoodBinaryCmdLine-build)
run_cmake_presets(GoodBinaryCmdLine -B ${RunCMake_BINARY_DIR}/GoodBinaryCmdLine-build)
run_cmake_presets(GoodGeneratorCmdLine -G ${RunCMake_GENERATOR})
run_cmake_presets(InvalidGeneratorCmdLine -G "Invalid Generator")
set(CMakePresets_NO_S_ARG TRUE)
run_cmake_presets(GoodNoS)
set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/GoodNoSCachePrep-build")
run_cmake_presets(GoodNoSCachePrep)
set(CMakePresets_SOURCE_ARG ".")
set(RunCMake_TEST_NO_CLEAN 1)
run_cmake_presets(GoodNoSCache)
unset(RunCMake_TEST_NO_CLEAN)
unset(CMakePresets_SOURCE_ARG)
unset(RunCMake_TEST_BINARY_DIR)
unset(CMakePresets_NO_S_ARG)
set(CMakePresets_NO_SOURCE_ARGS 1)
set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/GoodNoSourceArg")
set(RunCMake_TEST_NO_CLEAN 1)
run_cmake_presets(GoodNoSourceArg)
unset(RunCMake_TEST_NO_CLEAN)
unset(RunCMake_TEST_BINARY_DIR)
unset(CMakePresets_NO_SOURCE_ARGS)
run_cmake_presets(GoodInheritanceParent)
run_cmake_presets(GoodInheritanceChild)
run_cmake_presets(GoodInheritanceOverride)
run_cmake_presets(GoodInheritanceMulti)
run_cmake_presets(GoodInheritanceMultiSecond)
run_cmake_presets(GoodInheritanceMacro)

set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/GoodInstall.json.in")
run_cmake_presets(GoodInstallDefault)
run_cmake_presets(GoodInstallInherit)
run_cmake_presets(GoodInstallOverride)
run_cmake_presets(GoodInstallCommandLine  "--install-prefix=${RunCMake_SOURCE_DIR}/path/passed/on/command_line")

set(RunCMake_TEST_SOURCE_DIR_NO_CLEAN 1)
set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/GoodToolchain.json.in")

reset_cmake_presets_directory(GoodToolchainInherit)
file(WRITE "${RunCMake_BINARY_DIR}/GoodToolchainDefault/toolchain.cmake" "")
run_cmake_presets(GoodToolchainDefault)

reset_cmake_presets_directory(GoodToolchainInherit)
file(WRITE "${RunCMake_BINARY_DIR}/GoodToolchainInherit/toolchain.cmake" "")
run_cmake_presets(GoodToolchainInherit)

reset_cmake_presets_directory(GoodToolchainOverride)
file(WRITE "${RunCMake_BINARY_DIR}/GoodToolchainOverride/override_toolchain.cmake" "")
run_cmake_presets(GoodToolchainOverride)

reset_cmake_presets_directory(GoodToolchainCommandLine)
file(WRITE "${RunCMake_BINARY_DIR}/GoodToolchainCommandLine/cmd_line_toolchain.cmake" "")
run_cmake_presets(GoodToolchainCommandLine  "--toolchain=${RunCMake_BINARY_DIR}/GoodToolchainCommandLine/cmd_line_toolchain.cmake")

unset(RunCMake_TEST_SOURCE_DIR_NO_CLEAN)


set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/CMakePresets.json.in")
# Test bad preset arguments
run_cmake_presets(VendorMacro)
run_cmake_presets(InvalidGenerator)

# Test Visual Studio-specific stuff
if(RunCMake_GENERATOR MATCHES "^Visual Studio ")
  run_cmake_presets(VisualStudioGeneratorArch)
  run_cmake_presets(VisualStudioWin32)
  run_cmake_presets(VisualStudioWin64)
  run_cmake_presets(VisualStudioWin32Override -A x64)
  if(NOT RunCMake_GENERATOR STREQUAL "Visual Studio 9 2008")
    run_cmake_presets(VisualStudioToolset)
    run_cmake_presets(VisualStudioToolsetOverride -T "Test Toolset")
    run_cmake_presets(VisualStudioInheritanceParent)
    run_cmake_presets(VisualStudioInheritanceChild)
    run_cmake_presets(VisualStudioInheritanceOverride)
    run_cmake_presets(VisualStudioInheritanceMulti)
    run_cmake_presets(VisualStudioInheritanceMultiSecond)
  endif()
else()
  run_cmake_presets(ArchToolsetStrategyNone)
  run_cmake_presets(ArchToolsetStrategyDefault)
  run_cmake_presets(ArchToolsetStrategyIgnore)
endif()

# Test bad command line arguments
run_cmake_presets(NoSuchPreset)
run_cmake_presets(NoPresetArgument --preset)
run_cmake_presets(NoPresetArgumentEq --preset= -DA=B)
run_cmake_presets(UseHiddenPreset)

# Test CMakeUserPresets.json
unset(CMakePresets_FILE)
run_cmake_presets(GoodUserOnly)
run_cmake_presets(GoodUserFromMain)
run_cmake_presets(GoodUserFromUser)
run_cmake_presets(V2InheritV3Optional)

# Test CMakeUserPresets.json errors
run_cmake_presets(UserDuplicateInUser)
run_cmake_presets(UserDuplicateCross)
run_cmake_presets(UserInheritance)

# Test listing presets
set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/ListPresets.json.in")
run_cmake_presets(ListPresets --list-presets)
run_cmake_presets(ListPresetsInvalidType --list-presets=invalid-type)

set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/ListPresetsWorkingDir")
set(RunCMake_TEST_NO_CLEAN 1)
set(CMakePresets_NO_SOURCE_ARGS 1)
set(CMakePresets_NO_PRESET 1)
run_cmake_presets(ListPresetsWorkingDir --list-presets)
run_cmake_presets(ListConfigurePresetsWorkingDir --list-presets=configure)
unset(CMakePresets_NO_PRESET)
unset(CMakePresets_NO_SOURCE_ARGS)
unset(RunCMake_TEST_NO_CLEAN)
unset(RunCMake_TEST_BINARY_DIR)

run_cmake_presets(ListPresetsNoSuchPreset)
run_cmake_presets(ListPresetsHidden)

set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/ListAllPresetsNoBuild.json.in")
run_cmake_presets(ListAllPresetsNoBuild --list-presets=all)

set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/ListAllPresetsNoTest.json.in")
run_cmake_presets(ListAllPresetsNoTest --list-presets=all)

# Test warning and error flags
set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/Warnings.json.in")
set(CMakePresets_WARN_UNUSED_CLI 1)
run_cmake_presets(NoWarningFlags)
run_cmake_presets(WarningFlags)
run_cmake_presets(DisableWarningFlags)
run_cmake_presets(ErrorDev)
run_cmake_presets(ErrorDeprecated)
unset(CMakePresets_WARN_UNUSED_CLI)

# Test debug
set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/Debug.json.in")
run_cmake_presets(NoDebug)
run_cmake_presets(Debug)

# Test trace
set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/Trace.json.in")
run_cmake_presets(NoTrace)
run_cmake_presets(ExplicitNoTrace)
run_cmake_presets(Trace)
run_cmake_presets(TraceExpand)
run_cmake_presets(TraceFormatJSON)
run_cmake_presets(TraceFormatHuman)
run_cmake_presets(TraceSource)
run_cmake_presets(TraceRedirect)
run_cmake_presets(TraceAll)

# Test ${hostSystemName} macro
set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/HostSystemName.json.in")
run_cmake_presets(HostSystemName)
set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/HostSystemNameFuture.json.in")
run_cmake_presets(HostSystemNameFuture)

# Test ${fileDir} macro
set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/FileDir.json.in")
set(CMakePresets_EXTRA_FILES
  "${RunCMake_SOURCE_DIR}/subdir/FileDir.json.in"
  )
run_cmake_presets(FileDir)
unset(CMakePresets_EXTRA_FILES)
set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/FileDirFuture.json.in")
run_cmake_presets(FileDirFuture)

# Test ${pathListSep} macro
set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/PathListSep.json.in")
run_cmake_presets(PathListSep)
set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/PathListSepFuture.json.in")
run_cmake_presets(PathListSepFuture)

# Test conditions
set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/Conditions.json.in")
run_cmake_presets(ListConditions --list-presets)
run_cmake_presets(SimpleTrue)
run_cmake_presets(SimpleFalse)
unset(CMakePresets_FILE)

# Test optional generator and buildDir fields
run_cmake_presets(OptionalBinaryDirField -B "${RunCMake_BINARY_DIR}/OptionalBinaryDirField/build")
run_cmake_presets(OptionalGeneratorField -G "${RunCMake_GENERATOR}")
set(CMakePresets_NO_S_ARG TRUE)
set(CMakePresets_SOURCE_ARG "../OptionalBinaryDirFieldNoS")
run_cmake_presets(OptionalBinaryDirFieldNoS)
unset(CMakePresets_SOURCE_ARG)
unset(CMakePresets_NO_S_ARG)

# Test include field
set(CMakePresets_SCHEMA_EXPECTED_RESULT 1)
run_cmake_presets(IncludeV3)
set(CMakePresets_SCHEMA_EXPECTED_RESULT 0)
set(CMakePresets_EXTRA_FILES
  "${RunCMake_SOURCE_DIR}/IncludeV4V3Extra.json.in"
  )
set(CMakePresets_EXTRA_FILES_SCHEMA_EXPECTED_RESULTS 1)
run_cmake_presets(IncludeV4V3)
unset(CMakePresets_EXTRA_FILES_SCHEMA_EXPECTED_RESULTS)
set(CMakePresets_EXTRA_FILES
  "${RunCMake_SOURCE_DIR}/IncludeCommon.json.in"
  "${RunCMake_SOURCE_DIR}/IncludeUserCommon.json.in"
  "${RunCMake_SOURCE_DIR}/subdir/CMakePresets.json.in"
  )
run_cmake_presets(Include --list-presets)
set(CMakePresets_EXTRA_FILES
  "${RunCMake_SOURCE_DIR}/IncludeCommon.json.in"
  )
set(ENV{TEST_ENV_INCLUDE_DIR} ${RunCMake_BINARY_DIR}/IncludeExpansion)
run_cmake_presets(IncludeExpansion --list-presets)
unset(ENV{TEST_ENV_INCLUDE_DIR})
unset(CMakePresets_EXTRA_FILES)
run_cmake_presets(IncludeNotFound)
run_cmake_presets(IncludeCycle)
set(CMakePresets_EXTRA_FILES
  "${RunCMake_SOURCE_DIR}/IncludeCycle3Files2.json.in"
  "${RunCMake_SOURCE_DIR}/IncludeCycle3Files3.json.in"
  )
run_cmake_presets(IncludeCycle3Files)
set(CMakePresets_EXTRA_FILES
  "${RunCMake_SOURCE_DIR}/IncludeOutsideProjectIntermediate.json.in"
  )
run_cmake_presets(IncludeOutsideProject)
unset(CMakePresets_EXTRA_FILES)
run_cmake_presets(IncludeUserOutsideProject)

# Test the example from the documentation
file(READ "${RunCMake_SOURCE_DIR}/../../../Help/manual/presets/example.json" _example)
string(REPLACE "\"generator\": \"Ninja\"" "\"generator\": \"@RunCMake_GENERATOR@\"" _example "${_example}")
if(CMAKE_HOST_WIN32)
  string(REPLACE [["PATH": "$env{HOME}/ninja/bin:$penv{PATH}"]] [["PATH": "$env{HOME}/ninja/bin;$penv{PATH}"]] _example "${_example}")
endif()
file(WRITE "${RunCMake_BINARY_DIR}/example.json.in" "${_example}")
set(CMakePresets_FILE "${RunCMake_BINARY_DIR}/example.json.in")
set(CMakePresets_EXTRA_FILES
  "${RunCMake_SOURCE_DIR}/otherThings.json.in"
  "${RunCMake_SOURCE_DIR}/moreThings.json.in"
)
run_cmake_presets(DocumentationExample --preset=default)
run_cmake_presets(DocumentationExampleListAllPresets --list-presets=all)
unset(CMakePresets_EXTRA_FILES)
