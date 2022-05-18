/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalVisualStudio10Generator.h"

#include <algorithm>
#include <cstring>
#include <map>
#include <sstream>
#include <utility>

#include <cm/memory>

#include <cm3p/json/reader.h>
#include <cm3p/json/value.h>

#include "cmsys/FStream.hxx"
#include "cmsys/Glob.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cmDocumentationEntry.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmGlobalGeneratorFactory.h"
#include "cmGlobalVisualStudio71Generator.h"
#include "cmGlobalVisualStudio7Generator.h"
#include "cmGlobalVisualStudioGenerator.h"
#include "cmIDEFlagTable.h"
#include "cmLocalGenerator.h"
#include "cmLocalVisualStudio10Generator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmSourceFile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
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
    const std::string& name, bool allowArch, cmake* cm) const override
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
    if (!allowArch || *p++ != ' ') {
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
    entry.Brief = "Deprecated.  Generates Visual Studio 2010 project files.  "
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

  this->Version = VSVersion::VS10;
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

  if (!this->CustomFlagTableDir.empty() &&
      !(cmSystemTools::FileIsFullPath(this->CustomFlagTableDir) &&
        cmSystemTools::FileIsDirectory(this->CustomFlagTableDir))) {
    std::ostringstream e;
    /* clang-format off */
    e <<
      "Generator\n"
      "  " << this->GetName() << "\n"
      "given toolset\n"
      "  customFlagTableDir=" << this->CustomFlagTableDir << "\n"
      "that is not an absolute path to an existing directory.";
    /* clang-format on */
    mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
    cmSystemTools::SetFatalErrorOccured();
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
    this->Version >= cmGlobalVisualStudioGenerator::VSVersion::VS16 ||
    (this->Version == cmGlobalVisualStudioGenerator::VSVersion::VS15 &&
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
        this->GetPlatformToolsetCudaVSIntegrationSubdirString() +
        "extras\\visual_studio_integration\\MSBuildExtensions";
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
    std::string versionToolset = this->GeneratorToolsetVersion;
    cmsys::RegularExpression regex("[0-9][0-9]\\.[0-9][0-9]");
    if (regex.find(versionToolset)) {
      versionToolset = "v" + versionToolset.erase(2, 1);
    } else {
      // Version not recognized. Clear it.
      versionToolset.clear();
    }

    if (!cmHasPrefix(versionToolset, this->GetPlatformToolsetString())) {
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

    std::string auxProps;
    switch (this->FindAuxToolset(this->GeneratorToolsetVersion, auxProps)) {
      case AuxToolset::None:
        this->GeneratorToolsetVersionProps = {};
        break;
      case AuxToolset::Default:
        // The given version is the default toolset.  Remove the setting.
        this->GeneratorToolsetVersion.clear();
        this->GeneratorToolsetVersionProps = {};
        break;
      case AuxToolset::PropsExist:
        this->GeneratorToolsetVersionProps = std::move(auxProps);
        break;
      case AuxToolset::PropsMissing: {
        std::ostringstream e;
        /* clang-format off */
        e <<
          "Generator\n"
          "  " << this->GetName() << "\n"
          "given toolset and version specification\n"
          "  " << this->GetPlatformToolsetString() << ",version=" <<
          this->GeneratorToolsetVersion << "\n"
          "does not seem to be installed at\n" <<
          "  " << auxProps;
        ;
        /* clang-format on */
        mf->IssueMessage(MessageType::FATAL_ERROR, e.str());

        // Clear the configured tool-set
        this->GeneratorToolsetVersion.clear();
        this->GeneratorToolsetVersionProps = {};
      } break;
    }
  }

  if (const char* toolset = this->GetPlatformToolset()) {
    mf->AddDefinition("CMAKE_VS_PLATFORM_TOOLSET", toolset);
  }
  if (!this->GeneratorToolsetVersion.empty()) {
    mf->AddDefinition("CMAKE_VS_PLATFORM_TOOLSET_VERSION",
                      this->GeneratorToolsetVersion);
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
      /* check for legacy toolkit folder structure */
      if (cmsys::SystemTools::FileIsDirectory(
            cmStrCat(this->GeneratorToolsetCudaCustomDir, "nvcc"))) {
        this->GeneratorToolsetCudaNvccSubdir = "nvcc\\";
      }
      if (cmsys::SystemTools::FileIsDirectory(
            cmStrCat(this->GeneratorToolsetCudaCustomDir,
                     "CUDAVisualStudioIntegration"))) {
        this->GeneratorToolsetCudaVSIntegrationSubdir =
          "CUDAVisualStudioIntegration\\";
      }
    } else {
      this->GeneratorToolsetCuda = value;
    }
    return true;
  }
  if (key == "customFlagTableDir") {
    this->CustomFlagTableDir = value;
    cmSystemTools::ConvertToUnixSlashes(this->CustomFlagTableDir);
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
    if (mf->GetSafeDefinition("CMAKE_GENERATOR_PLATFORM") == "Tegra-Android") {
      if (!this->InitializeTegraAndroid(mf)) {
        return false;
      }
    } else {
      this->SystemIsAndroid = true;
      if (!this->InitializeAndroid(mf)) {
        return false;
      }
    }
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

  if (this->GetVersion() == cmGlobalVisualStudioGenerator::VSVersion::VS12) {
    // VS 12 .NET CF defaults to .NET framework 3.9 for Windows CE.
    this->DefaultTargetFrameworkVersion = "v3.9";
    this->DefaultTargetFrameworkIdentifier = "WindowsEmbeddedCompact";
    this->DefaultTargetFrameworkTargetsVersion = "v8.0";
  }

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

bool cmGlobalVisualStudio10Generator::InitializeTegraAndroid(cmMakefile* mf)
{
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
  return true;
}

bool cmGlobalVisualStudio10Generator::InitializeAndroid(cmMakefile* mf)
{
  std::ostringstream e;
  e << this->GetName() << " does not support Android.";
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
  if (!this->AndroidExecutableWarnings.empty() &&
      !this->CMakeInstance->GetIsInTryCompile()) {
    std::ostringstream e;
    /* clang-format off */
    e <<
      "You are using Visual Studio tools for Android, which does not support "
      "standalone executables. However, the following executable targets do "
      "not have the ANDROID_GUI property set, and thus will not be built as "
      "expected. They will be built as shared libraries with executable "
      "filenames:\n"
      "  ";
    /* clang-format on */
    bool first = true;
    for (auto const& name : this->AndroidExecutableWarnings) {
      if (!first) {
        e << ", ";
      }
      first = false;
      e << name;
    }
    this->CMakeInstance->IssueMessage(MessageType::WARNING, e.str());
  }
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
  if (cmValue cached = this->CMakeInstance->GetState()->GetCacheEntryValue(
        "CMAKE_VS_NUGET_PACKAGE_RESTORE")) {
    this->CMakeInstance->MarkCliAsUsed("CMAKE_VS_NUGET_PACKAGE_RESTORE");
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
  if (this->SystemIsAndroid) {
    if (!this->DefaultAndroidToolset.empty()) {
      return this->DefaultAndroidToolset;
    }
  } else {
    if (!this->DefaultPlatformToolset.empty()) {
      return this->DefaultPlatformToolset;
    }
  }
  static std::string const empty;
  return empty;
}

std::string const&
cmGlobalVisualStudio10Generator::GetPlatformToolsetVersionProps() const
{
  return this->GeneratorToolsetVersionProps;
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

std::string const&
cmGlobalVisualStudio10Generator::GetPlatformToolsetCudaNvccSubdirString() const
{
  return this->GeneratorToolsetCudaNvccSubdir;
}

std::string const& cmGlobalVisualStudio10Generator::
  GetPlatformToolsetCudaVSIntegrationSubdirString() const
{
  return this->GeneratorToolsetCudaVSIntegrationSubdir;
}

cmGlobalVisualStudio10Generator::AuxToolset
cmGlobalVisualStudio10Generator::FindAuxToolset(std::string&,
                                                std::string&) const
{
  return AuxToolset::None;
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

cm::optional<std::string>
cmGlobalVisualStudio10Generator::FindMSBuildCommandEarly(cmMakefile*)
{
  return this->GetMSBuildCommand();
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
      cmXMLElement(epg, "Keyword")
        .Content(mf->GetSafeDefinition("CMAKE_SYSTEM_NAME") == "Android"
                   ? "Android"
                   : "Win32Proj");
      cmXMLElement(epg, "Platform").Content(this->GetPlatformName());
      if (this->GetSystemName() == "WindowsPhone") {
        cmXMLElement(epg, "ApplicationType").Content("Windows Phone");
        cmXMLElement(epg, "ApplicationTypeRevision")
          .Content(this->GetApplicationTypeRevision());
      } else if (this->GetSystemName() == "WindowsStore") {
        cmXMLElement(epg, "ApplicationType").Content("Windows Store");
        cmXMLElement(epg, "ApplicationTypeRevision")
          .Content(this->GetApplicationTypeRevision());
      } else if (this->GetSystemName() == "Android") {
        cmXMLElement(epg, "ApplicationType").Content("Android");
        cmXMLElement(epg, "ApplicationTypeRevision")
          .Content(this->GetApplicationTypeRevision());
      }
      if (!this->WindowsTargetPlatformVersion.empty()) {
        cmXMLElement(epg, "WindowsTargetPlatformVersion")
          .Content(this->WindowsTargetPlatformVersion);
      }
      if (this->GetSystemName() != "Android") {
        if (this->GetPlatformName() == "ARM64") {
          cmXMLElement(epg, "WindowsSDKDesktopARM64Support").Content("true");
        } else if (this->GetPlatformName() == "ARM") {
          cmXMLElement(epg, "WindowsSDKDesktopARMSupport").Content("true");
        }
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
  cmd.push_back(cmStrCat("/p:Platform=", this->GetPlatformName()));
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
  const std::string& config, int jobs, bool verbose,
  const cmBuildOptions& buildOptions,
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
                         cmVisualStudioSlnParser::DataGroupAll)) {
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
      makeProgram, projectName, projectDir, targetNames, config, jobs, verbose,
      buildOptions, makeOptions);
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
    cm::optional<cmSlnProjectEntry> proj = cm::nullopt;

    if (tname == "clean") {
      makeCommand.Add(cmStrCat(projectName, ".sln"));
      makeCommand.Add("/t:Clean");
    } else {
      std::string targetProject = cmStrCat(tname, ".vcxproj");
      proj = slnData.GetProjectByName(tname);
      if (targetProject.find('/') == std::string::npos) {
        // it might be in a subdir
        if (proj) {
          targetProject = proj->GetRelativePath();
          cmSystemTools::ConvertToUnixSlashes(targetProject);
        }
      }
      makeCommand.Add(targetProject);

      // Check if we do need a restore at all (i.e. if there are package
      // references and restore has not been disabled by a command line option.
      PackageResolveMode restoreMode = buildOptions.ResolveMode;
      bool requiresRestore = true;

      if (restoreMode == PackageResolveMode::Disable) {
        requiresRestore = false;
      } else if (cmValue cached =
                   this->CMakeInstance->GetState()->GetCacheEntryValue(
                     tname + "_REQUIRES_VS_PACKAGE_RESTORE")) {
        requiresRestore = cached.IsOn();
      } else {
        // There are no package references defined.
        requiresRestore = false;
      }

      // If a restore is required, evaluate the restore mode.
      if (requiresRestore) {
        if (restoreMode == PackageResolveMode::OnlyResolve) {
          // Only invoke the restore target on the project.
          makeCommand.Add("/t:Restore");
        } else {
          // Invoke restore target, unless it has been explicitly disabled.
          bool restorePackages = true;

          if (this->Version < VSVersion::VS15) {
            // Package restore is only supported starting from Visual Studio
            // 2017. Package restore must be executed manually using NuGet
            // shell for older versions.
            this->CMakeInstance->IssueMessage(
              MessageType::WARNING,
              "Restoring package references is only supported for Visual "
              "Studio 2017 and later. You have to manually restore the "
              "packages using NuGet before building the project.");
            restorePackages = false;
          } else if (restoreMode == PackageResolveMode::Default) {
            // Decide if a restore is performed, based on a cache variable.
            if (cmValue cached =
                  this->CMakeInstance->GetState()->GetCacheEntryValue(
                    "CMAKE_VS_NUGET_PACKAGE_RESTORE"))
              restorePackages = cached.IsOn();
          }

          if (restorePackages) {
            if (this->IsMsBuildRestoreSupported()) {
              makeCommand.Add("/restore");
            } else {
              GeneratedMakeCommand restoreCommand;
              restoreCommand.Add(makeProgramSelected);
              restoreCommand.Add(targetProject);
              restoreCommand.Add("/t:Restore");
              makeCommands.emplace_back(restoreCommand);
            }
          }
        }
      }
    }

    std::string plainConfig = config;
    if (config.empty()) {
      plainConfig = "Debug";
    }

    std::string platform = GetPlatformName();
    if (proj) {
      std::string extension =
        cmSystemTools::GetFilenameLastExtension(proj->GetRelativePath());
      extension = cmSystemTools::LowerCase(extension);
      if (extension.compare(".csproj") == 0) {
        // Use correct platform name
        platform =
          slnData.GetConfigurationTarget(tname, plainConfig, platform);
      }
    }

    makeCommand.Add(cmStrCat("/p:Configuration=", plainConfig));
    makeCommand.Add(cmStrCat("/p:Platform=", platform));
    makeCommand.Add(
      cmStrCat("/p:VisualStudioVersion=", this->GetIDEVersion()));

    if (jobs != cmake::NO_BUILD_PARALLEL_LEVEL) {
      if (jobs == cmake::DEFAULT_BUILD_PARALLEL_LEVEL) {
        makeCommand.Add("/m");
      } else {
        makeCommand.Add(cmStrCat("/m:", std::to_string(jobs)));
      }
      // Having msbuild.exe and cl.exe using multiple jobs is discouraged
      makeCommand.Add("/p:CL_MPCount=1");
    }

    // Respect the verbosity: 'n' normal will show build commands
    //                        'm' minimal only the build step's title
    makeCommand.Add(cmStrCat("/v:", ((verbose) ? "n" : "m")));
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
    case cmGlobalVisualStudioGenerator::VSVersion::VS9:
    case cmGlobalVisualStudioGenerator::VSVersion::VS10:
    case cmGlobalVisualStudioGenerator::VSVersion::VS11:
      return "4.0";

      // in Visual Studio 2013 they detached the MSBuild tools version
      // from the .Net Framework version and instead made it have it's own
      // version number
    case cmGlobalVisualStudioGenerator::VSVersion::VS12:
      return "12.0";
    case cmGlobalVisualStudioGenerator::VSVersion::VS14:
      return "14.0";
    case cmGlobalVisualStudioGenerator::VSVersion::VS15:
      return "15.0";
    case cmGlobalVisualStudioGenerator::VSVersion::VS16:
      return "16.0";
    case cmGlobalVisualStudioGenerator::VSVersion::VS17:
      return "17.0";
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
  if (this->GetSystemName() == "Android") {
    return this->GetAndroidApplicationTypeRevision();
  }

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

namespace {

cmIDEFlagTable const* cmLoadFlagTableJson(std::string const& flagJsonPath,
                                          cm::optional<std::string> vsVer)
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
          Json::Value const& vsminJson = flag["vsmin"];
          if (vsminJson.isString()) {
            std::string const& vsmin = vsminJson.asString();
            if (!vsmin.empty()) {
              if (!vsVer ||
                  cmSystemTools::VersionCompareGreater(vsmin, *vsVer)) {
                continue;
              }
            }
          }
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
}

cm::optional<std::string> cmGlobalVisualStudio10Generator::FindFlagTable(
  cm::string_view toolsetName, cm::string_view table) const
{
  if (!this->CustomFlagTableDir.empty()) {
    std::string customFlagTableFile =
      cmStrCat(this->CustomFlagTableDir, '/', this->GetPlatformName(), '_',
               toolsetName, '_', table, ".json");
    if (cmSystemTools::FileExists(customFlagTableFile)) {
      return customFlagTableFile;
    }
    customFlagTableFile =
      cmStrCat(this->CustomFlagTableDir, '/', this->GetPlatformName(), '_',
               table, ".json");
    if (cmSystemTools::FileExists(customFlagTableFile)) {
      return customFlagTableFile;
    }
  }
  std::string fullPath =
    cmStrCat(cmSystemTools::GetCMakeRoot(), "/Templates/MSBuild/FlagTables/",
             toolsetName, '_', table, ".json");
  if (cmSystemTools::FileExists(fullPath)) {
    return fullPath;
  }
  return {};
}

cmIDEFlagTable const* cmGlobalVisualStudio10Generator::LoadFlagTable(
  std::string const& toolSpecificName, std::string const& defaultName,
  std::string const& table) const
{
  cmMakefile* mf = this->GetCurrentMakefile();

  std::string filename;
  if (!toolSpecificName.empty()) {
    if (cm::optional<std::string> found =
          this->FindFlagTable(toolSpecificName, table)) {
      filename = std::move(*found);
    } else {
      mf->IssueMessage(MessageType::FATAL_ERROR,
                       cmStrCat("JSON flag table for ", table,
                                " not found for toolset ", toolSpecificName));
      return nullptr;
    }
  } else {
    std::string const& genericName =
      this->CanonicalToolsetName(this->GetPlatformToolsetString());
    cm::optional<std::string> found = this->FindFlagTable(genericName, table);
    if (!found) {
      found = this->FindFlagTable(defaultName, table);
    }
    if (found) {
      filename = std::move(*found);
    } else {
      mf->IssueMessage(MessageType::FATAL_ERROR,
                       cmStrCat("JSON flag table for ", table,
                                " not found for toolset ", genericName, " ",
                                defaultName));
      return nullptr;
    }
  }

  cm::optional<std::string> vsVer = this->GetVSInstanceVersion();
  if (cmIDEFlagTable const* ret = cmLoadFlagTableJson(filename, vsVer)) {
    return ret;
  }

  mf->IssueMessage(
    MessageType::FATAL_ERROR,
    cmStrCat("JSON flag table could not be loaded:\n  ", filename));
  return nullptr;
}

cmIDEFlagTable const* cmGlobalVisualStudio10Generator::GetClFlagTable() const
{
  return LoadFlagTable(this->GetClFlagTableName(),
                       this->DefaultCLFlagTableName, "CL");
}

cmIDEFlagTable const* cmGlobalVisualStudio10Generator::GetCSharpFlagTable()
  const
{
  return LoadFlagTable(this->GetCSharpFlagTableName(),
                       this->DefaultCSharpFlagTableName, "CSharp");
}

cmIDEFlagTable const* cmGlobalVisualStudio10Generator::GetRcFlagTable() const
{
  return LoadFlagTable(this->GetRcFlagTableName(),
                       this->DefaultRCFlagTableName, "RC");
}

cmIDEFlagTable const* cmGlobalVisualStudio10Generator::GetLibFlagTable() const
{
  return LoadFlagTable(this->GetLibFlagTableName(),
                       this->DefaultLibFlagTableName, "LIB");
}

cmIDEFlagTable const* cmGlobalVisualStudio10Generator::GetLinkFlagTable() const
{
  return LoadFlagTable(this->GetLinkFlagTableName(),
                       this->DefaultLinkFlagTableName, "Link");
}

cmIDEFlagTable const* cmGlobalVisualStudio10Generator::GetCudaFlagTable() const
{
  return LoadFlagTable(std::string(), this->DefaultCudaFlagTableName, "Cuda");
}

cmIDEFlagTable const* cmGlobalVisualStudio10Generator::GetCudaHostFlagTable()
  const
{
  return LoadFlagTable(std::string(), this->DefaultCudaHostFlagTableName,
                       "CudaHost");
}

cmIDEFlagTable const* cmGlobalVisualStudio10Generator::GetMasmFlagTable() const
{
  return LoadFlagTable(this->GetMasmFlagTableName(),
                       this->DefaultMasmFlagTableName, "MASM");
}

cmIDEFlagTable const* cmGlobalVisualStudio10Generator::GetNasmFlagTable() const
{
  return LoadFlagTable(std::string(), this->DefaultNasmFlagTableName, "NASM");
}

bool cmGlobalVisualStudio10Generator::IsMsBuildRestoreSupported() const
{
  if (this->Version >= VSVersion::VS16) {
    return true;
  }

  static std::string const vsVer15_7_5 = "15.7.27703.2042";
  cm::optional<std::string> vsVer = this->GetVSInstanceVersion();
  return (vsVer &&
          cmSystemTools::VersionCompareGreaterEq(*vsVer, vsVer15_7_5));
}

std::string cmGlobalVisualStudio10Generator::GetClFlagTableName() const
{
  std::string const& toolset = this->GetPlatformToolsetString();
  std::string const useToolset = this->CanonicalToolsetName(toolset);

  if (toolset == "v142") {
    return "v142";
  } else if (toolset == "v141") {
    return "v141";
  } else if (useToolset == "v140") {
    return "v140";
  } else if (useToolset == "v120") {
    return "v12";
  } else if (useToolset == "v110") {
    return "v11";
  } else if (useToolset == "v100") {
    return "v10";
  } else {
    return "";
  }
}

std::string cmGlobalVisualStudio10Generator::GetCSharpFlagTableName() const
{
  std::string const& toolset = this->GetPlatformToolsetString();
  std::string const useToolset = this->CanonicalToolsetName(toolset);

  if (useToolset == "v142") {
    return "v142";
  } else if (useToolset == "v141") {
    return "v141";
  } else if (useToolset == "v140") {
    return "v140";
  } else if (useToolset == "v120") {
    return "v12";
  } else if (useToolset == "v110") {
    return "v11";
  } else if (useToolset == "v100") {
    return "v10";
  } else {
    return "";
  }
}

std::string cmGlobalVisualStudio10Generator::GetRcFlagTableName() const
{
  std::string const& toolset = this->GetPlatformToolsetString();
  std::string const useToolset = this->CanonicalToolsetName(toolset);

  if ((useToolset == "v140") || (useToolset == "v141") ||
      (useToolset == "v142")) {
    return "v14";
  } else if (useToolset == "v120") {
    return "v12";
  } else if (useToolset == "v110") {
    return "v11";
  } else if (useToolset == "v100") {
    return "v10";
  } else {
    return "";
  }
}

std::string cmGlobalVisualStudio10Generator::GetLibFlagTableName() const
{
  std::string const& toolset = this->GetPlatformToolsetString();
  std::string const useToolset = this->CanonicalToolsetName(toolset);

  if ((useToolset == "v140") || (useToolset == "v141") ||
      (useToolset == "v142")) {
    return "v14";
  } else if (useToolset == "v120") {
    return "v12";
  } else if (useToolset == "v110") {
    return "v11";
  } else if (useToolset == "v100") {
    return "v10";
  } else {
    return "";
  }
}

std::string cmGlobalVisualStudio10Generator::GetLinkFlagTableName() const
{
  std::string const& toolset = this->GetPlatformToolsetString();
  std::string const useToolset = this->CanonicalToolsetName(toolset);

  if (useToolset == "v142") {
    return "v142";
  } else if (useToolset == "v141") {
    return "v141";
  } else if (useToolset == "v140") {
    return "v140";
  } else if (useToolset == "v120") {
    return "v12";
  } else if (useToolset == "v110") {
    return "v11";
  } else if (useToolset == "v100") {
    return "v10";
  } else {
    return "";
  }
}

std::string cmGlobalVisualStudio10Generator::GetMasmFlagTableName() const
{
  std::string const& toolset = this->GetPlatformToolsetString();
  std::string const useToolset = this->CanonicalToolsetName(toolset);

  if ((useToolset == "v140") || (useToolset == "v141") ||
      (useToolset == "v142")) {
    return "v14";
  } else if (useToolset == "v120") {
    return "v12";
  } else if (useToolset == "v110") {
    return "v11";
  } else if (useToolset == "v100") {
    return "v10";
  } else {
    return "";
  }
}

std::string cmGlobalVisualStudio10Generator::CanonicalToolsetName(
  std::string const& toolset) const
{
  std::size_t length = toolset.length();

  if (cmHasLiteralSuffix(toolset, "_xp")) {
    length -= 3;
  }

  return toolset.substr(0, length);
}
