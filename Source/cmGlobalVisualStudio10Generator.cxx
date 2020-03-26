/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalVisualStudio10Generator.h"

#include <algorithm>

#include <cm/memory>

#include "cmsys/FStream.hxx"
#include "cmsys/Glob.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cm_jsoncpp_reader.h"

#include "cmAlgorithms.h"
#include "cmDocumentationEntry.h"
#include "cmGeneratorTarget.h"
#include "cmLocalVisualStudio10Generator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmSourceFile.h"
#include "cmVersion.h"
#include "cmVisualStudioSlnData.h"
#include "cmVisualStudioSlnParser.h"
#include "cmXMLWriter.h"
#include "cmake.h"

static const char vs10generatorName[] = "Visual Studio 10 2010";
static std::map<std::string, std::vector<cmIDEFlagTable>> loadedFlagJsonFiles;

static void ConvertToWindowsSlashes(std::string& s)
{
  // first convert all of the slashes
  for (auto& ch : s) {
    if (ch == '/') {
      ch = '\\';
    }
  }
}

// Map generator name without year to name with year.
static const char* cmVS10GenName(const std::string& name, std::string& genName)
{
  if (strncmp(name.c_str(), vs10generatorName,
              sizeof(vs10generatorName) - 6) != 0) {
    return 0;
  }
  const char* p = name.c_str() + sizeof(vs10generatorName) - 6;
  if (cmHasLiteralPrefix(p, " 2010")) {
    p += 5;
  }
  genName = std::string(vs10generatorName) + p;
  return p;
}

class cmGlobalVisualStudio10Generator::Factory
  : public cmGlobalGeneratorFactory
{
public:
  std::unique_ptr<cmGlobalGenerator> CreateGlobalGenerator(
    const std::string& name, cmake* cm) const override
  {
    std::string genName;
    const char* p = cmVS10GenName(name, genName);
    if (!p) {
      return std::unique_ptr<cmGlobalGenerator>();
    }
    if (!*p) {
      return std::unique_ptr<cmGlobalGenerator>(
        new cmGlobalVisualStudio10Generator(cm, genName, ""));
    }
    if (*p++ != ' ') {
      return std::unique_ptr<cmGlobalGenerator>();
    }
    if (strcmp(p, "Win64") == 0) {
      return std::unique_ptr<cmGlobalGenerator>(
        new cmGlobalVisualStudio10Generator(cm, genName, "x64"));
    }
    if (strcmp(p, "IA64") == 0) {
      return std::unique_ptr<cmGlobalGenerator>(
        new cmGlobalVisualStudio10Generator(cm, genName, "Itanium"));
    }
    return std::unique_ptr<cmGlobalGenerator>();
  }

  void GetDocumentation(cmDocumentationEntry& entry) const override
  {
    entry.Name = std::string(vs10generatorName) + " [arch]";
    entry.Brief = "Generates Visual Studio 2010 project files.  "
                  "Optional [arch] can be \"Win64\" or \"IA64\".";
  }

  std::vector<std::string> GetGeneratorNames() const override
  {
    std::vector<std::string> names;
    names.push_back(vs10generatorName);
    return names;
  }

  std::vector<std::string> GetGeneratorNamesWithPlatform() const override
  {
    std::vector<std::string> names;
    names.push_back(vs10generatorName + std::string(" IA64"));
    names.push_back(vs10generatorName + std::string(" Win64"));
    return names;
  }

  bool SupportsToolset() const override { return true; }
  bool SupportsPlatform() const override { return true; }

  std::vector<std::string> GetKnownPlatforms() const override
  {
    std::vector<std::string> platforms;
    platforms.emplace_back("x64");
    platforms.emplace_back("Win32");
    platforms.emplace_back("Itanium");
    return platforms;
  }

  std::string GetDefaultPlatformName() const override { return "Win32"; }
};

std::unique_ptr<cmGlobalGeneratorFactory>
cmGlobalVisualStudio10Generator::NewFactory()
{
  return std::unique_ptr<cmGlobalGeneratorFactory>(new Factory);
}

cmGlobalVisualStudio10Generator::cmGlobalVisualStudio10Generator(
  cmake* cm, const std::string& name,
  std::string const& platformInGeneratorName)
  : cmGlobalVisualStudio8Generator(cm, name, platformInGeneratorName)
{
  std::string vc10Express;
  this->ExpressEdition = cmSystemTools::ReadRegistryValue(
    "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VCExpress\\10.0\\Setup\\VC;"
    "ProductDir",
    vc10Express, cmSystemTools::KeyWOW64_32);
  this->CudaEnabled = false;
  this->SystemIsWindowsCE = false;
  this->SystemIsWindowsPhone = false;
  this->SystemIsWindowsStore = false;
  this->MSBuildCommandInitialized = false;
  {
    std::string envPlatformToolset;
    if (cmSystemTools::GetEnv("PlatformToolset", envPlatformToolset) &&
        envPlatformToolset == "Windows7.1SDK") {
      // We are running from a Windows7.1SDK command prompt.
      this->DefaultPlatformToolset = "Windows7.1SDK";
    } else {
      this->DefaultPlatformToolset = "v100";
    }
  }
  this->DefaultCLFlagTableName = "v10";
  this->DefaultCSharpFlagTableName = "v10";
  this->DefaultLibFlagTableName = "v10";
  this->DefaultLinkFlagTableName = "v10";
  this->DefaultCudaFlagTableName = "v10";
  this->DefaultCudaHostFlagTableName = "v10";
  this->DefaultMasmFlagTableName = "v10";
  this->DefaultNasmFlagTableName = "v10";
  this->DefaultRCFlagTableName = "v10";

  this->Version = VS10;
  this->PlatformToolsetNeedsDebugEnum = false;
}

bool cmGlobalVisualStudio10Generator::MatchesGeneratorName(
  const std::string& name) const
{
  std::string genName;
  if (cmVS10GenName(name, genName)) {
    return genName == this->GetName();
  }
  return false;
}

bool cmGlobalVisualStudio10Generator::SetSystemName(std::string const& s,
                                                    cmMakefile* mf)
{
  this->SystemName = s;
  this->SystemVersion = mf->GetSafeDefinition("CMAKE_SYSTEM_VERSION");
  if (!this->InitializeSystem(mf)) {
    return false;
  }
  return this->cmGlobalVisualStudio8Generator::SetSystemName(s, mf);
}

bool cmGlobalVisualStudio10Generator::SetGeneratorPlatform(
  std::string const& p, cmMakefile* mf)
{
  if (!this->cmGlobalVisualStudio8Generator::SetGeneratorPlatform(p, mf)) {
    return false;
  }
  if (this->GetPlatformName() == "Itanium" ||
      this->GetPlatformName() == "x64") {
    if (this->IsExpressEdition() && !this->Find64BitTools(mf)) {
      return false;
    }
  }
  return true;
}

static void cmCudaToolVersion(std::string& s)
{
  // "CUDA x.y.props" => "x.y"
  s = s.substr(5);
  s = s.substr(0, s.size() - 6);
}

bool cmGlobalVisualStudio10Generator::SetGeneratorToolset(
  std::string const& ts, bool build, cmMakefile* mf)
{
  if (this->SystemIsWindowsCE && ts.empty() &&
      this->DefaultPlatformToolset.empty()) {
    std::ostringstream e;
    e << this->GetName() << " Windows CE version '" << this->SystemVersion
      << "' requires CMAKE_GENERATOR_TOOLSET to be set.";
    mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return false;
  }

  if (!this->ParseGeneratorToolset(ts, mf)) {
    return false;
  }

  if (build) {
    return true;
  }

  if (this->CustomVCTargetsPath.empty() && !this->FindVCTargetsPath(mf)) {
    return false;
  }

  if (cmHasLiteralPrefix(this->GetPlatformToolsetString(), "v140")) {
    // The GenerateDebugInformation link setting for the v140 toolset
    // in VS 2015 was originally an enum with "No" and "Debug" values,
    // differing from the "false" and "true" values used in older toolsets.
    // A VS 2015 update changed it back.  Parse the "link.xml" file to
    // discover which one we need.
    std::string const link_xml = this->VCTargetsPath + "/1033/link.xml";
    cmsys::ifstream fin(link_xml.c_str());
    std::string line;
    while (fin && cmSystemTools::GetLineFromStream(fin, line)) {
      if (line.find(" Switch=\"DEBUG\" ") != std::string::npos) {
        this->PlatformToolsetNeedsDebugEnum =
          line.find(" Name=\"Debug\" ") != std::string::npos;
        break;
      }
    }
  }

  this->SupportsUnityBuilds =
    this->Version >= cmGlobalVisualStudioGenerator::VS16 ||
    (this->Version == cmGlobalVisualStudioGenerator::VS15 &&
     cmSystemTools::PathExists(this->VCTargetsPath +
                               "/Microsoft.Cpp.Unity.targets"));

  if (this->GeneratorToolsetCuda.empty()) {
    // Find the highest available version of the CUDA tools.
    std::vector<std::string> cudaTools;
    std::string bcDir;
    if (this->GeneratorToolsetCudaCustomDir.empty()) {
      bcDir = this->VCTargetsPath + "/BuildCustomizations";
    } else {
      bcDir = this->GetPlatformToolsetCudaCustomDirString() +
        "CUDAVisualStudioIntegration\\extras\\"
        "visual_studio_integration\\MSBuildExtensions";
      cmSystemTools::ConvertToUnixSlashes(bcDir);
    }
    cmsys::Glob gl;
    gl.SetRelative(bcDir.c_str());
    if (gl.FindFiles(bcDir + "/CUDA *.props")) {
      cudaTools = gl.GetFiles();
    }
    if (!cudaTools.empty()) {
      std::for_each(cudaTools.begin(), cudaTools.end(), cmCudaToolVersion);
      std::sort(cudaTools.begin(), cudaTools.end(),
                cmSystemTools::VersionCompareGreater);
      this->GeneratorToolsetCuda = cudaTools.at(0);
    } else if (!this->GeneratorToolsetCudaCustomDir.empty()) {
      // Generate an error if Visual Studio integration files are not found
      // inside of custom cuda toolset.
      std::ostringstream e;
      /* clang-format off */
      e <<
        "Generator\n"
        "  " << this->GetName() << "\n"
        "given toolset\n"
        "  cuda=" << this->GeneratorToolsetCudaCustomDir << "\n"
        "cannot detect Visual Studio integration files in path\n"
        "  " << bcDir;

      /* clang-format on */
      mf->IssueMessage(MessageType::FATAL_ERROR, e.str());

      // Clear the configured tool-set
      this->GeneratorToolsetCuda.clear();
    }
  }

  if (!this->GeneratorToolsetVersion.empty() &&
      this->GeneratorToolsetVersion != "Test Toolset Version") {
    // If a specific minor version of the toolset was requested, verify that it
    // is compatible to the major version and that is exists on disk.
    // If not clear the value.
    std::string version = this->GeneratorToolsetVersion;
    cmsys::RegularExpression regex("[0-9][0-9]\\.[0-9][0-9]");
    if (regex.find(version)) {
      version = "v" + version.erase(2, 1);
    } else {
      // Version not recognized. Clear it.
      version.clear();
    }

    if (version.find(this->GetPlatformToolsetString()) != 0) {
      std::ostringstream e;
      /* clang-format off */
      e <<
        "Generator\n"
        "  " << this->GetName() << "\n"
        "given toolset and version specification\n"
        "  " << this->GetPlatformToolsetString() << ",version=" <<
        this->GeneratorToolsetVersion << "\n"
        "contains an invalid version specification."
      ;
      /* clang-format on */
      mf->IssueMessage(MessageType::FATAL_ERROR, e.str());

      // Clear the configured tool-set
      this->GeneratorToolsetVersion.clear();
    }

    bool const isDefaultToolset =
      this->IsDefaultToolset(this->GeneratorToolsetVersion);
    if (isDefaultToolset) {
      // If the given version is the default toolset, remove the setting
      this->GeneratorToolsetVersion.clear();
    } else {
      std::string const toolsetPath = this->GetAuxiliaryToolset();
      if (!toolsetPath.empty() && !cmSystemTools::FileExists(toolsetPath)) {

        std::ostringstream e;
        /* clang-format off */
        e <<
          "Generator\n"
          "  " << this->GetName() << "\n"
          "given toolset and version specification\n"
          "  " << this->GetPlatformToolsetString() << ",version=" <<
          this->GeneratorToolsetVersion << "\n"
          "does not seem to be installed at\n" <<
          "  " << toolsetPath;
        ;
        /* clang-format on */
        mf->IssueMessage(MessageType::FATAL_ERROR, e.str());

        // Clear the configured tool-set
        this->GeneratorToolsetVersion.clear();
      }
    }
  }

  if (const char* toolset = this->GetPlatformToolset()) {
    mf->AddDefinition("CMAKE_VS_PLATFORM_TOOLSET", toolset);
  }
  if (const char* version = this->GetPlatformToolsetVersion()) {
    mf->AddDefinition("CMAKE_VS_PLATFORM_TOOLSET_VERSION", version);
  }
  if (const char* hostArch = this->GetPlatformToolsetHostArchitecture()) {
    mf->AddDefinition("CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE", hostArch);
  }
  if (const char* cuda = this->GetPlatformToolsetCuda()) {
    mf->AddDefinition("CMAKE_VS_PLATFORM_TOOLSET_CUDA", cuda);
  }
  if (const char* cudaDir = this->GetPlatformToolsetCudaCustomDir()) {
    mf->AddDefinition("CMAKE_VS_PLATFORM_TOOLSET_CUDA_CUSTOM_DIR", cudaDir);
  }
  if (const char* vcTargetsDir = this->GetCustomVCTargetsPath()) {
    mf->AddDefinition("CMAKE_VS_PLATFORM_TOOLSET_VCTARGETS_CUSTOM_DIR",
                      vcTargetsDir);
  }

  return true;
}

bool cmGlobalVisualStudio10Generator::ParseGeneratorToolset(
  std::string const& ts, cmMakefile* mf)
{
  std::vector<std::string> const fields = cmTokenize(ts, ",");
  std::vector<std::string>::const_iterator fi = fields.begin();
  if (fi == fields.end()) {
    return true;
  }

  // The first field may be the VS platform toolset.
  if (fi->find('=') == fi->npos) {
    this->GeneratorToolset = *fi;
    ++fi;
  }

  std::set<std::string> handled;

  // The rest of the fields must be key=value pairs.
  for (; fi != fields.end(); ++fi) {
    std::string::size_type pos = fi->find('=');
    if (pos == fi->npos) {
      std::ostringstream e;
      /* clang-format off */
      e <<
        "Generator\n"
        "  " << this->GetName() << "\n"
        "given toolset specification\n"
        "  " << ts << "\n"
        "that contains a field after the first ',' with no '='."
        ;
      /* clang-format on */
      mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return false;
    }
    std::string const key = fi->substr(0, pos);
    std::string const value = fi->substr(pos + 1);
    if (!handled.insert(key).second) {
      std::ostringstream e;
      /* clang-format off */
      e <<
        "Generator\n"
        "  " << this->GetName() << "\n"
        "given toolset specification\n"
        "  " << ts << "\n"
        "that contains duplicate field key '" << key << "'."
        ;
      /* clang-format on */
      mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return false;
    }
    if (!this->ProcessGeneratorToolsetField(key, value)) {
      std::ostringstream e;
      /* clang-format off */
      e <<
        "Generator\n"
        "  " << this->GetName() << "\n"
        "given toolset specification\n"
        "  " << ts << "\n"
        "that contains invalid field '" << *fi << "'."
        ;
      /* clang-format on */
      mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return false;
    }
  }

  return true;
}

bool cmGlobalVisualStudio10Generator::ProcessGeneratorToolsetField(
  std::string const& key, std::string const& value)
{
  if (key == "cuda") {
    /* test if cuda toolset is path to custom dir or cuda version */
    auto pos = value.find_first_not_of("0123456789.");
    if (pos != std::string::npos) {
      this->GeneratorToolsetCudaCustomDir = value;
      /* ensure trailing backslash for easy path joining */
      if (this->GeneratorToolsetCudaCustomDir.back() != '\\') {
        this->GeneratorToolsetCudaCustomDir.push_back('\\');
      }
    } else {
      this->GeneratorToolsetCuda = value;
    }
    return true;
  }
  if (key == "version") {
    this->GeneratorToolsetVersion = value;
    return true;
  }
  if (key == "VCTargetsPath") {
    this->CustomVCTargetsPath = value;
    ConvertToWindowsSlashes(this->CustomVCTargetsPath);
    return true;
  }
  return false;
}

bool cmGlobalVisualStudio10Generator::InitializeSystem(cmMakefile* mf)
{
  if (this->SystemName == "Windows") {
    if (!this->InitializeWindows(mf)) {
      return false;
    }
  } else if (this->SystemName == "WindowsCE") {
    this->SystemIsWindowsCE = true;
    if (!this->InitializeWindowsCE(mf)) {
      return false;
    }
  } else if (this->SystemName == "WindowsPhone") {
    this->SystemIsWindowsPhone = true;
    if (!this->InitializeWindowsPhone(mf)) {
      return false;
    }
  } else if (this->SystemName == "WindowsStore") {
    this->SystemIsWindowsStore = true;
    if (!this->InitializeWindowsStore(mf)) {
      return false;
    }
  } else if (this->SystemName == "Android") {
    if (this->PlatformInGeneratorName) {
      std::ostringstream e;
      e << "CMAKE_SYSTEM_NAME is 'Android' but CMAKE_GENERATOR "
        << "specifies a platform too: '" << this->GetName() << "'";
      mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return false;
    }
    std::string v = this->GetInstalledNsightTegraVersion();
    if (v.empty()) {
      mf->IssueMessage(MessageType::FATAL_ERROR,
                       "CMAKE_SYSTEM_NAME is 'Android' but "
                       "'NVIDIA Nsight Tegra Visual Studio Edition' "
                       "is not installed.");
      return false;
    }
    this->DefaultPlatformName = "Tegra-Android";
    this->DefaultPlatformToolset = "Default";
    this->NsightTegraVersion = v;
    mf->AddDefinition("CMAKE_VS_NsightTegra_VERSION", v);
  }

  return true;
}

bool cmGlobalVisualStudio10Generator::InitializeWindows(cmMakefile*)
{
  return true;
}

bool cmGlobalVisualStudio10Generator::InitializeWindowsCE(cmMakefile* mf)
{
  if (this->PlatformInGeneratorName) {
    std::ostringstream e;
    e << "CMAKE_SYSTEM_NAME is 'WindowsCE' but CMAKE_GENERATOR "
      << "specifies a platform too: '" << this->GetName() << "'";
    mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return false;
  }

  this->DefaultPlatformToolset = this->SelectWindowsCEToolset();

  return true;
}

bool cmGlobalVisualStudio10Generator::InitializeWindowsPhone(cmMakefile* mf)
{
  std::ostringstream e;
  e << this->GetName() << " does not support Windows Phone.";
  mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
  return false;
}

bool cmGlobalVisualStudio10Generator::InitializeWindowsStore(cmMakefile* mf)
{
  std::ostringstream e;
  e << this->GetName() << " does not support Windows Store.";
  mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
  return false;
}

bool cmGlobalVisualStudio10Generator::SelectWindowsPhoneToolset(
  std::string& toolset) const
{
  toolset.clear();
  return false;
}

bool cmGlobalVisualStudio10Generator::SelectWindowsStoreToolset(
  std::string& toolset) const
{
  toolset.clear();
  return false;
}

std::string cmGlobalVisualStudio10Generator::SelectWindowsCEToolset() const
{
  if (this->SystemVersion == "8.0") {
    return "CE800";
  }
  return "";
}

//! Create a local generator appropriate to this Global Generator
std::unique_ptr<cmLocalGenerator>
cmGlobalVisualStudio10Generator::CreateLocalGenerator(cmMakefile* mf)
{
  return std::unique_ptr<cmLocalGenerator>(
    cm::make_unique<cmLocalVisualStudio10Generator>(this, mf));
}

void cmGlobalVisualStudio10Generator::Generate()
{
  this->LongestSource = LongestSourcePath();
  this->cmGlobalVisualStudio8Generator::Generate();
  if (this->LongestSource.Length > 0) {
    cmLocalGenerator* lg = this->LongestSource.Target->GetLocalGenerator();
    std::ostringstream e;
    /* clang-format off */
    e <<
      "The binary and/or source directory paths may be too long to generate "
      "Visual Studio 10 files for this project.  "
      "Consider choosing shorter directory names to build this project with "
      "Visual Studio 10.  "
      "A more detailed explanation follows."
      "\n"
      "There is a bug in the VS 10 IDE that renders property dialog fields "
      "blank for files referenced by full path in the project file.  "
      "However, CMake must reference at least one file by full path:\n"
      "  " << this->LongestSource.SourceFile->GetFullPath() << "\n"
      "This is because some Visual Studio tools would append the relative "
      "path to the end of the referencing directory path, as in:\n"
      "  " << lg->GetCurrentBinaryDirectory() << "/"
      << this->LongestSource.SourceRel << "\n"
      "and then incorrectly complain that the file does not exist because "
      "the path length is too long for some internal buffer or API.  "
      "To avoid this problem CMake must use a full path for this file "
      "which then triggers the VS 10 property dialog bug.";
    /* clang-format on */
    lg->IssueMessage(MessageType::WARNING, e.str());
  }
}

void cmGlobalVisualStudio10Generator::EnableLanguage(
  std::vector<std::string> const& lang, cmMakefile* mf, bool optional)
{
  for (std::string const& it : lang) {
    if (it == "ASM_NASM") {
      this->NasmEnabled = true;
    }
    if (it == "CUDA") {
      this->CudaEnabled = true;
    }
  }
  this->AddPlatformDefinitions(mf);
  cmGlobalVisualStudio8Generator::EnableLanguage(lang, mf, optional);
}

const char* cmGlobalVisualStudio10Generator::GetCustomVCTargetsPath() const
{
  if (this->CustomVCTargetsPath.empty()) {
    return nullptr;
  }
  return this->CustomVCTargetsPath.c_str();
}

const char* cmGlobalVisualStudio10Generator::GetPlatformToolset() const
{
  std::string const& toolset = this->GetPlatformToolsetString();
  if (toolset.empty()) {
    return nullptr;
  }
  return toolset.c_str();
}

std::string const& cmGlobalVisualStudio10Generator::GetPlatformToolsetString()
  const
{
  if (!this->GeneratorToolset.empty()) {
    return this->GeneratorToolset;
  }
  if (!this->DefaultPlatformToolset.empty()) {
    return this->DefaultPlatformToolset;
  }
  static std::string const empty;
  return empty;
}

const char* cmGlobalVisualStudio10Generator::GetPlatformToolsetVersion() const
{
  std::string const& version = this->GetPlatformToolsetVersionString();
  if (version.empty()) {
    return nullptr;
  }
  return version.c_str();
}

std::string const&
cmGlobalVisualStudio10Generator::GetPlatformToolsetVersionString() const
{
  if (!this->GeneratorToolsetVersion.empty()) {
    return this->GeneratorToolsetVersion;
  }
  static std::string const empty;
  return empty;
}

const char*
cmGlobalVisualStudio10Generator::GetPlatformToolsetHostArchitecture() const
{
  std::string const& hostArch =
    this->GetPlatformToolsetHostArchitectureString();
  if (hostArch.empty()) {
    return nullptr;
  }
  return hostArch.c_str();
}

std::string const&
cmGlobalVisualStudio10Generator::GetPlatformToolsetHostArchitectureString()
  const
{
  if (!this->GeneratorToolsetHostArchitecture.empty()) {
    return this->GeneratorToolsetHostArchitecture;
  }
  if (!this->DefaultPlatformToolsetHostArchitecture.empty()) {
    return this->DefaultPlatformToolsetHostArchitecture;
  }
  static std::string const empty;
  return empty;
}

const char* cmGlobalVisualStudio10Generator::GetPlatformToolsetCuda() const
{
  if (!this->GeneratorToolsetCuda.empty()) {
    return this->GeneratorToolsetCuda.c_str();
  }
  return nullptr;
}

std::string const&
cmGlobalVisualStudio10Generator::GetPlatformToolsetCudaString() const
{
  return this->GeneratorToolsetCuda;
}

const char* cmGlobalVisualStudio10Generator::GetPlatformToolsetCudaCustomDir()
  const
{
  if (!this->GeneratorToolsetCudaCustomDir.empty()) {
    return this->GeneratorToolsetCudaCustomDir.c_str();
  }
  return nullptr;
}

std::string const&
cmGlobalVisualStudio10Generator::GetPlatformToolsetCudaCustomDirString() const
{
  return this->GeneratorToolsetCudaCustomDir;
}

bool cmGlobalVisualStudio10Generator::IsDefaultToolset(
  const std::string&) const
{
  return true;
}

std::string cmGlobalVisualStudio10Generator::GetAuxiliaryToolset() const
{
  return {};
}

bool cmGlobalVisualStudio10Generator::FindMakeProgram(cmMakefile* mf)
{
  if (!this->cmGlobalVisualStudio8Generator::FindMakeProgram(mf)) {
    return false;
  }
  mf->AddDefinition("CMAKE_VS_MSBUILD_COMMAND", this->GetMSBuildCommand());
  return true;
}

std::string const& cmGlobalVisualStudio10Generator::GetMSBuildCommand()
{
  if (!this->MSBuildCommandInitialized) {
    this->MSBuildCommandInitialized = true;
    this->MSBuildCommand = this->FindMSBuildCommand();
  }
  return this->MSBuildCommand;
}

std::string cmGlobalVisualStudio10Generator::FindMSBuildCommand()
{
  std::string msbuild;
  std::string mskey;

  // Search in standard location.
  mskey = cmStrCat(
    "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\MSBuild\\ToolsVersions\\",
    this->GetToolsVersion(), ";MSBuildToolsPath");
  if (cmSystemTools::ReadRegistryValue(mskey.c_str(), msbuild,
                                       cmSystemTools::KeyWOW64_32)) {
    cmSystemTools::ConvertToUnixSlashes(msbuild);
    msbuild += "/MSBuild.exe";
    if (cmSystemTools::FileExists(msbuild, true)) {
      return msbuild;
    }
  }

  msbuild = "MSBuild.exe";
  return msbuild;
}

std::string cmGlobalVisualStudio10Generator::FindDevEnvCommand()
{
  if (this->ExpressEdition) {
    // Visual Studio Express >= 10 do not have "devenv.com" or
    // "VCExpress.exe" that we can use to build reliably.
    // Tell the caller it needs to use MSBuild instead.
    return "";
  }
  // Skip over the cmGlobalVisualStudio8Generator implementation because
  // we expect a real devenv and do not want to look for VCExpress.
  return this->cmGlobalVisualStudio71Generator::FindDevEnvCommand();
}

bool cmGlobalVisualStudio10Generator::FindVCTargetsPath(cmMakefile* mf)
{
  // Skip this in special cases within our own test suite.
  if (this->GetPlatformName() == "Test Platform" ||
      this->GetPlatformToolsetString() == "Test Toolset") {
    return true;
  }

  std::string wd;
  if (!this->ConfiguredFilesPath.empty()) {
    // In a try-compile we are given the outer CMakeFiles directory.
    wd = this->ConfiguredFilesPath;
  } else {
    wd = cmStrCat(this->GetCMakeInstance()->GetHomeOutputDirectory(),
                  "/CMakeFiles");
  }
  wd += "/";
  wd += cmVersion::GetCMakeVersion();

  // We record the result persistently in a file.
  std::string const txt = wd + "/VCTargetsPath.txt";

  // If we have a recorded result, use it.
  {
    cmsys::ifstream fin(txt.c_str());
    if (fin && cmSystemTools::GetLineFromStream(fin, this->VCTargetsPath) &&
        cmSystemTools::FileIsDirectory(this->VCTargetsPath)) {
      cmSystemTools::ConvertToUnixSlashes(this->VCTargetsPath);
      return true;
    }
  }

  // Prepare the work directory.
  if (!cmSystemTools::MakeDirectory(wd)) {
    std::string e = "Failed to make directory:\n  " + wd;
    mf->IssueMessage(MessageType::FATAL_ERROR, e);
    cmSystemTools::SetFatalErrorOccured();
    return false;
  }

  // Generate a project file for MSBuild to tell us the VCTargetsPath value.
  std::string const vcxproj = "VCTargetsPath.vcxproj";
  {
    std::string const vcxprojAbs = wd + "/" + vcxproj;
    cmsys::ofstream fout(vcxprojAbs.c_str());
    cmXMLWriter xw(fout);

    cmXMLDocument doc(xw);
    cmXMLElement eprj(doc, "Project");
    eprj.Attribute("DefaultTargets", "Build");
    eprj.Attribute("ToolsVersion", "4.0");
    eprj.Attribute("xmlns",
                   "http://schemas.microsoft.com/developer/msbuild/2003");
    if (this->IsNsightTegra()) {
      cmXMLElement epg(eprj, "PropertyGroup");
      epg.Attribute("Label", "NsightTegraProject");
      cmXMLElement(epg, "NsightTegraProjectRevisionNumber").Content("6");
    }
    {
      cmXMLElement eig(eprj, "ItemGroup");
      eig.Attribute("Label", "ProjectConfigurations");
      cmXMLElement epc(eig, "ProjectConfiguration");
      epc.Attribute("Include", "Debug|" + this->GetPlatformName());
      cmXMLElement(epc, "Configuration").Content("Debug");
      cmXMLElement(epc, "Platform").Content(this->GetPlatformName());
    }
    {
      cmXMLElement epg(eprj, "PropertyGroup");
      epg.Attribute("Label", "Globals");
      cmXMLElement(epg, "ProjectGuid")
        .Content("{F3FC6D86-508D-3FB1-96D2-995F08B142EC}");
      cmXMLElement(epg, "Keyword").Content("Win32Proj");
      cmXMLElement(epg, "Platform").Content(this->GetPlatformName());
      if (this->GetSystemName() == "WindowsPhone") {
        cmXMLElement(epg, "ApplicationType").Content("Windows Phone");
        cmXMLElement(epg, "ApplicationTypeRevision")
          .Content(this->GetApplicationTypeRevision());
      } else if (this->GetSystemName() == "WindowsStore") {
        cmXMLElement(epg, "ApplicationType").Content("Windows Store");
        cmXMLElement(epg, "ApplicationTypeRevision")
          .Content(this->GetApplicationTypeRevision());
      }
      if (!this->WindowsTargetPlatformVersion.empty()) {
        cmXMLElement(epg, "WindowsTargetPlatformVersion")
          .Content(this->WindowsTargetPlatformVersion);
      }
      if (this->GetPlatformName() == "ARM64") {
        cmXMLElement(epg, "WindowsSDKDesktopARM64Support").Content("true");
      } else if (this->GetPlatformName() == "ARM") {
        cmXMLElement(epg, "WindowsSDKDesktopARMSupport").Content("true");
      }
    }
    cmXMLElement(eprj, "Import")
      .Attribute("Project", "$(VCTargetsPath)\\Microsoft.Cpp.Default.props");
    if (const char* hostArch = this->GetPlatformToolsetHostArchitecture()) {
      cmXMLElement epg(eprj, "PropertyGroup");
      cmXMLElement(epg, "PreferredToolArchitecture").Content(hostArch);
    }
    {
      cmXMLElement epg(eprj, "PropertyGroup");
      epg.Attribute("Label", "Configuration");
      {
        cmXMLElement ect(epg, "ConfigurationType");
        if (this->IsNsightTegra()) {
          // Tegra-Android platform does not understand "Utility".
          ect.Content("StaticLibrary");
        } else {
          ect.Content("Utility");
        }
      }
      cmXMLElement(epg, "CharacterSet").Content("MultiByte");
      if (this->IsNsightTegra()) {
        cmXMLElement(epg, "NdkToolchainVersion")
          .Content(this->GetPlatformToolsetString());
      } else {
        cmXMLElement(epg, "PlatformToolset")
          .Content(this->GetPlatformToolsetString());
      }
    }
    cmXMLElement(eprj, "Import")
      .Attribute("Project", "$(VCTargetsPath)\\Microsoft.Cpp.props");
    {
      cmXMLElement eidg(eprj, "ItemDefinitionGroup");
      cmXMLElement epbe(eidg, "PostBuildEvent");
      cmXMLElement(epbe, "Command")
        .Content("echo VCTargetsPath=$(VCTargetsPath)");
    }
    cmXMLElement(eprj, "Import")
      .Attribute("Project", "$(VCTargetsPath)\\Microsoft.Cpp.targets");
  }

  std::vector<std::string> cmd;
  cmd.push_back(this->GetMSBuildCommand());
  cmd.push_back(vcxproj);
  cmd.push_back("/p:Configuration=Debug");
  cmd.push_back(std::string("/p:VisualStudioVersion=") +
                this->GetIDEVersion());
  std::string out;
  int ret = 0;
  cmsys::RegularExpression regex("\n *VCTargetsPath=([^%\r\n]+)[\r\n]");
  if (!cmSystemTools::RunSingleCommand(cmd, &out, &out, &ret, wd.c_str(),
                                       cmSystemTools::OUTPUT_NONE) ||
      ret != 0 || !regex.find(out)) {
    cmSystemTools::ReplaceString(out, "\n", "\n  ");
    std::ostringstream e;
    /* clang-format off */
    e <<
      "Failed to run MSBuild command:\n"
      "  " << cmd[0] << "\n"
      "to get the value of VCTargetsPath:\n"
      "  " << out << "\n"
      ;
    /* clang-format on */
    if (ret != 0) {
      e << "Exit code: " << ret << "\n";
    }
    mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
    cmSystemTools::SetFatalErrorOccured();
    return false;
  }
  this->VCTargetsPath = regex.match(1);
  cmSystemTools::ConvertToUnixSlashes(this->VCTargetsPath);

  {
    cmsys::ofstream fout(txt.c_str());
    fout << this->VCTargetsPath << "\n";
  }
  return true;
}

std::vector<cmGlobalGenerator::GeneratedMakeCommand>
cmGlobalVisualStudio10Generator::GenerateBuildCommand(
  const std::string& makeProgram, const std::string& projectName,
  const std::string& projectDir, std::vector<std::string> const& targetNames,
  const std::string& config, bool fast, int jobs, bool verbose,
  std::vector<std::string> const& makeOptions)
{
  std::vector<GeneratedMakeCommand> makeCommands;
  // Select the caller- or user-preferred make program, else MSBuild.
  std::string makeProgramSelected =
    this->SelectMakeProgram(makeProgram, this->GetMSBuildCommand());

  // Check if the caller explicitly requested a devenv tool.
  std::string makeProgramLower = makeProgramSelected;
  cmSystemTools::LowerCase(makeProgramLower);
  bool useDevEnv = (makeProgramLower.find("devenv") != std::string::npos ||
                    makeProgramLower.find("vcexpress") != std::string::npos);

  // Workaround to convince VCExpress.exe to produce output.
  const bool requiresOutputForward =
    (makeProgramLower.find("vcexpress") != std::string::npos);

  // MSBuild is preferred (and required for VS Express), but if the .sln has
  // an Intel Fortran .vfproj then we have to use devenv. Parse it to find out.
  cmSlnData slnData;
  {
    std::string slnFile;
    if (!projectDir.empty()) {
      slnFile = cmStrCat(projectDir, '/');
    }
    slnFile += projectName;
    slnFile += ".sln";
    cmVisualStudioSlnParser parser;
    if (parser.ParseFile(slnFile, slnData,
                         cmVisualStudioSlnParser::DataGroupProjects)) {
      std::vector<cmSlnProjectEntry> slnProjects = slnData.GetProjects();
      for (cmSlnProjectEntry const& project : slnProjects) {
        if (useDevEnv) {
          break;
        }
        std::string proj = project.GetRelativePath();
        if (proj.size() > 7 && proj.substr(proj.size() - 7) == ".vfproj") {
          useDevEnv = true;
        }
      }
    }
  }
  if (useDevEnv) {
    // Use devenv to build solutions containing Intel Fortran projects.
    return cmGlobalVisualStudio7Generator::GenerateBuildCommand(
      makeProgram, projectName, projectDir, targetNames, config, fast, jobs,
      verbose, makeOptions);
  }

  std::vector<std::string> realTargetNames = targetNames;
  if (targetNames.empty() ||
      ((targetNames.size() == 1) && targetNames.front().empty())) {
    realTargetNames = { "ALL_BUILD" };
  }
  for (const auto& tname : realTargetNames) {
    // msbuild.exe CxxOnly.sln /t:Build /p:Configuration=Debug
    // /target:ALL_BUILD
    //                         /m
    if (tname.empty()) {
      continue;
    }

    GeneratedMakeCommand makeCommand;
    makeCommand.RequiresOutputForward = requiresOutputForward;
    makeCommand.Add(makeProgramSelected);

    if (tname == "clean") {
      makeCommand.Add(std::string(projectName) + ".sln");
      makeCommand.Add("/t:Clean");
    } else {
      std::string targetProject = cmStrCat(tname, ".vcxproj");
      if (targetProject.find('/') == std::string::npos) {
        // it might be in a subdir
        if (cmSlnProjectEntry const* proj = slnData.GetProjectByName(tname)) {
          targetProject = proj->GetRelativePath();
          cmSystemTools::ConvertToUnixSlashes(targetProject);
        }
      }
      makeCommand.Add(std::move(targetProject));
    }
    std::string configArg = "/p:Configuration=";
    if (!config.empty()) {
      configArg += config;
    } else {
      configArg += "Debug";
    }
    makeCommand.Add(configArg);
    makeCommand.Add(std::string("/p:Platform=") + this->GetPlatformName());
    makeCommand.Add(std::string("/p:VisualStudioVersion=") +
                    this->GetIDEVersion());

    if (jobs != cmake::NO_BUILD_PARALLEL_LEVEL) {
      if (jobs == cmake::DEFAULT_BUILD_PARALLEL_LEVEL) {
        makeCommand.Add("/m");
      } else {
        makeCommand.Add(std::string("/m:") + std::to_string(jobs));
      }
      // Having msbuild.exe and cl.exe using multiple jobs is discouraged
      makeCommand.Add("/p:CL_MPCount=1");
    }

    // Respect the verbosity: 'n' normal will show build commands
    //                        'm' minimal only the build step's title
    makeCommand.Add(std::string("/v:") + ((verbose) ? "n" : "m"));
    makeCommand.Add(makeOptions.begin(), makeOptions.end());
    makeCommands.emplace_back(std::move(makeCommand));
  }
  return makeCommands;
}

bool cmGlobalVisualStudio10Generator::Find64BitTools(cmMakefile* mf)
{
  if (this->DefaultPlatformToolset == "v100") {
    // The v100 64-bit toolset does not exist in the express edition.
    this->DefaultPlatformToolset.clear();
  }
  if (this->GetPlatformToolset()) {
    return true;
  }
  // This edition does not come with 64-bit tools.  Look for them.
  //
  // TODO: Detect available tools?  x64\v100 exists but does not work?
  // HKLM\\SOFTWARE\\Microsoft\\MSBuild\\ToolsVersions\\4.0;VCTargetsPath
  // c:/Program Files (x86)/MSBuild/Microsoft.Cpp/v4.0/Platforms/
  //   {Itanium,Win32,x64}/PlatformToolsets/{v100,v90,Windows7.1SDK}
  std::string winSDK_7_1;
  if (cmSystemTools::ReadRegistryValue(
        "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft SDKs\\"
        "Windows\\v7.1;InstallationFolder",
        winSDK_7_1)) {
    std::ostringstream m;
    m << "Found Windows SDK v7.1: " << winSDK_7_1;
    mf->DisplayStatus(m.str(), -1);
    this->DefaultPlatformToolset = "Windows7.1SDK";
    return true;
  } else {
    std::ostringstream e;
    /* clang-format off */
    e << "Cannot enable 64-bit tools with Visual Studio 2010 Express.\n"
      << "Install the Microsoft Windows SDK v7.1 to get 64-bit tools:\n"
      << "  http://msdn.microsoft.com/en-us/windows/bb980924.aspx";
    /* clang-format on */
    mf->IssueMessage(MessageType::FATAL_ERROR, e.str().c_str());
    cmSystemTools::SetFatalErrorOccured();
    return false;
  }
}

std::string cmGlobalVisualStudio10Generator::GenerateRuleFile(
  std::string const& output) const
{
  // The VS 10 generator needs to create the .rule files on disk.
  // Hide them away under the CMakeFiles directory.
  std::string ruleDir = cmStrCat(
    this->GetCMakeInstance()->GetHomeOutputDirectory(), "/CMakeFiles/",
    cmSystemTools::ComputeStringMD5(cmSystemTools::GetFilenamePath(output)));
  std::string ruleFile =
    cmStrCat(ruleDir, '/', cmSystemTools::GetFilenameName(output), ".rule");
  return ruleFile;
}

void cmGlobalVisualStudio10Generator::PathTooLong(cmGeneratorTarget* target,
                                                  cmSourceFile const* sf,
                                                  std::string const& sfRel)
{
  size_t len =
    (target->GetLocalGenerator()->GetCurrentBinaryDirectory().length() + 1 +
     sfRel.length());
  if (len > this->LongestSource.Length) {
    this->LongestSource.Length = len;
    this->LongestSource.Target = target;
    this->LongestSource.SourceFile = sf;
    this->LongestSource.SourceRel = sfRel;
  }
}

std::string cmGlobalVisualStudio10Generator::Encoding()
{
  return "utf-8";
}

const char* cmGlobalVisualStudio10Generator::GetToolsVersion() const
{
  switch (this->Version) {
    case cmGlobalVisualStudioGenerator::VS9:
    case cmGlobalVisualStudioGenerator::VS10:
    case cmGlobalVisualStudioGenerator::VS11:
      return "4.0";

      // in Visual Studio 2013 they detached the MSBuild tools version
      // from the .Net Framework version and instead made it have it's own
      // version number
    case cmGlobalVisualStudioGenerator::VS12:
      return "12.0";
    case cmGlobalVisualStudioGenerator::VS14:
      return "14.0";
    case cmGlobalVisualStudioGenerator::VS15:
      return "15.0";
    case cmGlobalVisualStudioGenerator::VS16:
      return "16.0";
  }
  return "";
}

bool cmGlobalVisualStudio10Generator::IsNsightTegra() const
{
  return !this->NsightTegraVersion.empty();
}

std::string cmGlobalVisualStudio10Generator::GetNsightTegraVersion() const
{
  return this->NsightTegraVersion;
}

std::string cmGlobalVisualStudio10Generator::GetInstalledNsightTegraVersion()
{
  std::string version;
  cmSystemTools::ReadRegistryValue(
    "HKEY_LOCAL_MACHINE\\SOFTWARE\\NVIDIA Corporation\\Nsight Tegra;"
    "Version",
    version, cmSystemTools::KeyWOW64_32);
  return version;
}

std::string cmGlobalVisualStudio10Generator::GetApplicationTypeRevision() const
{
  // Return the first two '.'-separated components of the Windows version.
  std::string::size_type end1 = this->SystemVersion.find('.');
  std::string::size_type end2 =
    end1 == std::string::npos ? end1 : this->SystemVersion.find('.', end1 + 1);
  return this->SystemVersion.substr(0, end2);
}

static std::string cmLoadFlagTableString(Json::Value entry, const char* field)
{
  if (entry.isMember(field)) {
    auto string = entry[field];
    if (string.isConvertibleTo(Json::ValueType::stringValue)) {
      return string.asString();
    }
  }
  return "";
}

static unsigned int cmLoadFlagTableSpecial(Json::Value entry,
                                           const char* field)
{
  unsigned int value = 0;
  if (entry.isMember(field)) {
    auto specials = entry[field];
    if (specials.isArray()) {
      for (auto const& special : specials) {
        std::string s = special.asString();
        if (s == "UserValue") {
          value |= cmIDEFlagTable::UserValue;
        } else if (s == "UserIgnored") {
          value |= cmIDEFlagTable::UserIgnored;
        } else if (s == "UserRequired") {
          value |= cmIDEFlagTable::UserRequired;
        } else if (s == "Continue") {
          value |= cmIDEFlagTable::Continue;
        } else if (s == "SemicolonAppendable") {
          value |= cmIDEFlagTable::SemicolonAppendable;
        } else if (s == "UserFollowing") {
          value |= cmIDEFlagTable::UserFollowing;
        } else if (s == "CaseInsensitive") {
          value |= cmIDEFlagTable::CaseInsensitive;
        } else if (s == "SpaceAppendable") {
          value |= cmIDEFlagTable::SpaceAppendable;
        } else if (s == "CommaAppendable") {
          value |= cmIDEFlagTable::CommaAppendable;
        }
      }
    }
  }
  return value;
}

static cmIDEFlagTable const* cmLoadFlagTableJson(
  std::string const& flagJsonPath)
{
  cmIDEFlagTable* ret = nullptr;
  auto savedFlagIterator = loadedFlagJsonFiles.find(flagJsonPath);
  if (savedFlagIterator != loadedFlagJsonFiles.end()) {
    ret = savedFlagIterator->second.data();
  } else {
    Json::Reader reader;
    cmsys::ifstream stream;

    stream.open(flagJsonPath.c_str(), std::ios_base::in);
    if (stream) {
      Json::Value flags;
      if (reader.parse(stream, flags, false) && flags.isArray()) {
        std::vector<cmIDEFlagTable> flagTable;
        for (auto const& flag : flags) {
          cmIDEFlagTable flagEntry;
          flagEntry.IDEName = cmLoadFlagTableString(flag, "name");
          flagEntry.commandFlag = cmLoadFlagTableString(flag, "switch");
          flagEntry.comment = cmLoadFlagTableString(flag, "comment");
          flagEntry.value = cmLoadFlagTableString(flag, "value");
          flagEntry.special = cmLoadFlagTableSpecial(flag, "flags");
          flagTable.push_back(flagEntry);
        }
        cmIDEFlagTable endFlag{ "", "", "", "", 0 };
        flagTable.push_back(endFlag);

        loadedFlagJsonFiles[flagJsonPath] = flagTable;
        ret = loadedFlagJsonFiles[flagJsonPath].data();
      }
    }
  }
  return ret;
}

static std::string cmGetFlagTableName(std::string const& toolsetName,
                                      std::string const& table)
{
  return cmSystemTools::GetCMakeRoot() + "/Templates/MSBuild/FlagTables/" +
    toolsetName + "_" + table + ".json";
}

cmIDEFlagTable const* cmGlobalVisualStudio10Generator::LoadFlagTable(
  std::string const& optionsName, std::string const& toolsetName,
  std::string const& defaultName, std::string const& table) const
{
  cmIDEFlagTable const* ret = nullptr;

  std::string filename;
  if (!optionsName.empty()) {
    filename = cmGetFlagTableName(optionsName, table);
    ret = cmLoadFlagTableJson(filename);
  } else {
    filename = cmGetFlagTableName(toolsetName, table);
    if (cmSystemTools::FileExists(filename)) {
      ret = cmLoadFlagTableJson(filename);
    } else {
      filename = cmGetFlagTableName(defaultName, table);
      ret = cmLoadFlagTableJson(filename);
    }
  }

  if (!ret) {
    cmMakefile* mf = this->GetCurrentMakefile();

    std::ostringstream e;
    /* clang-format off */
    e << "JSON flag table \"" << filename <<
      "\" could not be loaded.\n";
    /* clang-format on */
    mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
  }
  return ret;
}

cmIDEFlagTable const* cmGlobalVisualStudio10Generator::GetClFlagTable() const
{
  std::string optionsName = this->ToolsetOptions.GetClFlagTableName(
    this->GetPlatformName(), this->GetPlatformToolsetString());
  std::string toolsetName = this->ToolsetOptions.GetToolsetName(
    this->GetPlatformName(), this->GetPlatformToolsetString());
  std::string defaultName = this->ToolsetOptions.GetToolsetName(
    this->GetPlatformName(), this->DefaultCLFlagTableName);
  return LoadFlagTable(optionsName, toolsetName, defaultName, "CL");
}

cmIDEFlagTable const* cmGlobalVisualStudio10Generator::GetCSharpFlagTable()
  const
{
  std::string optionsName = this->ToolsetOptions.GetCSharpFlagTableName(
    this->GetPlatformName(), this->GetPlatformToolsetString());
  std::string toolsetName = this->ToolsetOptions.GetToolsetName(
    this->GetPlatformName(), this->GetPlatformToolsetString());
  std::string defaultName = this->ToolsetOptions.GetToolsetName(
    this->GetPlatformName(), this->DefaultCSharpFlagTableName);
  return LoadFlagTable(optionsName, toolsetName, defaultName, "CSharp");
}

cmIDEFlagTable const* cmGlobalVisualStudio10Generator::GetRcFlagTable() const
{
  std::string optionsName = this->ToolsetOptions.GetRcFlagTableName(
    this->GetPlatformName(), this->GetPlatformToolsetString());
  std::string toolsetName = this->ToolsetOptions.GetToolsetName(
    this->GetPlatformName(), this->GetPlatformToolsetString());
  std::string defaultName = this->ToolsetOptions.GetToolsetName(
    this->GetPlatformName(), this->DefaultRCFlagTableName);
  return LoadFlagTable(optionsName, toolsetName, defaultName, "RC");
}

cmIDEFlagTable const* cmGlobalVisualStudio10Generator::GetLibFlagTable() const
{
  std::string optionsName = this->ToolsetOptions.GetLibFlagTableName(
    this->GetPlatformName(), this->GetPlatformToolsetString());
  std::string toolsetName = this->ToolsetOptions.GetToolsetName(
    this->GetPlatformName(), this->GetPlatformToolsetString());
  std::string defaultName = this->ToolsetOptions.GetToolsetName(
    this->GetPlatformName(), this->DefaultLibFlagTableName);
  return LoadFlagTable(optionsName, toolsetName, defaultName, "LIB");
}

cmIDEFlagTable const* cmGlobalVisualStudio10Generator::GetLinkFlagTable() const
{
  std::string optionsName = this->ToolsetOptions.GetLinkFlagTableName(
    this->GetPlatformName(), this->GetPlatformToolsetString());
  std::string toolsetName = this->ToolsetOptions.GetToolsetName(
    this->GetPlatformName(), this->GetPlatformToolsetString());
  std::string defaultName = this->ToolsetOptions.GetToolsetName(
    this->GetPlatformName(), this->DefaultLinkFlagTableName);
  return LoadFlagTable(optionsName, toolsetName, defaultName, "Link");
}

cmIDEFlagTable const* cmGlobalVisualStudio10Generator::GetCudaFlagTable() const
{
  std::string toolsetName = this->ToolsetOptions.GetToolsetName(
    this->GetPlatformName(), this->GetPlatformToolsetString());
  std::string defaultName = this->ToolsetOptions.GetToolsetName(
    this->GetPlatformName(), this->DefaultCudaFlagTableName);
  return LoadFlagTable("", toolsetName, defaultName, "Cuda");
}

cmIDEFlagTable const* cmGlobalVisualStudio10Generator::GetCudaHostFlagTable()
  const
{
  std::string toolsetName = this->ToolsetOptions.GetToolsetName(
    this->GetPlatformName(), this->GetPlatformToolsetString());
  std::string defaultName = this->ToolsetOptions.GetToolsetName(
    this->GetPlatformName(), this->DefaultCudaHostFlagTableName);
  return LoadFlagTable("", toolsetName, defaultName, "CudaHost");
}

cmIDEFlagTable const* cmGlobalVisualStudio10Generator::GetMasmFlagTable() const
{
  std::string optionsName = this->ToolsetOptions.GetMasmFlagTableName(
    this->GetPlatformName(), this->GetPlatformToolsetString());
  std::string toolsetName = this->ToolsetOptions.GetToolsetName(
    this->GetPlatformName(), this->GetPlatformToolsetString());
  std::string defaultName = this->ToolsetOptions.GetToolsetName(
    this->GetPlatformName(), this->DefaultMasmFlagTableName);
  return LoadFlagTable(optionsName, toolsetName, defaultName, "MASM");
}

cmIDEFlagTable const* cmGlobalVisualStudio10Generator::GetNasmFlagTable() const
{
  std::string toolsetName = this->ToolsetOptions.GetToolsetName(
    this->GetPlatformName(), this->GetPlatformToolsetString());
  std::string defaultName = this->ToolsetOptions.GetToolsetName(
    this->GetPlatformName(), this->DefaultNasmFlagTableName);
  return LoadFlagTable("", toolsetName, defaultName, "NASM");
}
