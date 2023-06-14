/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmLocalVisualStudio7Generator.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <functional>
#include <sstream>
#include <utility>

#include <cm/memory>
#include <cm/optional>
#include <cmext/algorithm>

#include <windows.h>

#include <cm3p/expat.h>

#include "cmsys/FStream.hxx"

#include "cmComputeLinkInformation.h"
#include "cmCustomCommand.h"
#include "cmCustomCommandGenerator.h"
#include "cmCustomCommandLines.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmGlobalVisualStudio7Generator.h"
#include "cmGlobalVisualStudioGenerator.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmOutputConverter.h"
#include "cmPolicies.h"
#include "cmSourceFile.h"
#include "cmSourceGroup.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetDepend.h"
#include "cmValue.h"
#include "cmVsProjectType.h"
#include "cmXMLParser.h"
#include "cmake.h"

static bool cmLVS7G_IsFAT(const char* dir);

class cmLocalVisualStudio7GeneratorInternals
{
public:
  cmLocalVisualStudio7GeneratorInternals(cmLocalVisualStudio7Generator* e)
    : LocalGenerator(e)
  {
  }
  using ItemVector = cmComputeLinkInformation::ItemVector;
  void OutputLibraries(std::ostream& fout, ItemVector const& libs);
  void OutputObjects(std::ostream& fout, cmGeneratorTarget* t,
                     std::string const& config, const char* isep = nullptr);

private:
  cmLocalVisualStudio7Generator* LocalGenerator;
};

class cmLocalVisualStudio7Generator::AllConfigSources
{
public:
  std::vector<cmGeneratorTarget::AllConfigSource> Sources;
  std::map<cmSourceFile const*, size_t> Index;
};

extern cmVS7FlagTable cmLocalVisualStudio7GeneratorFlagTable[];

cmLocalVisualStudio7Generator::cmLocalVisualStudio7Generator(
  cmGlobalGenerator* gg, cmMakefile* mf)
  : cmLocalVisualStudioGenerator(gg, mf)
  , Internal(cm::make_unique<cmLocalVisualStudio7GeneratorInternals>(this))
{
}

cmLocalVisualStudio7Generator::~cmLocalVisualStudio7Generator() = default;

void cmLocalVisualStudio7Generator::AddHelperCommands()
{
  // Now create GUIDs for targets
  const auto& tgts = this->GetGeneratorTargets();
  for (const auto& l : tgts) {
    if (!l->IsInBuildSystem()) {
      continue;
    }
    cmValue path = l->GetProperty("EXTERNAL_MSPROJECT");
    if (path) {
      this->ReadAndStoreExternalGUID(l->GetName(), path->c_str());
    }
  }

  this->FixGlobalTargets();
}

void cmLocalVisualStudio7Generator::Generate()
{
  // Create the project file for each target.
  for (cmGeneratorTarget* gt :
       this->GlobalGenerator->GetLocalGeneratorTargetsInOrder(this)) {
    if (!gt->IsInBuildSystem() || gt->GetProperty("EXTERNAL_MSPROJECT")) {
      continue;
    }

    auto& gtVisited = this->GetSourcesVisited(gt);
    auto const& deps = this->GlobalGenerator->GetTargetDirectDepends(gt);
    for (auto const& d : deps) {
      // Take the union of visited source files of custom commands
      auto depVisited = this->GetSourcesVisited(d);
      gtVisited.insert(depVisited.begin(), depVisited.end());
    }

    this->GenerateTarget(gt);
  }

  this->WriteStampFiles();
}

void cmLocalVisualStudio7Generator::FixGlobalTargets()
{
  // Visual Studio .NET 2003 Service Pack 1 will not run post-build
  // commands for targets in which no sources are built.  Add dummy
  // rules to force these targets to build.
  const auto& tgts = this->GetGeneratorTargets();
  for (auto const& l : tgts) {
    if (l->GetType() == cmStateEnums::GLOBAL_TARGET) {
      cmCustomCommandLines force_commands =
        cmMakeSingleCommandLine({ "cd", "." });
      std::string force = cmStrCat(this->GetCurrentBinaryDirectory(),
                                   "/CMakeFiles/", l->GetName(), "_force");
      if (cmSourceFile* sf =
            this->Makefile->GetOrCreateGeneratedSource(force)) {
        sf->SetProperty("SYMBOLIC", "1");
      }
      auto cc = cm::make_unique<cmCustomCommand>();
      cc->SetOutputs(force);
      cc->SetCommandLines(force_commands);
      cc->SetComment(" ");
      if (cmSourceFile* file =
            this->AddCustomCommandToOutput(std::move(cc), true)) {
        l->AddSource(file->ResolveFullPath());
      }
    }
  }
}

void cmLocalVisualStudio7Generator::WriteStampFiles()
{
  // Touch a timestamp file used to determine when the project file is
  // out of date.
  std::string stampName =
    cmStrCat(this->GetCurrentBinaryDirectory(), "/CMakeFiles");
  cmSystemTools::MakeDirectory(stampName);
  stampName += "/generate.stamp";
  cmsys::ofstream stamp(stampName.c_str());
  stamp << "# CMake generation timestamp file for this directory.\n";

  // Create a helper file so CMake can determine when it is run
  // through the rule created by CreateVCProjBuildRule whether it
  // really needs to regenerate the project.  This file lists its own
  // dependencies.  If any file listed in it is newer than itself then
  // CMake must rerun.  Otherwise the project files are up to date and
  // the stamp file can just be touched.
  std::string depName = cmStrCat(stampName, ".depend");
  cmsys::ofstream depFile(depName.c_str());
  depFile << "# CMake generation dependency list for this directory.\n";

  std::vector<std::string> listFiles(this->Makefile->GetListFiles());
  cmake* cm = this->GlobalGenerator->GetCMakeInstance();
  if (cm->DoWriteGlobVerifyTarget()) {
    listFiles.push_back(cm->GetGlobVerifyStamp());
  }

  // Sort the list of input files and remove duplicates.
  std::sort(listFiles.begin(), listFiles.end(), std::less<std::string>());
  auto new_end = std::unique(listFiles.begin(), listFiles.end());
  listFiles.erase(new_end, listFiles.end());

  for (const std::string& lf : listFiles) {
    depFile << lf << "\n";
  }
}

void cmLocalVisualStudio7Generator::GenerateTarget(cmGeneratorTarget* target)
{
  std::string const& lname = target->GetName();
  cmGlobalVisualStudioGenerator* gg =
    static_cast<cmGlobalVisualStudioGenerator*>(this->GlobalGenerator);
  this->FortranProject = gg->TargetIsFortranOnly(target);
  this->WindowsCEProject = gg->TargetsWindowsCE();

  // Intel Fortran always uses VS9 format ".vfproj" files.
  cmGlobalVisualStudioGenerator::VSVersion realVersion = gg->GetVersion();
  if (this->FortranProject &&
      gg->GetVersion() >= cmGlobalVisualStudioGenerator::VSVersion::VS12) {
    gg->SetVersion(cmGlobalVisualStudioGenerator::VSVersion::VS9);
  }

  // add to the list of projects
  target->Target->SetProperty("GENERATOR_FILE_NAME", lname);
  // create the dsp.cmake file
  std::string fname;
  fname = cmStrCat(this->GetCurrentBinaryDirectory(), '/', lname);
  if (this->FortranProject) {
    fname += ".vfproj";
  } else {
    fname += ".vcproj";
  }

  // Generate the project file and replace it atomically with
  // copy-if-different.  We use a separate timestamp so that the IDE
  // does not reload project files unnecessarily.
  cmGeneratedFileStream fout(fname);
  fout.SetCopyIfDifferent(true);
  this->WriteVCProjFile(fout, lname, target);
  if (fout.Close()) {
    this->GlobalGenerator->FileReplacedDuringGenerate(fname);
  }

  gg->SetVersion(realVersion);
}

cmSourceFile* cmLocalVisualStudio7Generator::CreateVCProjBuildRule()
{
  if (this->GlobalGenerator->GlobalSettingIsOn(
        "CMAKE_SUPPRESS_REGENERATION")) {
    return nullptr;
  }

  std::string makefileIn =
    cmStrCat(this->GetCurrentSourceDirectory(), "/CMakeLists.txt");
  if (cmSourceFile* file = this->Makefile->GetSource(makefileIn)) {
    if (file->GetCustomCommand()) {
      return file;
    }
  }
  if (!cmSystemTools::FileExists(makefileIn)) {
    return nullptr;
  }

  std::vector<std::string> listFiles = this->Makefile->GetListFiles();
  cmake* cm = this->GlobalGenerator->GetCMakeInstance();
  if (cm->DoWriteGlobVerifyTarget()) {
    listFiles.push_back(cm->GetGlobVerifyStamp());
  }

  // Sort the list of input files and remove duplicates.
  std::sort(listFiles.begin(), listFiles.end(), std::less<std::string>());
  auto new_end = std::unique(listFiles.begin(), listFiles.end());
  listFiles.erase(new_end, listFiles.end());

  std::string argS = cmStrCat("-S", this->GetSourceDirectory());
  std::string argB = cmStrCat("-B", this->GetBinaryDirectory());
  std::string stampName =
    cmStrCat(this->GetCurrentBinaryDirectory(), "/CMakeFiles/generate.stamp");
  cmCustomCommandLines commandLines =
    cmMakeSingleCommandLine({ cmSystemTools::GetCMakeCommand(), argS, argB,
                              "--check-stamp-file", stampName });

  if (cm->GetIgnoreWarningAsError()) {
    commandLines[0].emplace_back("--compile-no-warning-as-error");
  }
  std::string comment = cmStrCat("Building Custom Rule ", makefileIn);
  auto cc = cm::make_unique<cmCustomCommand>();
  cc->SetOutputs(stampName);
  cc->SetMainDependency(makefileIn);
  cc->SetDepends(listFiles);
  cc->SetCommandLines(commandLines);
  cc->SetComment(comment.c_str());
  cc->SetEscapeOldStyle(false);
  cc->SetStdPipesUTF8(true);
  cc->SetUsesTerminal(true);
  this->AddCustomCommandToOutput(std::move(cc), true);
  if (cmSourceFile* file = this->Makefile->GetSource(makefileIn)) {
    // Finalize the source file path now since we're adding this after
    // the generator validated all project-named sources.
    file->ResolveFullPath();
    return file;
  }
  cmSystemTools::Error("Error adding rule for " + makefileIn);
  return nullptr;
}

void cmLocalVisualStudio7Generator::WriteConfigurations(
  std::ostream& fout, std::vector<std::string> const& configs,
  const std::string& libName, cmGeneratorTarget* target)
{
  fout << "\t<Configurations>\n";
  for (std::string const& config : configs) {
    this->WriteConfiguration(fout, config, libName, target);
  }
  fout << "\t</Configurations>\n";
}
cmVS7FlagTable cmLocalVisualStudio7GeneratorFortranFlagTable[] = {
  { "Preprocess", "fpp", "Run Preprocessor on files", "preprocessYes", 0 },
  { "Preprocess", "nofpp", "Run Preprocessor on files", "preprocessNo", 0 },
  { "SuppressStartupBanner", "nologo", "SuppressStartupBanner", "true", 0 },
  { "SourceFileFormat", "fixed", "Use Fixed Format", "fileFormatFixed", 0 },
  { "SourceFileFormat", "free", "Use Free Format", "fileFormatFree", 0 },
  { "DebugInformationFormat", "debug:full", "full debug", "debugEnabled", 0 },
  { "DebugInformationFormat", "debug:minimal", "line numbers",
    "debugLineInfoOnly", 0 },
  { "Optimization", "Od", "disable optimization", "optimizeDisabled", 0 },
  { "Optimization", "O1", "min space", "optimizeMinSpace", 0 },
  { "Optimization", "O3", "full optimize", "optimizeFull", 0 },
  { "GlobalOptimizations", "Og", "global optimize", "true", 0 },
  { "InlineFunctionExpansion", "Ob0", "", "expandDisable", 0 },
  { "InlineFunctionExpansion", "Ob1", "", "expandOnlyInline", 0 },
  { "FavorSizeOrSpeed", "Os", "", "favorSize", 0 },
  { "OmitFramePointers", "Oy-", "", "false", 0 },
  { "OptimizeForProcessor", "GB", "", "procOptimizeBlended", 0 },
  { "OptimizeForProcessor", "G5", "", "procOptimizePentium", 0 },
  { "OptimizeForProcessor", "G6", "", "procOptimizePentiumProThruIII", 0 },
  { "UseProcessorExtensions", "QzxK", "", "codeForStreamingSIMD", 0 },
  { "OptimizeForProcessor", "QaxN", "", "codeForPentium4", 0 },
  { "OptimizeForProcessor", "QaxB", "", "codeForPentiumM", 0 },
  { "OptimizeForProcessor", "QaxP", "", "codeForCodeNamedPrescott", 0 },
  { "OptimizeForProcessor", "QaxT", "", "codeForCore2Duo", 0 },
  { "OptimizeForProcessor", "QxK", "", "codeExclusivelyStreamingSIMD", 0 },
  { "OptimizeForProcessor", "QxN", "", "codeExclusivelyPentium4", 0 },
  { "OptimizeForProcessor", "QxB", "", "codeExclusivelyPentiumM", 0 },
  { "OptimizeForProcessor", "QxP", "", "codeExclusivelyCodeNamedPrescott", 0 },
  { "OptimizeForProcessor", "QxT", "", "codeExclusivelyCore2Duo", 0 },
  { "OptimizeForProcessor", "QxO", "", "codeExclusivelyCore2StreamingSIMD",
    0 },
  { "OptimizeForProcessor", "QxS", "", "codeExclusivelyCore2StreamingSIMD4",
    0 },
  { "OpenMP", "Qopenmp", "", "OpenMPParallelCode", 0 },
  { "OpenMP", "Qopenmp-stubs", "", "OpenMPSequentialCode", 0 },
  { "Traceback", "traceback", "", "true", 0 },
  { "Traceback", "notraceback", "", "false", 0 },
  { "FloatingPointExceptionHandling", "fpe:0", "", "fpe0", 0 },
  { "FloatingPointExceptionHandling", "fpe:1", "", "fpe1", 0 },
  { "FloatingPointExceptionHandling", "fpe:3", "", "fpe3", 0 },

  { "MultiProcessorCompilation", "MP", "", "true",
    cmVS7FlagTable::UserValueIgnored | cmVS7FlagTable::Continue },
  { "ProcessorNumber", "MP", "Multi-processor Compilation", "",
    cmVS7FlagTable::UserValueRequired },

  { "ModulePath", "module:", "", "", cmVS7FlagTable::UserValueRequired },
  { "LoopUnrolling", "Qunroll:", "", "", cmVS7FlagTable::UserValueRequired },
  { "AutoParallelThreshold", "Qpar-threshold:", "", "",
    cmVS7FlagTable::UserValueRequired },
  { "HeapArrays", "heap-arrays:", "", "", cmVS7FlagTable::UserValueRequired },
  { "ObjectText", "bintext:", "", "", cmVS7FlagTable::UserValueRequired },
  { "Parallelization", "Qparallel", "", "true", 0 },
  { "PrefetchInsertion", "Qprefetch-", "", "false", 0 },
  { "BufferedIO", "assume:buffered_io", "", "true", 0 },
  { "CallingConvention", "iface:stdcall", "", "callConventionStdCall", 0 },
  { "CallingConvention", "iface:cref", "", "callConventionCRef", 0 },
  { "CallingConvention", "iface:stdref", "", "callConventionStdRef", 0 },
  { "CallingConvention", "iface:stdcall", "", "callConventionStdCall", 0 },
  { "CallingConvention", "iface:cvf", "", "callConventionCVF", 0 },
  { "EnableRecursion", "recursive", "", "true", 0 },
  { "ReentrantCode", "reentrancy", "", "true", 0 },
  // done up to Language
  { "", "", "", "", 0 }
};
// fill the table here currently the comment field is not used for
// anything other than documentation NOTE: Make sure the longer
// commandFlag comes FIRST!
cmVS7FlagTable cmLocalVisualStudio7GeneratorFlagTable[] = {
  // option flags (some flags map to the same option)
  { "BasicRuntimeChecks", "GZ", "Stack frame checks", "1", 0 },
  { "BasicRuntimeChecks", "RTCsu", "Both stack and uninitialized checks", "3",
    0 },
  { "BasicRuntimeChecks", "RTCs", "Stack frame checks", "1", 0 },
  { "BasicRuntimeChecks", "RTCu", "Uninitialized Variables ", "2", 0 },
  { "BasicRuntimeChecks", "RTC1", "Both stack and uninitialized checks", "3",
    0 },
  { "DebugInformationFormat", "Z7", "debug format", "1", 0 },
  { "DebugInformationFormat", "Zd", "debug format", "2", 0 },
  { "DebugInformationFormat", "Zi", "debug format", "3", 0 },
  { "DebugInformationFormat", "ZI", "debug format", "4", 0 },
  { "EnableEnhancedInstructionSet", "arch:SSE2", "Use sse2 instructions", "2",
    0 },
  { "EnableEnhancedInstructionSet", "arch:SSE", "Use sse instructions", "1",
    0 },
  { "FloatingPointModel", "fp:precise", "Use precise floating point model",
    "0", 0 },
  { "FloatingPointModel", "fp:strict", "Use strict floating point model", "1",
    0 },
  { "FloatingPointModel", "fp:fast", "Use fast floating point model", "2", 0 },
  { "FavorSizeOrSpeed", "Ot", "Favor fast code", "1", 0 },
  { "FavorSizeOrSpeed", "Os", "Favor small code", "2", 0 },
  { "CompileAs", "TC", "Compile as c code", "1", 0 },
  { "CompileAs", "TP", "Compile as c++ code", "2", 0 },
  { "Optimization", "Od", "Non Debug", "0", 0 },
  { "Optimization", "O1", "Min Size", "1", 0 },
  { "Optimization", "O2", "Max Speed", "2", 0 },
  { "Optimization", "Ox", "Max Optimization", "3", 0 },
  { "OptimizeForProcessor", "GB", "Blended processor mode", "0", 0 },
  { "OptimizeForProcessor", "G5", "Pentium", "1", 0 },
  { "OptimizeForProcessor", "G6", "PPro PII PIII", "2", 0 },
  { "OptimizeForProcessor", "G7", "Pentium 4 or Athlon", "3", 0 },
  { "InlineFunctionExpansion", "Ob0", "no inlines", "0", 0 },
  { "InlineFunctionExpansion", "Ob1", "when inline keyword", "1", 0 },
  { "InlineFunctionExpansion", "Ob2", "any time you can inline", "2", 0 },
  { "RuntimeLibrary", "MTd", "Multithreaded debug", "1", 0 },
  { "RuntimeLibrary", "MT", "Multithreaded", "0", 0 },
  { "RuntimeLibrary", "MDd", "Multithreaded dll debug", "3", 0 },
  { "RuntimeLibrary", "MD", "Multithreaded dll", "2", 0 },
  { "RuntimeLibrary", "MLd", "Single Thread debug", "5", 0 },
  { "RuntimeLibrary", "ML", "Single Thread", "4", 0 },
  { "StructMemberAlignment", "Zp16", "struct align 16 byte ", "5", 0 },
  { "StructMemberAlignment", "Zp1", "struct align 1 byte ", "1", 0 },
  { "StructMemberAlignment", "Zp2", "struct align 2 byte ", "2", 0 },
  { "StructMemberAlignment", "Zp4", "struct align 4 byte ", "3", 0 },
  { "StructMemberAlignment", "Zp8", "struct align 8 byte ", "4", 0 },
  { "WarningLevel", "W0", "Warning level", "0", 0 },
  { "WarningLevel", "W1", "Warning level", "1", 0 },
  { "WarningLevel", "W2", "Warning level", "2", 0 },
  { "WarningLevel", "W3", "Warning level", "3", 0 },
  { "WarningLevel", "W4", "Warning level", "4", 0 },
  { "DisableSpecificWarnings", "wd", "Disable specific warnings", "",
    cmVS7FlagTable::UserValue | cmVS7FlagTable::SemicolonAppendable },

  // Precompiled header and related options.  Note that the
  // UsePrecompiledHeader entries are marked as "Continue" so that the
  // corresponding PrecompiledHeaderThrough entry can be found.
  { "UsePrecompiledHeader", "Yc", "Create Precompiled Header", "1",
    cmVS7FlagTable::UserValueIgnored | cmVS7FlagTable::Continue },
  { "PrecompiledHeaderThrough", "Yc", "Precompiled Header Name", "",
    cmVS7FlagTable::UserValueRequired },
  { "UsePrecompiledHeader", "Y-", "Don't use precompiled header", "0", 0 },
  { "PrecompiledHeaderFile", "Fp", "Generated Precompiled Header", "",
    cmVS7FlagTable::UserValue },
  // The YX and Yu options are in a per-global-generator table because
  // their values differ based on the VS IDE version.
  { "ForcedIncludeFiles", "FI", "Forced include files", "",
    cmVS7FlagTable::UserValueRequired | cmVS7FlagTable::SemicolonAppendable },

  { "AssemblerListingLocation", "Fa", "ASM List Location", "",
    cmVS7FlagTable::UserValue },
  { "ProgramDataBaseFileName", "Fd", "Program Database File Name", "",
    cmVS7FlagTable::UserValue },

  // boolean flags
  { "BufferSecurityCheck", "GS", "Buffer security check", "true", 0 },
  { "BufferSecurityCheck", "GS-", "Turn off Buffer security check", "false",
    0 },
  { "Detect64BitPortabilityProblems", "Wp64",
    "Detect 64-bit Portability Problems", "true", 0 },
  { "EnableFiberSafeOptimizations", "GT", "Enable Fiber-safe Optimizations",
    "true", 0 },
  { "EnableFunctionLevelLinking", "Gy", "EnableFunctionLevelLinking", "true",
    0 },
  { "EnableIntrinsicFunctions", "Oi", "EnableIntrinsicFunctions", "true", 0 },
  { "GlobalOptimizations", "Og", "Global Optimize", "true", 0 },
  { "ImproveFloatingPointConsistency", "Op", "ImproveFloatingPointConsistency",
    "true", 0 },
  { "MinimalRebuild", "Gm", "minimal rebuild", "true", 0 },
  { "OmitFramePointers", "Oy", "OmitFramePointers", "true", 0 },
  { "OptimizeForWindowsApplication", "GA", "Optimize for windows", "true", 0 },
  { "RuntimeTypeInfo", "GR", "Turn on Run time type information for c++",
    "true", 0 },
  { "RuntimeTypeInfo", "GR-", "Turn off Run time type information for c++",
    "false", 0 },
  { "SmallerTypeCheck", "RTCc", "smaller type check", "true", 0 },
  { "SuppressStartupBanner", "nologo", "SuppressStartupBanner", "true", 0 },
  { "WholeProgramOptimization", "GL", "Enables whole program optimization",
    "true", 0 },
  { "WholeProgramOptimization", "GL-", "Disables whole program optimization",
    "false", 0 },
  { "WarnAsError", "WX", "Treat warnings as errors", "true", 0 },
  { "BrowseInformation", "FR", "Generate browse information", "1", 0 },
  { "StringPooling", "GF", "Enable StringPooling", "true", 0 },
  { "", "", "", "", 0 }
};

cmVS7FlagTable cmLocalVisualStudio7GeneratorLinkFlagTable[] = {
  // option flags (some flags map to the same option)
  { "GenerateManifest", "MANIFEST:NO", "disable manifest generation", "false",
    0 },
  { "GenerateManifest", "MANIFEST", "enable manifest generation", "true", 0 },
  { "LinkIncremental", "INCREMENTAL:NO", "link incremental", "1", 0 },
  { "LinkIncremental", "INCREMENTAL:YES", "link incremental", "2", 0 },
  { "CLRUnmanagedCodeCheck", "CLRUNMANAGEDCODECHECK:NO", "", "false", 0 },
  { "CLRUnmanagedCodeCheck", "CLRUNMANAGEDCODECHECK", "", "true", 0 },
  { "DataExecutionPrevention", "NXCOMPAT:NO",
    "Not known to work with Windows Data Execution Prevention", "1", 0 },
  { "DataExecutionPrevention", "NXCOMPAT",
    "Known to work with Windows Data Execution Prevention", "2", 0 },
  { "DelaySign", "DELAYSIGN:NO", "", "false", 0 },
  { "DelaySign", "DELAYSIGN", "", "true", 0 },
  { "EntryPointSymbol", "ENTRY:", "sets the starting address", "",
    cmVS7FlagTable::UserValue },
  { "IgnoreDefaultLibraryNames", "NODEFAULTLIB:", "default libs to ignore", "",
    cmVS7FlagTable::UserValue | cmVS7FlagTable::SemicolonAppendable },
  { "IgnoreAllDefaultLibraries", "NODEFAULTLIB", "ignore all default libs",
    "true", 0 },
  { "FixedBaseAddress", "FIXED:NO", "Generate a relocation section", "1", 0 },
  { "FixedBaseAddress", "FIXED", "Image must be loaded at a fixed address",
    "2", 0 },
  { "EnableCOMDATFolding", "OPT:NOICF", "Do not remove redundant COMDATs", "1",
    0 },
  { "EnableCOMDATFolding", "OPT:ICF", "Remove redundant COMDATs", "2", 0 },
  { "ResourceOnlyDLL", "NOENTRY", "Create DLL with no entry point", "true",
    0 },
  { "OptimizeReferences", "OPT:NOREF", "Keep unreferenced data", "1", 0 },
  { "OptimizeReferences", "OPT:REF", "Eliminate unreferenced data", "2", 0 },
  { "Profile", "PROFILE", "", "true", 0 },
  { "RandomizedBaseAddress", "DYNAMICBASE:NO",
    "Image may not be rebased at load-time", "1", 0 },
  { "RandomizedBaseAddress", "DYNAMICBASE",
    "Image may be rebased at load-time", "2", 0 },
  { "SetChecksum", "RELEASE", "Enable setting checksum in header", "true", 0 },
  { "SupportUnloadOfDelayLoadedDLL", "DELAY:UNLOAD", "", "true", 0 },
  { "TargetMachine", "MACHINE:I386", "Machine x86", "1", 0 },
  { "TargetMachine", "MACHINE:X86", "Machine x86", "1", 0 },
  { "TargetMachine", "MACHINE:AM33", "Machine AM33", "2", 0 },
  { "TargetMachine", "MACHINE:ARM", "Machine ARM", "3", 0 },
  { "TargetMachine", "MACHINE:EBC", "Machine EBC", "4", 0 },
  { "TargetMachine", "MACHINE:IA64", "Machine IA64", "5", 0 },
  { "TargetMachine", "MACHINE:M32R", "Machine M32R", "6", 0 },
  { "TargetMachine", "MACHINE:MIPS", "Machine MIPS", "7", 0 },
  { "TargetMachine", "MACHINE:MIPS16", "Machine MIPS16", "8", 0 },
  { "TargetMachine", "MACHINE:MIPSFPU)", "Machine MIPSFPU", "9", 0 },
  { "TargetMachine", "MACHINE:MIPSFPU16", "Machine MIPSFPU16", "10", 0 },
  { "TargetMachine", "MACHINE:MIPSR41XX", "Machine MIPSR41XX", "11", 0 },
  { "TargetMachine", "MACHINE:SH3", "Machine SH3", "12", 0 },
  { "TargetMachine", "MACHINE:SH3DSP", "Machine SH3DSP", "13", 0 },
  { "TargetMachine", "MACHINE:SH4", "Machine SH4", "14", 0 },
  { "TargetMachine", "MACHINE:SH5", "Machine SH5", "15", 0 },
  { "TargetMachine", "MACHINE:THUMB", "Machine THUMB", "16", 0 },
  { "TargetMachine", "MACHINE:X64", "Machine x64", "17", 0 },
  { "TargetMachine", "MACHINE:ARM64", "Machine ARM64", "18", 0 },
  { "TurnOffAssemblyGeneration", "NOASSEMBLY",
    "No assembly even if CLR information is present in objects.", "true", 0 },
  { "ModuleDefinitionFile", "DEF:", "add an export def file", "",
    cmVS7FlagTable::UserValue },
  { "GenerateMapFile", "MAP", "enable generation of map file", "true", 0 },
  { "", "", "", "", 0 }
};

cmVS7FlagTable cmLocalVisualStudio7GeneratorFortranLinkFlagTable[] = {
  { "LinkIncremental", "INCREMENTAL:NO", "link incremental",
    "linkIncrementalNo", 0 },
  { "LinkIncremental", "INCREMENTAL:YES", "link incremental",
    "linkIncrementalYes", 0 },
  { "EnableCOMDATFolding", "OPT:NOICF", "Do not remove redundant COMDATs",
    "optNoFolding", 0 },
  { "EnableCOMDATFolding", "OPT:ICF", "Remove redundant COMDATs", "optFolding",
    0 },
  { "OptimizeReferences", "OPT:NOREF", "Keep unreferenced data",
    "optNoReferences", 0 },
  { "OptimizeReferences", "OPT:REF", "Eliminate unreferenced data",
    "optReferences", 0 },
  { "", "", "", "", 0 }
};

// Helper class to write build event <Tool .../> elements.
class cmLocalVisualStudio7Generator::EventWriter
{
public:
  EventWriter(cmLocalVisualStudio7Generator* lg, std::string config,
              std::ostream& os)
    : LG(lg)
    , Config(std::move(config))
    , Stream(os)
  {
  }
  void Start(const char* tool)
  {
    this->First = true;
    this->Stream << "\t\t\t<Tool\n\t\t\t\tName=\"" << tool << "\"";
  }
  void Finish()
  {
    // If any commands were generated, finish constructing them.
    if (!this->First) {
      std::string finishScript =
        this->LG->FinishConstructScript(VsProjectType::vcxproj);
      this->Stream << this->LG->EscapeForXML(finishScript) << "\"";
    }

    this->Stream << "/>\n";
  }
  void Write(std::vector<cmCustomCommand> const& ccs)
  {
    for (cmCustomCommand const& command : ccs) {
      this->Write(command);
    }
  }
  void Write(cmCustomCommand const& cc)
  {
    cmCustomCommandGenerator ccg(cc, this->Config, this->LG);
    if (this->First) {
      if (cm::optional<std::string> comment = ccg.GetComment()) {
        this->Stream << "\nDescription=\"" << this->LG->EscapeForXML(*comment)
                     << "\"";
      }
      this->Stream << "\nCommandLine=\"";
      this->First = false;
    } else {
      this->Stream << this->LG->EscapeForXML("\n");
    }
    std::string script = this->LG->ConstructScript(ccg);
    this->Stream << this->LG->EscapeForXML(script);
  }

private:
  cmLocalVisualStudio7Generator* LG;
  std::string Config;
  std::ostream& Stream;
  bool First = true;
};

void cmLocalVisualStudio7Generator::WriteConfiguration(
  std::ostream& fout, const std::string& configName,
  const std::string& libName, cmGeneratorTarget* target)
{
  std::string mfcFlag;
  if (cmValue p = this->Makefile->GetDefinition("CMAKE_MFC_FLAG")) {
    mfcFlag = cmGeneratorExpression::Evaluate(*p, this, configName);
  } else {
    mfcFlag = "0";
  }
  cmGlobalVisualStudio7Generator* gg =
    static_cast<cmGlobalVisualStudio7Generator*>(this->GlobalGenerator);
  fout << "\t\t<Configuration\n"
       << "\t\t\tName=\"" << configName << "|" << gg->GetPlatformName()
       << "\"\n";
  // This is an internal type to Visual Studio, it seems that:
  // 4 == static library
  // 2 == dll
  // 1 == executable
  // 10 == utility
  const char* configType = "10";
  const char* projectType = nullptr;
  bool targetBuilds = true;

  switch (target->GetType()) {
    case cmStateEnums::OBJECT_LIBRARY:
      targetBuilds = false; // no manifest tool for object library
      CM_FALLTHROUGH;
    case cmStateEnums::STATIC_LIBRARY:
      projectType = "typeStaticLibrary";
      configType = "4";
      break;
    case cmStateEnums::SHARED_LIBRARY:
    case cmStateEnums::MODULE_LIBRARY:
      projectType = "typeDynamicLibrary";
      configType = "2";
      break;
    case cmStateEnums::EXECUTABLE:
      configType = "1";
      break;
    case cmStateEnums::UTILITY:
    case cmStateEnums::GLOBAL_TARGET:
    case cmStateEnums::INTERFACE_LIBRARY:
      configType = "10";
      CM_FALLTHROUGH;
    case cmStateEnums::UNKNOWN_LIBRARY:
      targetBuilds = false;
      break;
  }
  if (this->FortranProject && projectType) {
    configType = projectType;
  }
  std::string flags;
  std::string langForClCompile;
  if (target->GetType() <= cmStateEnums::OBJECT_LIBRARY) {
    const std::string& linkLanguage =
      (this->FortranProject ? std::string("Fortran")
                            : target->GetLinkerLanguage(configName));
    if (linkLanguage.empty()) {
      cmSystemTools::Error(
        "CMake can not determine linker language for target: " +
        target->GetName());
      return;
    }
    langForClCompile = linkLanguage;
    if (langForClCompile == "C" || langForClCompile == "CXX" ||
        langForClCompile == "Fortran") {
      this->AddLanguageFlags(flags, target, cmBuildStep::Compile,
                             langForClCompile, configName);
    }
    // set the correct language
    if (linkLanguage == "C") {
      flags += " /TC ";
    }
    if (linkLanguage == "CXX") {
      flags += " /TP ";
    }

    // Add the target-specific flags.
    this->AddCompileOptions(flags, target, langForClCompile, configName);

    // Check IPO related warning/error.
    target->IsIPOEnabled(linkLanguage, configName);
  }

  if (this->FortranProject) {
    switch (cmOutputConverter::GetFortranFormat(
      target->GetSafeProperty("Fortran_FORMAT"))) {
      case cmOutputConverter::FortranFormatFixed:
        flags += " -fixed";
        break;
      case cmOutputConverter::FortranFormatFree:
        flags += " -free";
        break;
      default:
        break;
    }

    switch (cmOutputConverter::GetFortranPreprocess(
      target->GetSafeProperty("Fortran_PREPROCESS"))) {
      case cmOutputConverter::FortranPreprocess::Needed:
        flags += " -fpp";
        break;
      case cmOutputConverter::FortranPreprocess::NotNeeded:
        flags += " -nofpp";
        break;
      default:
        break;
    }
  }

  // Get preprocessor definitions for this directory.
  std::string defineFlags = this->Makefile->GetDefineFlags();
  Options::Tool t = Options::Compiler;
  cmVS7FlagTable const* table = cmLocalVisualStudio7GeneratorFlagTable;
  if (this->FortranProject) {
    t = Options::FortranCompiler;
    table = cmLocalVisualStudio7GeneratorFortranFlagTable;
  }
  Options targetOptions(this, t, table, gg->ExtraFlagTable);
  targetOptions.FixExceptionHandlingDefault();
  targetOptions.AddFlag("AssemblerListingLocation", "$(IntDir)\\");
  targetOptions.Parse(flags);
  targetOptions.Parse(defineFlags);
  targetOptions.ParseFinish();
  if (!langForClCompile.empty()) {
    std::vector<std::string> targetDefines;
    target->GetCompileDefinitions(targetDefines, configName, langForClCompile);
    targetOptions.AddDefines(targetDefines);

    std::vector<std::string> targetIncludes;
    this->GetIncludeDirectories(targetIncludes, target, langForClCompile,
                                configName);
    targetOptions.AddIncludes(targetIncludes);
  }
  targetOptions.SetVerboseMakefile(
    this->Makefile->IsOn("CMAKE_VERBOSE_MAKEFILE"));

  // Add a definition for the configuration name.
  std::string configDefine = cmStrCat("CMAKE_INTDIR=\"", configName, '"');
  targetOptions.AddDefine(configDefine);

  // Add the export symbol definition for shared library objects.
  if (const std::string* exportMacro = target->GetExportMacro()) {
    targetOptions.AddDefine(*exportMacro);
  }

  // The intermediate directory name consists of a directory for the
  // target and a subdirectory for the configuration name.
  std::string intermediateDir =
    cmStrCat(this->GetTargetDirectory(target), '/', configName);

  if (target->GetType() < cmStateEnums::UTILITY) {
    std::string const& outDir =
      target->GetType() == cmStateEnums::OBJECT_LIBRARY
      ? intermediateDir
      : target->GetDirectory(configName);
    /* clang-format off */
    fout << "\t\t\tOutputDirectory=\""
         << this->ConvertToXMLOutputPathSingle(outDir) << "\"\n";
    /* clang-format on */
  }

  /* clang-format off */
  fout << "\t\t\tIntermediateDirectory=\""
       << this->ConvertToXMLOutputPath(intermediateDir)
       << "\"\n"
       << "\t\t\tConfigurationType=\"" << configType << "\"\n"
       << "\t\t\tUseOfMFC=\"" << mfcFlag << "\"\n"
       << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"false\"\n";
  /* clang-format on */

  if (this->FortranProject) {
    // Intel Fortran >= 15.0 uses TargetName property.
    std::string const targetNameFull = target->GetFullName(configName);
    std::string const targetName =
      cmSystemTools::GetFilenameWithoutLastExtension(targetNameFull);
    std::string const targetExt =
      target->GetType() == cmStateEnums::OBJECT_LIBRARY
      ? ".lib"
      : cmSystemTools::GetFilenameLastExtension(targetNameFull);
    /* clang-format off */
    fout <<
      "\t\t\tTargetName=\"" << this->EscapeForXML(targetName) << "\"\n"
      "\t\t\tTargetExt=\"" << this->EscapeForXML(targetExt) << "\"\n"
      ;
    /* clang-format on */
  }

  // If unicode is enabled change the character set to unicode, if not
  // then default to MBCS.
  if (targetOptions.UsingUnicode()) {
    fout << "\t\t\tCharacterSet=\"1\">\n";
  } else if (targetOptions.UsingSBCS()) {
    fout << "\t\t\tCharacterSet=\"0\">\n";
  } else {
    fout << "\t\t\tCharacterSet=\"2\">\n";
  }
  const char* tool = "VCCLCompilerTool";
  if (this->FortranProject) {
    tool = "VFFortranCompilerTool";
  }
  fout << "\t\t\t<Tool\n"
       << "\t\t\t\tName=\"" << tool << "\"\n";
  if (this->FortranProject) {
    cmValue target_mod_dir = target->GetProperty("Fortran_MODULE_DIRECTORY");
    std::string modDir;
    if (target_mod_dir) {
      modDir = this->MaybeRelativeToCurBinDir(*target_mod_dir);
    } else {
      modDir = ".";
    }
    fout << "\t\t\t\tModulePath=\"" << this->ConvertToXMLOutputPath(modDir)
         << "\\$(ConfigurationName)\"\n";
  }
  targetOptions.OutputAdditionalIncludeDirectories(
    fout, 4, this->FortranProject ? "Fortran" : langForClCompile);
  targetOptions.OutputFlagMap(fout, 4);
  targetOptions.OutputPreprocessorDefinitions(fout, 4, langForClCompile);
  fout << "\t\t\t\tObjectFile=\"$(IntDir)\\\"\n";
  if (target->GetType() <= cmStateEnums::OBJECT_LIBRARY) {
    // Specify the compiler program database file if configured.
    std::string pdb = target->GetCompilePDBPath(configName);
    if (!pdb.empty()) {
      fout << "\t\t\t\tProgramDataBaseFileName=\""
           << this->ConvertToXMLOutputPathSingle(pdb) << "\"\n";
    }
  }
  fout << "/>\n"; // end of <Tool Name=VCCLCompilerTool
  if (gg->IsMarmasmEnabled() && !this->FortranProject) {
    Options marmasmOptions(this, Options::MarmasmCompiler, nullptr, nullptr);
    /* clang-format off */
    fout <<
      "\t\t\t<Tool\n"
      "\t\t\t\tName=\"MARMASM\"\n"
      ;
    /* clang-format on */
    targetOptions.OutputAdditionalIncludeDirectories(fout, 4, "ASM_MARMASM");
    // Use same preprocessor definitions as VCCLCompilerTool.
    targetOptions.OutputPreprocessorDefinitions(fout, 4, "ASM_MARMASM");
    marmasmOptions.OutputFlagMap(fout, 4);
    /* clang-format off */
    fout <<
      "\t\t\t\tObjectFile=\"$(IntDir)\\\"\n"
      "\t\t\t/>\n";
    /* clang-format on */
  }
  if (gg->IsMasmEnabled() && !this->FortranProject) {
    Options masmOptions(this, Options::MasmCompiler, nullptr, nullptr);
    /* clang-format off */
    fout <<
      "\t\t\t<Tool\n"
      "\t\t\t\tName=\"MASM\"\n"
      ;
    /* clang-format on */
    targetOptions.OutputAdditionalIncludeDirectories(fout, 4, "ASM_MASM");
    // Use same preprocessor definitions as VCCLCompilerTool.
    targetOptions.OutputPreprocessorDefinitions(fout, 4, "ASM_MASM");
    masmOptions.OutputFlagMap(fout, 4);
    /* clang-format off */
    fout <<
      "\t\t\t\tObjectFile=\"$(IntDir)\\\"\n"
      "\t\t\t/>\n";
    /* clang-format on */
  }
  tool = "VCCustomBuildTool";
  if (this->FortranProject) {
    tool = "VFCustomBuildTool";
  }
  fout << "\t\t\t<Tool\n\t\t\t\tName=\"" << tool << "\"/>\n";
  tool = "VCResourceCompilerTool";
  if (this->FortranProject) {
    tool = "VFResourceCompilerTool";
  }
  fout << "\t\t\t<Tool\n\t\t\t\tName=\"" << tool << "\"\n";
  targetOptions.OutputAdditionalIncludeDirectories(fout, 4, "RC");
  // add the -D flags to the RC tool
  targetOptions.OutputPreprocessorDefinitions(fout, 4, "RC");
  fout << "\t\t\t/>\n";
  tool = "VCMIDLTool";
  if (this->FortranProject) {
    tool = "VFMIDLTool";
  }
  fout << "\t\t\t<Tool\n\t\t\t\tName=\"" << tool << "\"\n";
  targetOptions.OutputAdditionalIncludeDirectories(fout, 4, "MIDL");
  fout << "\t\t\t\tMkTypLibCompatible=\"false\"\n";
  if (gg->GetPlatformName() == "x64") {
    fout << "\t\t\t\tTargetEnvironment=\"3\"\n";
  } else if (gg->GetPlatformName() == "ia64") {
    fout << "\t\t\t\tTargetEnvironment=\"2\"\n";
  } else {
    fout << "\t\t\t\tTargetEnvironment=\"1\"\n";
  }
  fout << "\t\t\t\tGenerateStublessProxies=\"true\"\n";
  fout << "\t\t\t\tTypeLibraryName=\"$(InputName).tlb\"\n";
  fout << "\t\t\t\tOutputDirectory=\"$(IntDir)\"\n";
  fout << "\t\t\t\tHeaderFileName=\"$(InputName).h\"\n";
  fout << "\t\t\t\tDLLDataFileName=\"\"\n";
  fout << "\t\t\t\tInterfaceIdentifierFileName=\"$(InputName)_i.c\"\n";
  fout << "\t\t\t\tProxyFileName=\"$(InputName)_p.c\"/>\n";
  // end of <Tool Name=VCMIDLTool

  // Add manifest tool settings.
  if (targetBuilds) {
    const char* manifestTool = "VCManifestTool";
    if (this->FortranProject) {
      manifestTool = "VFManifestTool";
    }
    /* clang-format off */
    fout <<
      "\t\t\t<Tool\n"
      "\t\t\t\tName=\"" << manifestTool << "\"";
    /* clang-format on */

    std::vector<cmSourceFile const*> manifest_srcs;
    target->GetManifests(manifest_srcs, configName);
    if (!manifest_srcs.empty()) {
      fout << "\n\t\t\t\tAdditionalManifestFiles=\"";
      for (cmSourceFile const* manifest : manifest_srcs) {
        std::string m = manifest->GetFullPath();
        fout << this->ConvertToXMLOutputPath(m) << ";";
      }
      fout << "\"";
    }

    // Check if we need the FAT32 workaround.
    // Check the filesystem type where the target will be written.
    if (cmLVS7G_IsFAT(target->GetDirectory(configName).c_str())) {
      // Add a flag telling the manifest tool to use a workaround
      // for FAT32 file systems, which can cause an empty manifest
      // to be embedded into the resulting executable.  See CMake
      // bug #2617.
      fout << "\n\t\t\t\tUseFAT32Workaround=\"true\"";
    }
    fout << "/>\n";
  }

  this->OutputTargetRules(fout, configName, target, libName);
  this->OutputBuildTool(fout, configName, target, targetOptions);
  this->OutputDeploymentDebuggerTool(fout, configName, target);
  fout << "\t\t</Configuration>\n";
}

std::string cmLocalVisualStudio7Generator::GetBuildTypeLinkerFlags(
  std::string const& rootLinkerFlags, const std::string& configName)
{
  std::string configTypeUpper = cmSystemTools::UpperCase(configName);
  std::string extraLinkOptionsBuildTypeDef =
    rootLinkerFlags + "_" + configTypeUpper;

  const std::string& extraLinkOptionsBuildType =
    this->Makefile->GetRequiredDefinition(extraLinkOptionsBuildTypeDef);

  return extraLinkOptionsBuildType;
}

void cmLocalVisualStudio7Generator::OutputBuildTool(
  std::ostream& fout, const std::string& configName, cmGeneratorTarget* target,
  const Options& targetOptions)
{
  cmGlobalVisualStudio7Generator* gg =
    static_cast<cmGlobalVisualStudio7Generator*>(this->GlobalGenerator);
  std::string temp;
  std::string extraLinkOptions;
  if (target->GetType() == cmStateEnums::EXECUTABLE) {
    extraLinkOptions =
      this->Makefile->GetRequiredDefinition("CMAKE_EXE_LINKER_FLAGS") + " " +
      GetBuildTypeLinkerFlags("CMAKE_EXE_LINKER_FLAGS", configName);
  }
  if (target->GetType() == cmStateEnums::SHARED_LIBRARY) {
    extraLinkOptions =
      this->Makefile->GetRequiredDefinition("CMAKE_SHARED_LINKER_FLAGS") +
      " " + GetBuildTypeLinkerFlags("CMAKE_SHARED_LINKER_FLAGS", configName);
  }
  if (target->GetType() == cmStateEnums::MODULE_LIBRARY) {
    extraLinkOptions =
      this->Makefile->GetRequiredDefinition("CMAKE_MODULE_LINKER_FLAGS") +
      " " + GetBuildTypeLinkerFlags("CMAKE_MODULE_LINKER_FLAGS", configName);
  }

  cmValue targetLinkFlags = target->GetProperty("LINK_FLAGS");
  if (targetLinkFlags) {
    extraLinkOptions += " ";
    extraLinkOptions += *targetLinkFlags;
  }
  std::string configTypeUpper = cmSystemTools::UpperCase(configName);
  std::string linkFlagsConfig = cmStrCat("LINK_FLAGS_", configTypeUpper);
  targetLinkFlags = target->GetProperty(linkFlagsConfig);
  if (targetLinkFlags) {
    extraLinkOptions += " ";
    extraLinkOptions += *targetLinkFlags;
  }

  std::vector<std::string> opts;
  target->GetLinkOptions(opts, configName,
                         target->GetLinkerLanguage(configName));
  // LINK_OPTIONS are escaped.
  this->AppendCompileOptions(extraLinkOptions, opts);

  Options linkOptions(this, Options::Linker);
  if (this->FortranProject) {
    linkOptions.AddTable(cmLocalVisualStudio7GeneratorFortranLinkFlagTable);
  }
  linkOptions.AddTable(cmLocalVisualStudio7GeneratorLinkFlagTable);

  linkOptions.Parse(extraLinkOptions);
  cmGeneratorTarget::ModuleDefinitionInfo const* mdi =
    target->GetModuleDefinitionInfo(configName);
  if (mdi && !mdi->DefFile.empty()) {
    std::string defFile =
      this->ConvertToOutputFormat(mdi->DefFile, cmOutputConverter::SHELL);
    linkOptions.AddFlag("ModuleDefinitionFile", defFile);
  }

  switch (target->GetType()) {
    case cmStateEnums::UNKNOWN_LIBRARY:
      break;
    case cmStateEnums::OBJECT_LIBRARY: {
      std::string libpath =
        cmStrCat(this->GetTargetDirectory(target), '/', configName, '/',
                 target->GetName(), ".lib");
      const char* tool =
        this->FortranProject ? "VFLibrarianTool" : "VCLibrarianTool";
      fout << "\t\t\t<Tool\n"
           << "\t\t\t\tName=\"" << tool << "\"\n";
      fout << "\t\t\t\tOutputFile=\""
           << this->ConvertToXMLOutputPathSingle(libpath) << "\"/>\n";
      break;
    }
    case cmStateEnums::STATIC_LIBRARY: {
      std::string targetNameFull = target->GetFullName(configName);
      std::string libpath =
        cmStrCat(target->GetDirectory(configName), '/', targetNameFull);
      const char* tool = "VCLibrarianTool";
      if (this->FortranProject) {
        tool = "VFLibrarianTool";
      }
      fout << "\t\t\t<Tool\n"
           << "\t\t\t\tName=\"" << tool << "\"\n";

      if (this->FortranProject) {
        std::ostringstream libdeps;
        this->Internal->OutputObjects(libdeps, target, configName);
        if (!libdeps.str().empty()) {
          fout << "\t\t\t\tAdditionalDependencies=\"" << libdeps.str()
               << "\"\n";
        }
      }
      std::string libflags;
      this->GetStaticLibraryFlags(
        libflags, configName, target->GetLinkerLanguage(configName), target);
      if (!libflags.empty()) {
        fout << "\t\t\t\tAdditionalOptions=\"" << this->EscapeForXML(libflags)
             << "\"\n";
      }
      fout << "\t\t\t\tOutputFile=\""
           << this->ConvertToXMLOutputPathSingle(libpath) << "\"/>\n";
      break;
    }
    case cmStateEnums::SHARED_LIBRARY:
    case cmStateEnums::MODULE_LIBRARY: {
      cmGeneratorTarget::Names targetNames =
        target->GetLibraryNames(configName);

      // Compute the link library and directory information.
      cmComputeLinkInformation* pcli = target->GetLinkInformation(configName);
      if (!pcli) {
        return;
      }
      cmComputeLinkInformation& cli = *pcli;
      std::string linkLanguage = cli.GetLinkLanguage();

      // Compute the variable name to lookup standard libraries for this
      // language.
      std::string standardLibsVar =
        cmStrCat("CMAKE_", linkLanguage, "_STANDARD_LIBRARIES");
      const char* tool = "VCLinkerTool";
      if (this->FortranProject) {
        tool = "VFLinkerTool";
      }
      fout << "\t\t\t<Tool\n"
           << "\t\t\t\tName=\"" << tool << "\"\n";
      if (!gg->NeedLinkLibraryDependencies(target)) {
        fout << "\t\t\t\tLinkLibraryDependencies=\"false\"\n";
      }
      // Use the NOINHERIT macro to avoid getting VS project default
      // libraries which may be set by the user to something bad.
      fout << "\t\t\t\tAdditionalDependencies=\"$(NOINHERIT) "
           << this->Makefile->GetSafeDefinition(standardLibsVar);
      if (this->FortranProject) {
        this->Internal->OutputObjects(fout, target, configName, " ");
      }
      fout << " ";
      this->Internal->OutputLibraries(fout, cli.GetItems());
      fout << "\"\n";
      temp =
        cmStrCat(target->GetDirectory(configName), '/', targetNames.Output);
      fout << "\t\t\t\tOutputFile=\""
           << this->ConvertToXMLOutputPathSingle(temp) << "\"\n";
      this->WriteTargetVersionAttribute(fout, target);
      linkOptions.OutputFlagMap(fout, 4);
      fout << "\t\t\t\tAdditionalLibraryDirectories=\"";
      this->OutputLibraryDirectories(fout, cli.GetDirectories());
      fout << "\"\n";
      temp =
        cmStrCat(target->GetPDBDirectory(configName), '/', targetNames.PDB);
      fout << "\t\t\t\tProgramDatabaseFile=\""
           << this->ConvertToXMLOutputPathSingle(temp) << "\"\n";
      if (targetOptions.IsDebug()) {
        fout << "\t\t\t\tGenerateDebugInformation=\"true\"\n";
      }
      if (this->WindowsCEProject) {
        if (this->GetVersion() <
            cmGlobalVisualStudioGenerator::VSVersion::VS9) {
          fout << "\t\t\t\tSubSystem=\"9\"\n";
        } else {
          fout << "\t\t\t\tSubSystem=\"8\"\n";
        }
      }
      std::string stackVar = cmStrCat("CMAKE_", linkLanguage, "_STACK_SIZE");
      cmValue stackVal = this->Makefile->GetDefinition(stackVar);
      if (stackVal) {
        fout << "\t\t\t\tStackReserveSize=\"" << *stackVal << "\"\n";
      }
      if (!targetNames.ImportLibrary.empty()) {
        temp = cmStrCat(target->GetDirectory(
                          configName, cmStateEnums::ImportLibraryArtifact),
                        '/', targetNames.ImportLibrary);
        fout << "\t\t\t\tImportLibrary=\""
             << this->ConvertToXMLOutputPathSingle(temp) << "\"";
      }
      if (this->FortranProject) {
        fout << "\n\t\t\t\tLinkDLL=\"true\"";
      }
      fout << "/>\n";
    } break;
    case cmStateEnums::EXECUTABLE: {
      cmGeneratorTarget::Names targetNames =
        target->GetExecutableNames(configName);

      // Compute the link library and directory information.
      cmComputeLinkInformation* pcli = target->GetLinkInformation(configName);
      if (!pcli) {
        return;
      }
      cmComputeLinkInformation& cli = *pcli;
      std::string linkLanguage = cli.GetLinkLanguage();

      bool isWin32Executable = target->IsWin32Executable(configName);

      // Compute the variable name to lookup standard libraries for this
      // language.
      std::string standardLibsVar =
        cmStrCat("CMAKE_", linkLanguage, "_STANDARD_LIBRARIES");
      const char* tool = "VCLinkerTool";
      if (this->FortranProject) {
        tool = "VFLinkerTool";
      }
      fout << "\t\t\t<Tool\n"
           << "\t\t\t\tName=\"" << tool << "\"\n";
      if (!gg->NeedLinkLibraryDependencies(target)) {
        fout << "\t\t\t\tLinkLibraryDependencies=\"false\"\n";
      }
      // Use the NOINHERIT macro to avoid getting VS project default
      // libraries which may be set by the user to something bad.
      fout << "\t\t\t\tAdditionalDependencies=\"$(NOINHERIT) "
           << this->Makefile->GetSafeDefinition(standardLibsVar);
      if (this->FortranProject) {
        this->Internal->OutputObjects(fout, target, configName, " ");
      }
      fout << " ";
      this->Internal->OutputLibraries(fout, cli.GetItems());
      fout << "\"\n";
      temp =
        cmStrCat(target->GetDirectory(configName), '/', targetNames.Output);
      fout << "\t\t\t\tOutputFile=\""
           << this->ConvertToXMLOutputPathSingle(temp) << "\"\n";
      this->WriteTargetVersionAttribute(fout, target);
      linkOptions.OutputFlagMap(fout, 4);
      fout << "\t\t\t\tAdditionalLibraryDirectories=\"";
      this->OutputLibraryDirectories(fout, cli.GetDirectories());
      fout << "\"\n";
      std::string path = this->ConvertToXMLOutputPathSingle(
        target->GetPDBDirectory(configName));
      fout << "\t\t\t\tProgramDatabaseFile=\"" << path << "/"
           << targetNames.PDB << "\"\n";
      if (targetOptions.IsDebug()) {
        fout << "\t\t\t\tGenerateDebugInformation=\"true\"\n";
      }
      if (this->WindowsCEProject) {
        if (this->GetVersion() <
            cmGlobalVisualStudioGenerator::VSVersion::VS9) {
          fout << "\t\t\t\tSubSystem=\"9\"\n";
        } else {
          fout << "\t\t\t\tSubSystem=\"8\"\n";
        }

        if (!linkOptions.GetFlag("EntryPointSymbol")) {
          const char* entryPointSymbol = targetOptions.UsingUnicode()
            ? (isWin32Executable ? "wWinMainCRTStartup" : "mainWCRTStartup")
            : (isWin32Executable ? "WinMainCRTStartup" : "mainACRTStartup");
          fout << "\t\t\t\tEntryPointSymbol=\"" << entryPointSymbol << "\"\n";
        }
      } else if (this->FortranProject) {
        fout << "\t\t\t\tSubSystem=\""
             << (isWin32Executable ? "subSystemWindows" : "subSystemConsole")
             << "\"\n";
      } else {
        fout << "\t\t\t\tSubSystem=\"" << (isWin32Executable ? "2" : "1")
             << "\"\n";
      }
      std::string stackVar = cmStrCat("CMAKE_", linkLanguage, "_STACK_SIZE");
      cmValue stackVal = this->Makefile->GetDefinition(stackVar);
      if (stackVal) {
        fout << "\t\t\t\tStackReserveSize=\"" << *stackVal << "\"";
      }
      temp = cmStrCat(
        target->GetDirectory(configName, cmStateEnums::ImportLibraryArtifact),
        '/', targetNames.ImportLibrary);
      fout << "\t\t\t\tImportLibrary=\""
           << this->ConvertToXMLOutputPathSingle(temp) << "\"/>\n";
      break;
    }
    case cmStateEnums::UTILITY:
    case cmStateEnums::GLOBAL_TARGET:
    case cmStateEnums::INTERFACE_LIBRARY:
      break;
  }
}

static std::string cmLocalVisualStudio7GeneratorEscapeForXML(
  const std::string& s)
{
  std::string ret = s;
  cmSystemTools::ReplaceString(ret, "&", "&amp;");
  cmSystemTools::ReplaceString(ret, "\"", "&quot;");
  cmSystemTools::ReplaceString(ret, "<", "&lt;");
  cmSystemTools::ReplaceString(ret, ">", "&gt;");
  cmSystemTools::ReplaceString(ret, "\n", "&#x0D;&#x0A;");
  return ret;
}

static std::string GetEscapedPropertyIfValueNotNULL(const char* propertyValue)
{
  return propertyValue == nullptr
    ? std::string()
    : cmLocalVisualStudio7GeneratorEscapeForXML(propertyValue);
}

void cmLocalVisualStudio7Generator::OutputDeploymentDebuggerTool(
  std::ostream& fout, std::string const& config, cmGeneratorTarget* target)
{
  if (this->WindowsCEProject) {
    cmValue dir = target->GetProperty("DEPLOYMENT_REMOTE_DIRECTORY");
    cmValue additionalFiles =
      target->GetProperty("DEPLOYMENT_ADDITIONAL_FILES");

    if (!dir && !additionalFiles) {
      return;
    }

    fout << "\t\t\t<DeploymentTool\n"
            "\t\t\t\tForceDirty=\"-1\"\n"
            "\t\t\t\tRemoteDirectory=\""
         << GetEscapedPropertyIfValueNotNULL(dir->c_str())
         << "\"\n"
            "\t\t\t\tRegisterOutput=\"0\"\n"
            "\t\t\t\tAdditionalFiles=\""
         << GetEscapedPropertyIfValueNotNULL(additionalFiles->c_str())
         << "\"/>\n";

    if (dir) {
      std::string const exe = *dir + "\\" + target->GetFullName(config);

      fout << "\t\t\t<DebuggerTool\n"
              "\t\t\t\tRemoteExecutable=\""
           << this->EscapeForXML(exe)
           << "\"\n"
              "\t\t\t\tArguments=\"\"\n"
              "\t\t\t/>\n";
    }
  }
}

void cmLocalVisualStudio7Generator::WriteTargetVersionAttribute(
  std::ostream& fout, cmGeneratorTarget* gt)
{
  int major;
  int minor;
  gt->GetTargetVersion(major, minor);
  fout << "\t\t\t\tVersion=\"" << major << "." << minor << "\"\n";
}

void cmLocalVisualStudio7GeneratorInternals::OutputLibraries(
  std::ostream& fout, ItemVector const& libs)
{
  cmLocalVisualStudio7Generator* lg = this->LocalGenerator;
  for (auto const& lib : libs) {
    if (lib.IsPath == cmComputeLinkInformation::ItemIsPath::Yes) {
      std::string rel = lg->MaybeRelativeToCurBinDir(lib.Value.Value);
      rel = lg->ConvertToXMLOutputPath(rel);
      fout << (lib.HasFeature() ? lib.GetFormattedItem(rel).Value : rel)
           << " ";
    } else if (!lib.Target ||
               lib.Target->GetType() != cmStateEnums::INTERFACE_LIBRARY) {
      fout << lib.Value.Value << " ";
    }
  }
}

void cmLocalVisualStudio7GeneratorInternals::OutputObjects(
  std::ostream& fout, cmGeneratorTarget* gt, std::string const& configName,
  const char* isep)
{
  // VS < 8 does not support per-config source locations so we
  // list object library content on the link line instead.
  cmLocalVisualStudio7Generator* lg = this->LocalGenerator;

  std::vector<cmSourceFile const*> objs;
  gt->GetExternalObjects(objs, configName);

  const char* sep = isep ? isep : "";
  for (cmSourceFile const* obj : objs) {
    if (!obj->GetObjectLibrary().empty()) {
      std::string const& objFile = obj->GetFullPath();
      std::string rel = lg->MaybeRelativeToCurBinDir(objFile);
      fout << sep << lg->ConvertToXMLOutputPath(rel);
      sep = " ";
    }
  }
}

void cmLocalVisualStudio7Generator::OutputLibraryDirectories(
  std::ostream& fout, std::vector<std::string> const& dirs)
{
  const char* comma = "";
  for (std::string dir : dirs) {
    // Remove any trailing slash and skip empty paths.
    if (dir.back() == '/') {
      dir = dir.substr(0, dir.size() - 1);
    }
    if (dir.empty()) {
      continue;
    }

    // Switch to a relative path specification if it is shorter.
    if (cmSystemTools::FileIsFullPath(dir)) {
      std::string rel = this->MaybeRelativeToCurBinDir(dir);
      if (rel.size() < dir.size()) {
        dir = rel;
      }
    }

    // First search a configuration-specific subdirectory and then the
    // original directory.
    fout << comma
         << this->ConvertToXMLOutputPath(dir + "/$(ConfigurationName)") << ","
         << this->ConvertToXMLOutputPath(dir);
    comma = ",";
  }
}

void cmLocalVisualStudio7Generator::WriteVCProjFile(std::ostream& fout,
                                                    const std::string& libName,
                                                    cmGeneratorTarget* target)
{
  std::vector<std::string> configs =
    this->Makefile->GetGeneratorConfigs(cmMakefile::ExcludeEmptyConfig);

  // We may be modifying the source groups temporarily, so make a copy.
  std::vector<cmSourceGroup> sourceGroups = this->Makefile->GetSourceGroups();

  AllConfigSources sources;
  sources.Sources = target->GetAllConfigSources();

  // Add CMakeLists.txt file with rule to re-run CMake for user convenience.
  if (target->GetType() != cmStateEnums::GLOBAL_TARGET &&
      target->GetName() != CMAKE_CHECK_BUILD_SYSTEM_TARGET) {
    if (cmSourceFile* sf = this->CreateVCProjBuildRule()) {
      cmGeneratorTarget::AllConfigSource acs;
      acs.Source = sf;
      acs.Kind = cmGeneratorTarget::SourceKindCustomCommand;
      for (size_t ci = 0; ci < configs.size(); ++ci) {
        acs.Configs.push_back(ci);
      }
      bool haveCMakeLists = false;
      for (cmGeneratorTarget::AllConfigSource& si : sources.Sources) {
        if (si.Source == sf) {
          haveCMakeLists = true;
          // Replace the explicit source reference with our generated one.
          si = acs;
          break;
        }
      }
      if (!haveCMakeLists) {
        sources.Sources.emplace_back(std::move(acs));
      }
    }
  }

  for (size_t si = 0; si < sources.Sources.size(); ++si) {
    cmSourceFile const* sf = sources.Sources[si].Source;
    sources.Index[sf] = si;
    if (!sf->GetObjectLibrary().empty()) {
      if (this->FortranProject) {
        // Intel Fortran does not support per-config source locations
        // so we list object library content on the link line instead.
        // See OutputObjects.
        continue;
      }
    }
    // Add the file to the list of sources.
    std::string const source = sf->GetFullPath();
    cmSourceGroup* sourceGroup =
      this->Makefile->FindSourceGroup(source, sourceGroups);
    sourceGroup->AssignSource(sf);
  }

  // open the project
  this->WriteProjectStart(fout, libName, target, sourceGroups);
  // write the configuration information
  this->WriteConfigurations(fout, configs, libName, target);

  fout << "\t<Files>\n";

  // Loop through every source group.
  for (auto const& sg : sourceGroups) {
    this->WriteGroup(&sg, target, fout, libName, configs, sources);
  }

  fout << "\t</Files>\n";

  // Write the VCProj file's footer.
  this->WriteVCProjFooter(fout, target);
}

struct cmLVS7GFileConfig
{
  std::string ObjectName;
  std::string CompileFlags;
  std::string CompileDefs;
  std::string CompileDefsConfig;
  std::string AdditionalDeps;
  std::string IncludeDirs;
  bool ExcludedFromBuild;
};

class cmLocalVisualStudio7GeneratorFCInfo
{
public:
  cmLocalVisualStudio7GeneratorFCInfo(
    cmLocalVisualStudio7Generator* lg, cmGeneratorTarget* target,
    cmGeneratorTarget::AllConfigSource const& acs,
    std::vector<std::string> const& configs);
  std::map<std::string, cmLVS7GFileConfig> FileConfigMap;
};

cmLocalVisualStudio7GeneratorFCInfo::cmLocalVisualStudio7GeneratorFCInfo(
  cmLocalVisualStudio7Generator* lg, cmGeneratorTarget* gt,
  cmGeneratorTarget::AllConfigSource const& acs,
  std::vector<std::string> const& configs)
{
  cmSourceFile const& sf = *acs.Source;
  std::string objectName;
  if (gt->HasExplicitObjectName(&sf)) {
    objectName = gt->GetObjectName(&sf);
  }

  // Compute per-source, per-config information.
  size_t ci = 0;
  for (std::string const& config : configs) {
    std::string configUpper = cmSystemTools::UpperCase(config);
    cmLVS7GFileConfig fc;

    std::string lang =
      lg->GlobalGenerator->GetLanguageFromExtension(sf.GetExtension().c_str());
    const std::string& sourceLang = lg->GetSourceFileLanguage(sf);
    bool needForceLang = false;
    // source file does not match its extension language
    if (lang != sourceLang) {
      needForceLang = true;
      lang = sourceLang;
    }

    cmGeneratorExpressionInterpreter genexInterpreter(lg, config, gt, lang);

    bool needfc = false;
    if (!objectName.empty()) {
      fc.ObjectName = objectName;
      needfc = true;
    }
    const std::string COMPILE_FLAGS("COMPILE_FLAGS");
    if (cmValue cflags = sf.GetProperty(COMPILE_FLAGS)) {
      fc.CompileFlags = genexInterpreter.Evaluate(*cflags, COMPILE_FLAGS);
      needfc = true;
    }
    const std::string COMPILE_OPTIONS("COMPILE_OPTIONS");
    if (cmValue coptions = sf.GetProperty(COMPILE_OPTIONS)) {
      lg->AppendCompileOptions(
        fc.CompileFlags,
        genexInterpreter.Evaluate(*coptions, COMPILE_OPTIONS));
      needfc = true;
    }
    // Add precompile headers compile options.
    const std::string pchSource = gt->GetPchSource(config, lang);
    if (!pchSource.empty() && !sf.GetProperty("SKIP_PRECOMPILE_HEADERS")) {
      std::string pchOptions;
      if (sf.GetFullPath() == pchSource) {
        pchOptions = gt->GetPchCreateCompileOptions(config, lang);
      } else {
        pchOptions = gt->GetPchUseCompileOptions(config, lang);
      }

      lg->AppendCompileOptions(
        fc.CompileFlags,
        genexInterpreter.Evaluate(pchOptions, COMPILE_OPTIONS));
      needfc = true;
    }

    if (lg->FortranProject) {
      switch (cmOutputConverter::GetFortranPreprocess(
        sf.GetSafeProperty("Fortran_PREPROCESS"))) {
        case cmOutputConverter::FortranPreprocess::Needed:
          fc.CompileFlags = cmStrCat("-fpp ", fc.CompileFlags);
          needfc = true;
          break;
        case cmOutputConverter::FortranPreprocess::NotNeeded:
          fc.CompileFlags = cmStrCat("-nofpp ", fc.CompileFlags);
          needfc = true;
          break;
        default:
          break;
      }

      switch (cmOutputConverter::GetFortranFormat(
        sf.GetSafeProperty("Fortran_FORMAT"))) {
        case cmOutputConverter::FortranFormatFixed:
          fc.CompileFlags = "-fixed " + fc.CompileFlags;
          needfc = true;
          break;
        case cmOutputConverter::FortranFormatFree:
          fc.CompileFlags = "-free " + fc.CompileFlags;
          needfc = true;
          break;
        default:
          break;
      }
    }
    const std::string COMPILE_DEFINITIONS("COMPILE_DEFINITIONS");
    if (cmValue cdefs = sf.GetProperty(COMPILE_DEFINITIONS)) {
      fc.CompileDefs = genexInterpreter.Evaluate(*cdefs, COMPILE_DEFINITIONS);
      needfc = true;
    }
    std::string defPropName = cmStrCat("COMPILE_DEFINITIONS_", configUpper);
    if (cmValue ccdefs = sf.GetProperty(defPropName)) {
      fc.CompileDefsConfig =
        genexInterpreter.Evaluate(*ccdefs, COMPILE_DEFINITIONS);
      needfc = true;
    }

    const std::string INCLUDE_DIRECTORIES("INCLUDE_DIRECTORIES");
    if (cmValue cincs = sf.GetProperty(INCLUDE_DIRECTORIES)) {
      fc.IncludeDirs = genexInterpreter.Evaluate(*cincs, INCLUDE_DIRECTORIES);
      needfc = true;
    }

    // Check for extra object-file dependencies.
    if (cmValue deps = sf.GetProperty("OBJECT_DEPENDS")) {
      cmList depends{ *deps };
      if (!depends.empty()) {
        for (std::string& d : depends) {
          d = lg->ConvertToXMLOutputPath(d);
        }
        fc.AdditionalDeps += depends.to_string();
        needfc = true;
      }
    }

    const std::string& linkLanguage = gt->GetLinkerLanguage(config);
    // If HEADER_FILE_ONLY is set, we must suppress this generation in
    // the project file
    fc.ExcludedFromBuild = sf.GetPropertyAsBool("HEADER_FILE_ONLY") ||
      !cm::contains(acs.Configs, ci) ||
      (gt->GetPropertyAsBool("UNITY_BUILD") &&
       sf.GetProperty("UNITY_SOURCE_FILE") &&
       !sf.GetPropertyAsBool("SKIP_UNITY_BUILD_INCLUSION"));
    if (fc.ExcludedFromBuild) {
      needfc = true;
    }

    // if the source file does not match the linker language
    // then force c or c++
    if (needForceLang || (linkLanguage != lang)) {
      if (lang == "CXX") {
        // force a C++ file type
        fc.CompileFlags += " /TP ";
        needfc = true;
      } else if (lang == "C") {
        // force to c
        fc.CompileFlags += " /TC ";
        needfc = true;
      }
    }

    if (needfc) {
      this->FileConfigMap[config] = fc;
    }
    ++ci;
  }
}

std::string cmLocalVisualStudio7Generator::ComputeLongestObjectDirectory(
  cmGeneratorTarget const* target) const
{
  std::vector<std::string> configs =
    target->Target->GetMakefile()->GetGeneratorConfigs(
      cmMakefile::ExcludeEmptyConfig);

  // Compute the maximum length configuration name.
  std::string config_max;
  for (auto& config : configs) {
    if (config.size() > config_max.size()) {
      config_max = config;
    }
  }

  // Compute the maximum length full path to the intermediate
  // files directory for any configuration.  This is used to construct
  // object file names that do not produce paths that are too long.
  std::string dir_max =
    cmStrCat(this->GetCurrentBinaryDirectory(), '/',
             this->GetTargetDirectory(target), '/', config_max, '/');
  return dir_max;
}

bool cmLocalVisualStudio7Generator::WriteGroup(
  const cmSourceGroup* sg, cmGeneratorTarget* target, std::ostream& fout,
  const std::string& libName, std::vector<std::string> const& configs,
  AllConfigSources const& sources)
{
  cmGlobalVisualStudio7Generator* gg =
    static_cast<cmGlobalVisualStudio7Generator*>(this->GlobalGenerator);
  const std::vector<const cmSourceFile*>& sourceFiles = sg->GetSourceFiles();
  std::vector<cmSourceGroup> const& children = sg->GetGroupChildren();

  // Write the children to temporary output.
  bool hasChildrenWithSources = false;
  std::ostringstream tmpOut;
  for (const auto& child : children) {
    if (this->WriteGroup(&child, target, tmpOut, libName, configs, sources)) {
      hasChildrenWithSources = true;
    }
  }

  // If the group is empty, don't write it at all.
  if (sourceFiles.empty() && !hasChildrenWithSources) {
    return false;
  }

  // If the group has a name, write the header.
  std::string const& name = sg->GetName();
  if (!name.empty()) {
    this->WriteVCProjBeginGroup(fout, name.c_str(), "");
  }

  auto& sourcesVisited = this->GetSourcesVisited(target);

  // Loop through each source in the source group.
  for (const cmSourceFile* sf : sourceFiles) {
    std::string source = sf->GetFullPath();

    if (source != libName || target->GetType() == cmStateEnums::UTILITY ||
        target->GetType() == cmStateEnums::GLOBAL_TARGET ||
        target->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
      // Look up the source kind and configs.
      auto map_it = sources.Index.find(sf);
      // The map entry must exist because we populated it earlier.
      assert(map_it != sources.Index.end());
      cmGeneratorTarget::AllConfigSource const& acs =
        sources.Sources[map_it->second];

      FCInfo fcinfo(this, target, acs, configs);

      fout << "\t\t\t<File\n";
      std::string d = this->ConvertToXMLOutputPathSingle(source);
      // Tell MS-Dev what the source is.  If the compiler knows how to
      // build it, then it will.
      fout << "\t\t\t\tRelativePath=\"" << d << "\">\n";
      if (cmCustomCommand const* command = sf->GetCustomCommand()) {
        if (sourcesVisited.insert(sf).second) {
          this->WriteCustomRule(fout, configs, source.c_str(), *command,
                                fcinfo);
        }
      } else if (!fcinfo.FileConfigMap.empty()) {
        const char* aCompilerTool = "VCCLCompilerTool";
        std::string ppLang = "CXX";
        if (this->FortranProject) {
          aCompilerTool = "VFFortranCompilerTool";
        }
        std::string const& lang = sf->GetLanguage();
        std::string ext = sf->GetExtension();
        ext = cmSystemTools::LowerCase(ext);
        if (ext == "idl") {
          aCompilerTool = "VCMIDLTool";
          if (this->FortranProject) {
            aCompilerTool = "VFMIDLTool";
          }
        }
        if (ext == "rc") {
          aCompilerTool = "VCResourceCompilerTool";
          ppLang = "RC";
          if (this->FortranProject) {
            aCompilerTool = "VFResourceCompilerTool";
          }
        }
        if (ext == "def") {
          aCompilerTool = "VCCustomBuildTool";
          if (this->FortranProject) {
            aCompilerTool = "VFCustomBuildTool";
          }
        }
        if (gg->IsMarmasmEnabled() && !this->FortranProject &&
            lang == "ASM_MARMASM") {
          aCompilerTool = "MARMASM";
        }
        if (gg->IsMasmEnabled() && !this->FortranProject &&
            lang == "ASM_MASM") {
          aCompilerTool = "MASM";
        }
        if (acs.Kind == cmGeneratorTarget::SourceKindExternalObject) {
          aCompilerTool = "VCCustomBuildTool";
        }
        for (auto const& fci : fcinfo.FileConfigMap) {
          cmLVS7GFileConfig const& fc = fci.second;
          fout << "\t\t\t\t<FileConfiguration\n"
               << "\t\t\t\t\tName=\"" << fci.first << "|"
               << gg->GetPlatformName() << "\"";
          if (fc.ExcludedFromBuild) {
            fout << " ExcludedFromBuild=\"true\"";
          }
          fout << ">\n";
          fout << "\t\t\t\t\t<Tool\n"
               << "\t\t\t\t\tName=\"" << aCompilerTool << "\"\n";
          if (!fc.CompileFlags.empty() || !fc.CompileDefs.empty() ||
              !fc.CompileDefsConfig.empty() || !fc.IncludeDirs.empty()) {
            Options::Tool tool = Options::Compiler;
            cmVS7FlagTable const* table =
              cmLocalVisualStudio7GeneratorFlagTable;
            if (this->FortranProject) {
              tool = Options::FortranCompiler;
              table = cmLocalVisualStudio7GeneratorFortranFlagTable;
            }
            Options fileOptions(this, tool, table, gg->ExtraFlagTable);
            fileOptions.Parse(fc.CompileFlags);
            fileOptions.AddDefines(fc.CompileDefs);
            fileOptions.AddDefines(fc.CompileDefsConfig);
            // validate source level include directories
            std::vector<std::string> includes;
            this->AppendIncludeDirectories(includes, fc.IncludeDirs, *sf);
            fileOptions.AddIncludes(includes);
            fileOptions.OutputFlagMap(fout, 5);
            fileOptions.OutputAdditionalIncludeDirectories(
              fout, 5,
              ppLang == "CXX" && this->FortranProject ? "Fortran" : ppLang);
            fileOptions.OutputPreprocessorDefinitions(fout, 5, ppLang);
          }
          if (!fc.AdditionalDeps.empty()) {
            fout << "\t\t\t\t\tAdditionalDependencies=\"" << fc.AdditionalDeps
                 << "\"\n";
          }
          if (!fc.ObjectName.empty()) {
            fout << "\t\t\t\t\tObjectFile=\"$(IntDir)/" << fc.ObjectName
                 << "\"\n";
          }
          fout << "\t\t\t\t\t/>\n"
               << "\t\t\t\t</FileConfiguration>\n";
        }
      }
      fout << "\t\t\t</File>\n";
    }
  }

  // If the group has children with source files, write the children.
  if (hasChildrenWithSources) {
    fout << tmpOut.str();
  }

  // If the group has a name, write the footer.
  if (!name.empty()) {
    this->WriteVCProjEndGroup(fout);
  }

  return true;
}

void cmLocalVisualStudio7Generator::WriteCustomRule(
  std::ostream& fout, std::vector<std::string> const& configs,
  const char* source, const cmCustomCommand& command, FCInfo& fcinfo)
{
  cmGlobalVisualStudio7Generator* gg =
    static_cast<cmGlobalVisualStudio7Generator*>(this->GlobalGenerator);

  // Write the rule for each configuration.
  const char* compileTool = "VCCLCompilerTool";
  if (this->FortranProject) {
    compileTool = "VFCLCompilerTool";
  }
  const char* customTool = "VCCustomBuildTool";
  if (this->FortranProject) {
    customTool = "VFCustomBuildTool";
  }
  for (std::string const& config : configs) {
    cmCustomCommandGenerator ccg(command, config, this);
    cmLVS7GFileConfig const& fc = fcinfo.FileConfigMap[config];
    fout << "\t\t\t\t<FileConfiguration\n";
    fout << "\t\t\t\t\tName=\"" << config << "|" << gg->GetPlatformName()
         << "\">\n";
    if (!fc.CompileFlags.empty()) {
      fout << "\t\t\t\t\t<Tool\n"
           << "\t\t\t\t\tName=\"" << compileTool << "\"\n"
           << "\t\t\t\t\tAdditionalOptions=\""
           << this->EscapeForXML(fc.CompileFlags) << "\"/>\n";
    }

    std::string comment = this->ConstructComment(ccg);
    std::string script = this->ConstructScript(ccg);
    if (this->FortranProject) {
      cmSystemTools::ReplaceString(script, "$(Configuration)", config);
    }
    script += this->FinishConstructScript(VsProjectType::vcxproj);
    /* clang-format off */
    fout << "\t\t\t\t\t<Tool\n"
         << "\t\t\t\t\tName=\"" << customTool << "\"\n"
         << "\t\t\t\t\tDescription=\""
         << this->EscapeForXML(comment) << "\"\n"
         << "\t\t\t\t\tCommandLine=\""
         << this->EscapeForXML(script) << "\"\n"
         << "\t\t\t\t\tAdditionalDependencies=\"";
    /* clang-format on */
    if (ccg.GetDepends().empty()) {
      // There are no real dependencies.  Produce an artificial one to
      // make sure the rule runs reliably.
      if (!cmSystemTools::FileExists(source)) {
        cmsys::ofstream depout(source);
        depout << "Artificial dependency for a custom command.\n";
      }
      fout << this->ConvertToXMLOutputPath(source);
    } else {
      // Write out the dependencies for the rule.
      for (std::string const& d : ccg.GetDepends()) {
        // Get the real name of the dependency in case it is a CMake target.
        std::string dep;
        if (this->GetRealDependency(d, config, dep)) {
          fout << this->ConvertToXMLOutputPath(dep) << ";";
        }
      }
    }
    fout << "\"\n";
    fout << "\t\t\t\t\tOutputs=\"";
    if (ccg.GetOutputs().empty()) {
      fout << source << "_force";
    } else {
      // Write a rule for the output generated by this command.
      const char* sep = "";
      for (std::string const& output : ccg.GetOutputs()) {
        fout << sep << this->ConvertToXMLOutputPathSingle(output);
        sep = ";";
      }
    }
    fout << "\"/>\n";
    fout << "\t\t\t\t</FileConfiguration>\n";
  }
}

void cmLocalVisualStudio7Generator::WriteVCProjBeginGroup(std::ostream& fout,
                                                          const char* group,
                                                          const char*)
{
  /* clang-format off */
  fout << "\t\t<Filter\n"
       << "\t\t\tName=\"" << group << "\"\n"
       << "\t\t\tFilter=\"\">\n";
  /* clang-format on */
}

void cmLocalVisualStudio7Generator::WriteVCProjEndGroup(std::ostream& fout)
{
  fout << "\t\t</Filter>\n";
}

// look for custom rules on a target and collect them together
void cmLocalVisualStudio7Generator::OutputTargetRules(
  std::ostream& fout, const std::string& configName, cmGeneratorTarget* target,
  const std::string& /*libName*/)
{
  if (target->GetType() > cmStateEnums::GLOBAL_TARGET) {
    return;
  }
  EventWriter event(this, configName, fout);

  // Add pre-build event.
  const char* tool =
    this->FortranProject ? "VFPreBuildEventTool" : "VCPreBuildEventTool";
  event.Start(tool);
  event.Write(target->GetPreBuildCommands());
  event.Finish();

  // Add pre-link event.
  tool = this->FortranProject ? "VFPreLinkEventTool" : "VCPreLinkEventTool";
  event.Start(tool);
  bool addedPrelink = false;
  cmGeneratorTarget::ModuleDefinitionInfo const* mdi =
    target->GetModuleDefinitionInfo(configName);
  if (mdi && mdi->DefFileGenerated) {
    addedPrelink = true;
    std::vector<cmCustomCommand> commands = target->GetPreLinkCommands();
    cmGlobalVisualStudioGenerator* gg =
      static_cast<cmGlobalVisualStudioGenerator*>(this->GlobalGenerator);
    gg->AddSymbolExportCommand(target, commands, configName);
    event.Write(commands);
  }
  if (!addedPrelink) {
    event.Write(target->GetPreLinkCommands());
  }
  std::unique_ptr<cmCustomCommand> pcc(
    this->MaybeCreateImplibDir(target, configName, this->FortranProject));
  if (pcc) {
    event.Write(*pcc);
  }
  event.Finish();

  // Add post-build event.
  tool =
    this->FortranProject ? "VFPostBuildEventTool" : "VCPostBuildEventTool";
  event.Start(tool);
  event.Write(target->GetPostBuildCommands());
  event.Finish();
}

void cmLocalVisualStudio7Generator::WriteProjectSCC(std::ostream& fout,
                                                    cmGeneratorTarget* target)
{
  // if we have all the required Source code control tags
  // then add that to the project
  cmValue vsProjectname = target->GetProperty("VS_SCC_PROJECTNAME");
  cmValue vsLocalpath = target->GetProperty("VS_SCC_LOCALPATH");
  cmValue vsProvider = target->GetProperty("VS_SCC_PROVIDER");

  if (vsProvider && vsLocalpath && vsProjectname) {
    /* clang-format off */
    fout << "\tSccProjectName=\"" << *vsProjectname << "\"\n"
         << "\tSccLocalPath=\"" << *vsLocalpath << "\"\n"
         << "\tSccProvider=\"" << *vsProvider << "\"\n";
    /* clang-format on */

    cmValue vsAuxPath = target->GetProperty("VS_SCC_AUXPATH");
    if (vsAuxPath) {
      fout << "\tSccAuxPath=\"" << *vsAuxPath << "\"\n";
    }
  }
}

void cmLocalVisualStudio7Generator::WriteProjectStartFortran(
  std::ostream& fout, const std::string& libName, cmGeneratorTarget* target)
{

  cmGlobalVisualStudio7Generator* gg =
    static_cast<cmGlobalVisualStudio7Generator*>(this->GlobalGenerator);
  /* clang-format off */
  fout << R"(<?xml version="1.0" encoding = ")"
       << gg->Encoding() << "\"?>\n"
       << "<VisualStudioProject\n"
       << "\tProjectCreator=\"Intel Fortran\"\n"
       << "\tVersion=\"" << gg->GetIntelProjectVersion() << "\"\n";
  /* clang-format on */
  cmValue p = target->GetProperty("VS_KEYWORD");
  const char* keyword = p ? p->c_str() : "Console Application";
  const char* projectType = nullptr;
  switch (target->GetType()) {
    case cmStateEnums::OBJECT_LIBRARY:
    case cmStateEnums::STATIC_LIBRARY:
      projectType = "typeStaticLibrary";
      if (keyword) {
        keyword = "Static Library";
      }
      break;
    case cmStateEnums::SHARED_LIBRARY:
    case cmStateEnums::MODULE_LIBRARY:
      projectType = "typeDynamicLibrary";
      if (!keyword) {
        keyword = "Dll";
      }
      break;
    case cmStateEnums::EXECUTABLE:
      if (!keyword) {
        keyword = "Console Application";
      }
      projectType = nullptr;
      break;
    case cmStateEnums::UTILITY:
    case cmStateEnums::GLOBAL_TARGET:
    case cmStateEnums::INTERFACE_LIBRARY:
    case cmStateEnums::UNKNOWN_LIBRARY:
      break;
  }
  if (projectType) {
    fout << "\tProjectType=\"" << projectType << "\"\n";
  }
  this->WriteProjectSCC(fout, target);
  /* clang-format off */
  fout<< "\tKeyword=\"" << keyword << "\">\n"
       << "\tProjectGUID=\"{" << gg->GetGUID(libName) << "}\">\n"
       << "\t<Platforms>\n"
       << "\t\t<Platform\n\t\t\tName=\"" << gg->GetPlatformName() << "\"/>\n"
       << "\t</Platforms>\n";
  /* clang-format on */
}

void cmLocalVisualStudio7Generator::WriteProjectStart(
  std::ostream& fout, const std::string& libName, cmGeneratorTarget* target,
  std::vector<cmSourceGroup>&)
{
  if (this->FortranProject) {
    this->WriteProjectStartFortran(fout, libName, target);
    return;
  }

  cmGlobalVisualStudio7Generator* gg =
    static_cast<cmGlobalVisualStudio7Generator*>(this->GlobalGenerator);

  /* clang-format off */
  fout << R"(<?xml version="1.0" encoding = ")"
       << gg->Encoding() << "\"?>\n"
       << "<VisualStudioProject\n"
       << "\tProjectType=\"Visual C++\"\n";
  /* clang-format on */
  fout << "\tVersion=\"" << (static_cast<uint16_t>(gg->GetVersion()) / 10)
       << ".00\"\n";
  cmValue p = target->GetProperty("PROJECT_LABEL");
  const std::string projLabel = p ? *p : libName;
  p = target->GetProperty("VS_KEYWORD");
  const std::string keyword = p ? *p : "Win32Proj";
  fout << "\tName=\"" << projLabel << "\"\n";
  fout << "\tProjectGUID=\"{" << gg->GetGUID(libName) << "}\"\n";
  this->WriteProjectSCC(fout, target);
  if (cmValue targetFrameworkVersion =
        target->GetProperty("VS_DOTNET_TARGET_FRAMEWORK_VERSION")) {
    fout << "\tTargetFrameworkVersion=\"" << *targetFrameworkVersion << "\"\n";
  }
  /* clang-format off */
  fout << "\tKeyword=\"" << keyword << "\">\n"
       << "\t<Platforms>\n"
       << "\t\t<Platform\n\t\t\tName=\"" << gg->GetPlatformName() << "\"/>\n"
       << "\t</Platforms>\n";
  /* clang-format on */
  if (gg->IsMarmasmEnabled()) {
    /* clang-format off */
    fout <<
      "\t<ToolFiles>\n"
      "\t\t<DefaultToolFile\n"
      "\t\t\tFileName=\"marmasm.rules\"\n"
      "\t\t/>\n"
      "\t</ToolFiles>\n"
      ;
    /* clang-format on */
  }
  if (gg->IsMasmEnabled()) {
    /* clang-format off */
    fout <<
      "\t<ToolFiles>\n"
      "\t\t<DefaultToolFile\n"
      "\t\t\tFileName=\"masm.rules\"\n"
      "\t\t/>\n"
      "\t</ToolFiles>\n"
      ;
    /* clang-format on */
  }
}

void cmLocalVisualStudio7Generator::WriteVCProjFooter(
  std::ostream& fout, cmGeneratorTarget* target)
{
  fout << "\t<Globals>\n";

  for (std::string const& key : target->GetPropertyKeys()) {
    if (cmHasLiteralPrefix(key, "VS_GLOBAL_")) {
      std::string name = key.substr(10);
      if (!name.empty()) {
        /* clang-format off */
        fout << "\t\t<Global\n"
             << "\t\t\tName=\"" << name << "\"\n"
             << "\t\t\tValue=\"" << target->GetProperty(key) << "\"\n"
             << "\t\t/>\n";
        /* clang-format on */
      }
    }
  }

  fout << "\t</Globals>\n"
       << "</VisualStudioProject>\n";
}

std::string cmLocalVisualStudio7Generator::EscapeForXML(const std::string& s)
{
  return cmLocalVisualStudio7GeneratorEscapeForXML(s);
}

std::string cmLocalVisualStudio7Generator::ConvertToXMLOutputPath(
  const std::string& path)
{
  std::string ret =
    this->ConvertToOutputFormat(path, cmOutputConverter::SHELL);
  cmSystemTools::ReplaceString(ret, "&", "&amp;");
  cmSystemTools::ReplaceString(ret, "\"", "&quot;");
  cmSystemTools::ReplaceString(ret, "<", "&lt;");
  cmSystemTools::ReplaceString(ret, ">", "&gt;");
  return ret;
}

std::string cmLocalVisualStudio7Generator::ConvertToXMLOutputPathSingle(
  const std::string& path)
{
  std::string ret =
    this->ConvertToOutputFormat(path, cmOutputConverter::SHELL);
  cmSystemTools::ReplaceString(ret, "\"", "");
  cmSystemTools::ReplaceString(ret, "&", "&amp;");
  cmSystemTools::ReplaceString(ret, "<", "&lt;");
  cmSystemTools::ReplaceString(ret, ">", "&gt;");
  return ret;
}

void cmVS7GeneratorOptions::OutputFlag(std::ostream& fout, int indent,
                                       const std::string& flag,
                                       const std::string& content)
{
  fout.fill('\t');
  fout.width(indent);
  // write an empty string to get the fill level indent to print
  fout << "";
  fout << flag << "=\"";
  fout << cmLocalVisualStudio7GeneratorEscapeForXML(content);
  fout << "\"\n";
}

// This class is used to parse an existing vs 7 project
// and extract the GUID
class cmVS7XMLParser : public cmXMLParser
{
public:
  void EndElement(const std::string& /* name */) override {}
  void StartElement(const std::string& name, const char** atts) override
  {
    // once the GUID is found do nothing
    if (!this->GUID.empty()) {
      return;
    }
    int i = 0;
    if ("VisualStudioProject" == name) {
      while (atts[i]) {
        if (strcmp(atts[i], "ProjectGUID") == 0) {
          if (atts[i + 1]) {
            this->GUID = atts[i + 1];
            if (this->GUID[0] == '{') {
              // remove surrounding curly brackets
              this->GUID = this->GUID.substr(1, this->GUID.size() - 2);
            }
          } else {
            this->GUID.clear();
          }
          return;
        }
        ++i;
      }
    }
  }
  int InitializeParser() override
  {
    int ret = cmXMLParser::InitializeParser();
    if (ret == 0) {
      return ret;
    }
    // visual studio projects have a strange encoding, but it is
    // really utf-8
    XML_SetEncoding(static_cast<XML_Parser>(this->Parser), "utf-8");
    return 1;
  }
  std::string GUID;
};

void cmLocalVisualStudio7Generator::ReadAndStoreExternalGUID(
  const std::string& name, const char* path)
{
  cmVS7XMLParser parser;
  parser.ParseFile(path);
  // if we can not find a GUID then we will generate one later
  if (parser.GUID.empty()) {
    return;
  }
  std::string guidStoreName = cmStrCat(name, "_GUID_CMAKE");
  // save the GUID in the cache
  this->GlobalGenerator->GetCMakeInstance()->AddCacheEntry(
    guidStoreName, parser.GUID, "Stored GUID", cmStateEnums::INTERNAL);
}

std::string cmLocalVisualStudio7Generator::GetTargetDirectory(
  cmGeneratorTarget const* target) const
{
  std::string dir = cmStrCat(target->GetName(), ".dir");
  return dir;
}

static bool cmLVS7G_IsFAT(const char* dir)
{
  if (dir[0] && dir[1] == ':') {
    char volRoot[4] = "_:/";
    volRoot[0] = dir[0];
    char fsName[16];
    if (GetVolumeInformationA(volRoot, 0, 0, 0, 0, 0, fsName, 16) &&
        strstr(fsName, "FAT") != nullptr) {
      return true;
    }
  }
  return false;
}
