cmake_policy(SET CMP0057 NEW)

include(RunCMake)
cmake_policy(SET CMP0054 NEW)

run_cmake(VsCSharpCompilerOpts)
run_cmake(ExplicitCMakeLists)
run_cmake(RuntimeLibrary)
run_cmake(SourceGroupCMakeLists)
run_cmake(SourceGroupTreeCMakeLists)

run_cmake(VsConfigurationType)
run_cmake(VsTargetsFileReferences)
run_cmake(VsCustomProps)
run_cmake(VsDebuggerWorkingDir)
run_cmake(VsDebuggerCommand)
run_cmake(VsDebuggerCommandArguments)
run_cmake(VsDebuggerEnvironment)
run_cmake(VsCSharpCustomTags)
run_cmake(VsCSharpDocumentationFile)
run_cmake(VsCSharpReferenceProps)
run_cmake(VsCSharpWithoutSources)
run_cmake(VsCSharpDeployFiles)
run_cmake(VSCSharpDefines)
run_cmake(VsSdkDirectories)
run_cmake(VsGlobals)
run_cmake(VsProjectImport)
run_cmake(VsPackageReferences)
run_cmake(VsDpiAware)
run_cmake(VsDpiAwareBadParam)
run_cmake(VsPrecompileHeaders)
run_cmake(VsPrecompileHeadersReuseFromCompilePDBName)

run_cmake(VsWinRTByDefault)

set(RunCMake_GENERATOR_TOOLSET "VCTargetsPath=$(VCTargetsPath)")
run_cmake(VsVCTargetsPath)
unset(RunCMake_GENERATOR_TOOLSET)

if(CMAKE_C_COMPILER_ID STREQUAL "MSVC" AND CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 19.05)
  run_cmake(VsJustMyCode)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "MSVC" AND CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 19.20)
  run_cmake(VsSpectreMitigation)
endif()

# Visual Studio 2017 has toolset version 141
string(REPLACE "v" "" generator_toolset "${RunCMake_GENERATOR_TOOLSET}")
if (RunCMake_GENERATOR MATCHES "Visual Studio 1[0-4] 201[0-5]" OR
   (RunCMake_GENERATOR_TOOLSET AND generator_toolset VERSION_LESS "141"))
  run_cmake(UnityBuildPre2017)
else()
  run_cmake(UnityBuildNative)
endif()

run_cmake(VsDotnetTargetFramework)
run_cmake(VsDotnetTargetFrameworkVersion)
