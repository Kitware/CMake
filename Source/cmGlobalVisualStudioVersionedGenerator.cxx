/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalVisualStudioVersionedGenerator.h"

#include <cstring>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

#include <cmext/string_view>

#include "cmsys/FStream.hxx"
#include "cmsys/Glob.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cmGlobalGenerator.h"
#include "cmGlobalGeneratorFactory.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmVSSetupHelper.h"
#include "cmake.h"

#ifndef IMAGE_FILE_MACHINE_ARM64
#  define IMAGE_FILE_MACHINE_ARM64 0xaa64 // ARM64 Little-Endian
#endif

static bool VSIsWow64()
{
  BOOL isWow64 = false;
  return IsWow64Process(GetCurrentProcess(), &isWow64) && isWow64;
}

static bool VSIsArm64Host()
{
  typedef BOOL(WINAPI * CM_ISWOW64PROCESS2)(
    HANDLE hProcess, USHORT * pProcessMachine, USHORT * pNativeMachine);

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#  define CM_VS_GCC_DIAGNOSTIC_PUSHED
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
  static const CM_ISWOW64PROCESS2 s_IsWow64Process2Impl =
    (CM_ISWOW64PROCESS2)GetProcAddress(
      GetModuleHandleW(L"api-ms-win-core-wow64-l1-1-1.dll"),
      "IsWow64Process2");
#ifdef CM_VS_GCC_DIAGNOSTIC_PUSHED
#  pragma GCC diagnostic pop
#  undef CM_VS_GCC_DIAGNOSTIC_PUSHED
#endif

  USHORT processMachine;
  USHORT nativeMachine;

  return s_IsWow64Process2Impl != nullptr &&
    s_IsWow64Process2Impl(GetCurrentProcess(), &processMachine,
                          &nativeMachine) &&
    nativeMachine == IMAGE_FILE_MACHINE_ARM64;
}

static bool VSHasDotNETFrameworkArm64()
{
  std::string dotNetArm64;
  return cmSystemTools::ReadRegistryValue(
    R"(HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\.NETFramework;InstallRootArm64)",
    dotNetArm64, cmSystemTools::KeyWOW64_64);
}

static bool VSIsWindows11OrGreater()
{
  cmSystemTools::WindowsVersion const windowsVersion =
    cmSystemTools::GetWindowsVersion();
  return (windowsVersion.dwMajorVersion > 10 ||
          (windowsVersion.dwMajorVersion == 10 &&
           windowsVersion.dwMinorVersion > 0) ||
          (windowsVersion.dwMajorVersion == 10 &&
           windowsVersion.dwMinorVersion == 0 &&
           windowsVersion.dwBuildNumber >= 22000));
}

static std::string VSHostPlatformName()
{
  if (VSIsArm64Host()) {
    return "ARM64";
  }
  if (VSIsWow64()) {
    return "x64";
  }
#if defined(_M_ARM)
  return "ARM";
#elif defined(_M_IA64)
  return "Itanium";
#elif defined(_WIN64)
  return "x64";
#else
  return "Win32";
#endif
}

static std::string VSHostArchitecture(
  cmGlobalVisualStudioGenerator::VSVersion v)
{
  if (VSIsArm64Host()) {
    return v >= cmGlobalVisualStudioGenerator::VSVersion::VS17 ? "ARM64" : "";
  }
  if (VSIsWow64()) {
    return "x64";
  }
#if defined(_M_ARM)
  return "";
#elif defined(_M_IA64)
  return "";
#elif defined(_WIN64)
  return "x64";
#else
  return "x86";
#endif
}

static unsigned int VSVersionToMajor(
  cmGlobalVisualStudioGenerator::VSVersion v)
{
  switch (v) {
    case cmGlobalVisualStudioGenerator::VSVersion::VS9:
      return 9;
    case cmGlobalVisualStudioGenerator::VSVersion::VS12:
      return 12;
    case cmGlobalVisualStudioGenerator::VSVersion::VS14:
      return 14;
    case cmGlobalVisualStudioGenerator::VSVersion::VS15:
      return 15;
    case cmGlobalVisualStudioGenerator::VSVersion::VS16:
      return 16;
    case cmGlobalVisualStudioGenerator::VSVersion::VS17:
      return 17;
  }
  return 0;
}

static const char* VSVersionToToolset(
  cmGlobalVisualStudioGenerator::VSVersion v)
{
  switch (v) {
    case cmGlobalVisualStudioGenerator::VSVersion::VS9:
      return "v90";
    case cmGlobalVisualStudioGenerator::VSVersion::VS12:
      return "v120";
    case cmGlobalVisualStudioGenerator::VSVersion::VS14:
      return "v140";
    case cmGlobalVisualStudioGenerator::VSVersion::VS15:
      return "v141";
    case cmGlobalVisualStudioGenerator::VSVersion::VS16:
      return "v142";
    case cmGlobalVisualStudioGenerator::VSVersion::VS17:
      return "v143";
  }
  return "";
}

static std::string VSVersionToMajorString(
  cmGlobalVisualStudioGenerator::VSVersion v)
{
  switch (v) {
    case cmGlobalVisualStudioGenerator::VSVersion::VS9:
      return "9";
    case cmGlobalVisualStudioGenerator::VSVersion::VS12:
      return "12";
    case cmGlobalVisualStudioGenerator::VSVersion::VS14:
      return "14";
    case cmGlobalVisualStudioGenerator::VSVersion::VS15:
      return "15";
    case cmGlobalVisualStudioGenerator::VSVersion::VS16:
      return "16";
    case cmGlobalVisualStudioGenerator::VSVersion::VS17:
      return "17";
  }
  return "";
}

static const char* VSVersionToAndroidToolset(
  cmGlobalVisualStudioGenerator::VSVersion v)
{
  switch (v) {
    case cmGlobalVisualStudioGenerator::VSVersion::VS9:
    case cmGlobalVisualStudioGenerator::VSVersion::VS12:
      return "";
    case cmGlobalVisualStudioGenerator::VSVersion::VS14:
      return "Clang_3_8";
    case cmGlobalVisualStudioGenerator::VSVersion::VS15:
    case cmGlobalVisualStudioGenerator::VSVersion::VS16:
    case cmGlobalVisualStudioGenerator::VSVersion::VS17:
      return "Clang_5_0";
  }
  return "";
}

static const char vs15generatorName[] = "Visual Studio 15 2017";

// Map generator name without year to name with year.
static const char* cmVS15GenName(const std::string& name, std::string& genName)
{
  if (strncmp(name.c_str(), vs15generatorName,
              sizeof(vs15generatorName) - 6) != 0) {
    return nullptr;
  }
  const char* p = name.c_str() + sizeof(vs15generatorName) - 6;
  if (cmHasLiteralPrefix(p, " 2017")) {
    p += 5;
  }
  genName = std::string(vs15generatorName) + p;
  return p;
}

class cmGlobalVisualStudioVersionedGenerator::Factory15
  : public cmGlobalGeneratorFactory
{
public:
  std::unique_ptr<cmGlobalGenerator> CreateGlobalGenerator(
    const std::string& name, bool allowArch, cmake* cm) const override
  {
    std::string genName;
    const char* p = cmVS15GenName(name, genName);
    if (!p) {
      return std::unique_ptr<cmGlobalGenerator>();
    }
    if (!*p) {
      return std::unique_ptr<cmGlobalGenerator>(
        new cmGlobalVisualStudioVersionedGenerator(
          cmGlobalVisualStudioGenerator::VSVersion::VS15, cm, genName, ""));
    }
    if (!allowArch || *p++ != ' ') {
      return std::unique_ptr<cmGlobalGenerator>();
    }
    if (strcmp(p, "Win64") == 0) {
      return std::unique_ptr<cmGlobalGenerator>(
        new cmGlobalVisualStudioVersionedGenerator(
          cmGlobalVisualStudioGenerator::VSVersion::VS15, cm, genName, "x64"));
    }
    if (strcmp(p, "ARM") == 0) {
      return std::unique_ptr<cmGlobalGenerator>(
        new cmGlobalVisualStudioVersionedGenerator(
          cmGlobalVisualStudioGenerator::VSVersion::VS15, cm, genName, "ARM"));
    }
    return std::unique_ptr<cmGlobalGenerator>();
  }

  cmDocumentationEntry GetDocumentation() const override
  {
    return { std::string(vs15generatorName) + " [arch]",
             "Generates Visual Studio 2017 project files.  "
             "Optional [arch] can be \"Win64\" or \"ARM\"." };
  }

  std::vector<std::string> GetGeneratorNames() const override
  {
    std::vector<std::string> names;
    names.push_back(vs15generatorName);
    return names;
  }

  std::vector<std::string> GetGeneratorNamesWithPlatform() const override
  {
    std::vector<std::string> names;
    names.push_back(vs15generatorName + std::string(" ARM"));
    names.push_back(vs15generatorName + std::string(" Win64"));
    return names;
  }

  bool SupportsToolset() const override { return true; }
  bool SupportsPlatform() const override { return true; }

  std::vector<std::string> GetKnownPlatforms() const override
  {
    std::vector<std::string> platforms;
    platforms.emplace_back("x64");
    platforms.emplace_back("Win32");
    platforms.emplace_back("ARM");
    platforms.emplace_back("ARM64");
    return platforms;
  }

  std::string GetDefaultPlatformName() const override { return "Win32"; }
};

std::unique_ptr<cmGlobalGeneratorFactory>
cmGlobalVisualStudioVersionedGenerator::NewFactory15()
{
  return std::unique_ptr<cmGlobalGeneratorFactory>(new Factory15);
}

static const char vs16generatorName[] = "Visual Studio 16 2019";
static const char vs17generatorName[] = "Visual Studio 17 2022";

// Map generator name without year to name with year.
static const char* cmVS16GenName(const std::string& name, std::string& genName)
{
  if (strncmp(name.c_str(), vs16generatorName,
              sizeof(vs16generatorName) - 6) != 0) {
    return nullptr;
  }
  const char* p = name.c_str() + sizeof(vs16generatorName) - 6;
  if (cmHasLiteralPrefix(p, " 2019")) {
    p += 5;
  }
  genName = std::string(vs16generatorName) + p;
  return p;
}

static const char* cmVS17GenName(const std::string& name, std::string& genName)
{
  if (strncmp(name.c_str(), vs17generatorName,
              sizeof(vs17generatorName) - 6) != 0) {
    return nullptr;
  }
  const char* p = name.c_str() + sizeof(vs17generatorName) - 6;
  if (cmHasLiteralPrefix(p, " 2022")) {
    p += 5;
  }
  genName = std::string(vs17generatorName) + p;
  return p;
}

class cmGlobalVisualStudioVersionedGenerator::Factory16
  : public cmGlobalGeneratorFactory
{
public:
  std::unique_ptr<cmGlobalGenerator> CreateGlobalGenerator(
    const std::string& name, bool /*allowArch*/, cmake* cm) const override
  {
    std::string genName;
    const char* p = cmVS16GenName(name, genName);
    if (!p) {
      return std::unique_ptr<cmGlobalGenerator>();
    }
    if (!*p) {
      return std::unique_ptr<cmGlobalGenerator>(
        new cmGlobalVisualStudioVersionedGenerator(
          cmGlobalVisualStudioGenerator::VSVersion::VS16, cm, genName, ""));
    }
    return std::unique_ptr<cmGlobalGenerator>();
  }

  cmDocumentationEntry GetDocumentation() const override
  {
    return { std::string(vs16generatorName),
             "Generates Visual Studio 2019 project files.  "
             "Use -A option to specify architecture." };
  }

  std::vector<std::string> GetGeneratorNames() const override
  {
    std::vector<std::string> names;
    names.push_back(vs16generatorName);
    return names;
  }

  std::vector<std::string> GetGeneratorNamesWithPlatform() const override
  {
    return std::vector<std::string>();
  }

  bool SupportsToolset() const override { return true; }
  bool SupportsPlatform() const override { return true; }

  std::vector<std::string> GetKnownPlatforms() const override
  {
    std::vector<std::string> platforms;
    platforms.emplace_back("x64");
    platforms.emplace_back("Win32");
    platforms.emplace_back("ARM");
    platforms.emplace_back("ARM64");
    platforms.emplace_back("ARM64EC");
    return platforms;
  }

  std::string GetDefaultPlatformName() const override
  {
    return VSHostPlatformName();
  }
};

std::unique_ptr<cmGlobalGeneratorFactory>
cmGlobalVisualStudioVersionedGenerator::NewFactory16()
{
  return std::unique_ptr<cmGlobalGeneratorFactory>(new Factory16);
}

class cmGlobalVisualStudioVersionedGenerator::Factory17
  : public cmGlobalGeneratorFactory
{
public:
  std::unique_ptr<cmGlobalGenerator> CreateGlobalGenerator(
    const std::string& name, bool /*allowArch*/, cmake* cm) const override
  {
    std::string genName;
    const char* p = cmVS17GenName(name, genName);
    if (!p) {
      return std::unique_ptr<cmGlobalGenerator>();
    }
    if (!*p) {
      return std::unique_ptr<cmGlobalGenerator>(
        new cmGlobalVisualStudioVersionedGenerator(
          cmGlobalVisualStudioGenerator::VSVersion::VS17, cm, genName, ""));
    }
    return std::unique_ptr<cmGlobalGenerator>();
  }

  cmDocumentationEntry GetDocumentation() const override
  {
    return { std::string(vs17generatorName),
             "Generates Visual Studio 2022 project files.  "
             "Use -A option to specify architecture." };
  }

  std::vector<std::string> GetGeneratorNames() const override
  {
    std::vector<std::string> names;
    names.push_back(vs17generatorName);
    return names;
  }

  std::vector<std::string> GetGeneratorNamesWithPlatform() const override
  {
    return std::vector<std::string>();
  }

  bool SupportsToolset() const override { return true; }
  bool SupportsPlatform() const override { return true; }

  std::vector<std::string> GetKnownPlatforms() const override
  {
    std::vector<std::string> platforms;
    platforms.emplace_back("x64");
    platforms.emplace_back("Win32");
    platforms.emplace_back("ARM");
    platforms.emplace_back("ARM64");
    platforms.emplace_back("ARM64EC");
    return platforms;
  }

  std::string GetDefaultPlatformName() const override
  {
    return VSHostPlatformName();
  }
};

std::unique_ptr<cmGlobalGeneratorFactory>
cmGlobalVisualStudioVersionedGenerator::NewFactory17()
{
  return std::unique_ptr<cmGlobalGeneratorFactory>(new Factory17);
}

cmGlobalVisualStudioVersionedGenerator::cmGlobalVisualStudioVersionedGenerator(
  VSVersion version, cmake* cm, const std::string& name,
  std::string const& platformInGeneratorName)
  : cmGlobalVisualStudio14Generator(cm, name, platformInGeneratorName)
  , vsSetupAPIHelper(VSVersionToMajor(version))
{
  this->Version = version;
  this->ExpressEdition = false;
  this->DefaultPlatformToolset = VSVersionToToolset(this->Version);
  this->DefaultAndroidToolset = VSVersionToAndroidToolset(this->Version);
  this->DefaultCLFlagTableName = VSVersionToToolset(this->Version);
  this->DefaultCSharpFlagTableName = VSVersionToToolset(this->Version);
  this->DefaultLinkFlagTableName = VSVersionToToolset(this->Version);
  if (this->Version >= cmGlobalVisualStudioGenerator::VSVersion::VS16) {
    this->DefaultPlatformName = VSHostPlatformName();
    this->DefaultPlatformToolsetHostArchitecture =
      VSHostArchitecture(this->Version);
  }
  if (this->Version >= cmGlobalVisualStudioGenerator::VSVersion::VS17) {
    // FIXME: Search for an existing framework?  Under '%ProgramFiles(x86)%',
    // see 'Reference Assemblies\Microsoft\Framework\.NETFramework'.
    // Use a version installed by VS 2022 without a separate component.
    this->DefaultTargetFrameworkVersion = "v4.7.2";
  }
}

bool cmGlobalVisualStudioVersionedGenerator::MatchesGeneratorName(
  const std::string& name) const
{
  std::string genName;
  switch (this->Version) {
    case cmGlobalVisualStudioGenerator::VSVersion::VS9:
    case cmGlobalVisualStudioGenerator::VSVersion::VS12:
    case cmGlobalVisualStudioGenerator::VSVersion::VS14:
      break;
    case cmGlobalVisualStudioGenerator::VSVersion::VS15:
      if (cmVS15GenName(name, genName)) {
        return genName == this->GetName();
      }
      break;
    case cmGlobalVisualStudioGenerator::VSVersion::VS16:
      if (cmVS16GenName(name, genName)) {
        return genName == this->GetName();
      }
      break;
    case cmGlobalVisualStudioGenerator::VSVersion::VS17:
      if (cmVS17GenName(name, genName)) {
        return genName == this->GetName();
      }
      break;
  }
  return false;
}

bool cmGlobalVisualStudioVersionedGenerator::SetGeneratorInstance(
  std::string const& i, cmMakefile* mf)
{
  if (this->LastGeneratorInstanceString &&
      i == *(this->LastGeneratorInstanceString)) {
    this->SetVSVersionVar(mf);
    return true;
  }

  if (!this->ParseGeneratorInstance(i, mf)) {
    return false;
  }

  if (!this->GeneratorInstanceVersion.empty()) {
    std::string const majorStr = VSVersionToMajorString(this->Version);
    cmsys::RegularExpression versionRegex(
      cmStrCat("^", majorStr, R"(\.[0-9]+\.[0-9]+\.[0-9]+$)"));
    if (!versionRegex.find(this->GeneratorInstanceVersion)) {
      std::ostringstream e;
      /* clang-format off */
      e <<
        "Generator\n"
        "  " << this->GetName() << "\n"
        "given instance specification\n"
        "  " << i << "\n"
        "but the version field is not 4 integer components"
        " starting in " << majorStr << "."
        ;
      /* clang-format on */
      mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return false;
    }
  }

  std::string vsInstance;
  if (!i.empty()) {
    vsInstance = i;
    if (!this->vsSetupAPIHelper.SetVSInstance(
          this->GeneratorInstance, this->GeneratorInstanceVersion)) {
      std::ostringstream e;
      /* clang-format off */
      e <<
        "Generator\n"
        "  " << this->GetName() << "\n"
        "could not find specified instance of Visual Studio:\n"
        "  " << i;
      /* clang-format on */
      if (!this->GeneratorInstance.empty() &&
          this->GeneratorInstanceVersion.empty() &&
          cmSystemTools::FileIsDirectory(this->GeneratorInstance)) {
        e << "\n"
             "The directory exists, but the instance is not known to the "
             "Visual Studio Installer, and no 'version=' field was given.";
      }
      mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return false;
    }
  } else if (!this->vsSetupAPIHelper.GetVSInstanceInfo(vsInstance)) {
    std::ostringstream e;
    /* clang-format off */
    e <<
      "Generator\n"
      "  " << this->GetName() << "\n"
      "could not find any instance of Visual Studio.\n";
    /* clang-format on */
    mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return false;
  }

  // Save the selected instance persistently.
  std::string genInstance = mf->GetSafeDefinition("CMAKE_GENERATOR_INSTANCE");
  if (vsInstance != genInstance) {
    this->CMakeInstance->AddCacheEntry("CMAKE_GENERATOR_INSTANCE", vsInstance,
                                       "Generator instance identifier.",
                                       cmStateEnums::INTERNAL);
  }

  this->SetVSVersionVar(mf);

  // The selected instance may have a different MSBuild than previously found.
  this->MSBuildCommandInitialized = false;

  this->LastGeneratorInstanceString = i;

  return true;
}

bool cmGlobalVisualStudioVersionedGenerator::ParseGeneratorInstance(
  std::string const& is, cmMakefile* mf)
{
  this->GeneratorInstance.clear();
  this->GeneratorInstanceVersion.clear();

  std::vector<std::string> const fields = cmTokenize(is, ",");
  auto fi = fields.begin();
  if (fi == fields.end()) {
    return true;
  }

  // The first field may be the VS instance.
  if (fi->find('=') == fi->npos) {
    this->GeneratorInstance = *fi;
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
        "given instance specification\n"
        "  " << is << "\n"
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
        "given instance specification\n"
        "  " << is << "\n"
        "that contains duplicate field key '" << key << "'."
        ;
      /* clang-format on */
      mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return false;
    }
    if (!this->ProcessGeneratorInstanceField(key, value)) {
      std::ostringstream e;
      /* clang-format off */
      e <<
        "Generator\n"
        "  " << this->GetName() << "\n"
        "given instance specification\n"
        "  " << is << "\n"
        "that contains invalid field '" << *fi << "'."
        ;
      /* clang-format on */
      mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return false;
    }
  }

  return true;
}

void cmGlobalVisualStudioVersionedGenerator::SetVSVersionVar(cmMakefile* mf)
{
  if (cm::optional<std::string> vsVer = this->GetVSInstanceVersion()) {
    mf->AddDefinition("CMAKE_VS_VERSION_BUILD_NUMBER", *vsVer);
  }
}

bool cmGlobalVisualStudioVersionedGenerator::ProcessGeneratorInstanceField(
  std::string const& key, std::string const& value)
{
  if (key == "version") {
    this->GeneratorInstanceVersion = value;
    return true;
  }
  return false;
}

bool cmGlobalVisualStudioVersionedGenerator::GetVSInstance(
  std::string& dir) const
{
  return vsSetupAPIHelper.GetVSInstanceInfo(dir);
}

cm::optional<std::string>
cmGlobalVisualStudioVersionedGenerator::GetVSInstanceVersion() const
{
  cm::optional<std::string> result;
  std::string vsInstanceVersion;
  if (vsSetupAPIHelper.GetVSInstanceVersion(vsInstanceVersion)) {
    result = vsInstanceVersion;
  }
  return result;
}

bool cmGlobalVisualStudioVersionedGenerator::IsStdOutEncodingSupported() const
{
  // Supported from Visual Studio 16.7 Preview 3.
  if (this->Version > cmGlobalVisualStudioGenerator::VSVersion::VS16) {
    return true;
  }
  if (this->Version < cmGlobalVisualStudioGenerator::VSVersion::VS16) {
    return false;
  }
  static std::string const vsVer16_7_P2 = "16.7.30128.36";
  cm::optional<std::string> vsVer = this->GetVSInstanceVersion();
  return (vsVer &&
          cmSystemTools::VersionCompareGreaterEq(*vsVer, vsVer16_7_P2));
}

bool cmGlobalVisualStudioVersionedGenerator::IsUtf8EncodingSupported() const
{
  // Supported from Visual Studio 16.10 Preview 2.
  if (this->Version > cmGlobalVisualStudioGenerator::VSVersion::VS16) {
    return true;
  }
  if (this->Version < cmGlobalVisualStudioGenerator::VSVersion::VS16) {
    return false;
  }
  static std::string const vsVer16_10_P2 = "16.10.31213.239";
  cm::optional<std::string> vsVer = this->GetVSInstanceVersion();
  return (vsVer &&
          cmSystemTools::VersionCompareGreaterEq(*vsVer, vsVer16_10_P2));
}

bool cmGlobalVisualStudioVersionedGenerator::IsScanDependenciesSupported()
  const
{
  // Supported from Visual Studio 17.6 Preview 7.
  if (this->Version > cmGlobalVisualStudioGenerator::VSVersion::VS17) {
    return true;
  }
  if (this->Version < cmGlobalVisualStudioGenerator::VSVersion::VS17) {
    return false;
  }
  static std::string const vsVer17_6_P7 = "17.6.33706.43";
  cm::optional<std::string> vsVer = this->GetVSInstanceVersion();
  return (vsVer &&
          cmSystemTools::VersionCompareGreaterEq(*vsVer, vsVer17_6_P7));
}

const char*
cmGlobalVisualStudioVersionedGenerator::GetAndroidApplicationTypeRevision()
  const
{
  switch (this->Version) {
    case cmGlobalVisualStudioGenerator::VSVersion::VS9:
    case cmGlobalVisualStudioGenerator::VSVersion::VS12:
      return "";
    case cmGlobalVisualStudioGenerator::VSVersion::VS14:
      return "2.0";
    case cmGlobalVisualStudioGenerator::VSVersion::VS15:
    case cmGlobalVisualStudioGenerator::VSVersion::VS16:
    case cmGlobalVisualStudioGenerator::VSVersion::VS17:
      return "3.0";
  }
  return "";
}

cmGlobalVisualStudioVersionedGenerator::AuxToolset
cmGlobalVisualStudioVersionedGenerator::FindAuxToolset(
  std::string& version, std::string& props) const
{
  if (version.empty()) {
    return AuxToolset::None;
  }

  std::string instancePath;
  this->GetVSInstance(instancePath);
  cmSystemTools::ConvertToUnixSlashes(instancePath);

  // Translate three-component format accepted by "vcvarsall -vcvars_ver=".
  cmsys::RegularExpression threeComponentRegex(
    "^([0-9]+\\.[0-9]+)\\.[0-9][0-9][0-9][0-9][0-9]$");
  // The two-component format represents the two major components of the
  // three-component format
  cmsys::RegularExpression twoComponentRegex("^([0-9]+\\.[0-9]+)$");
  if (threeComponentRegex.find(version)) {
    // Load "VC/Auxiliary/Build/*/Microsoft.VCToolsVersion.*.txt" files
    // with two matching components to check their three-component version.
    std::string const& twoComponent = threeComponentRegex.match(1);
    std::string pattern =
      cmStrCat(instancePath, "/VC/Auxiliary/Build/"_s, twoComponent,
               "*/Microsoft.VCToolsVersion."_s, twoComponent, "*.txt"_s);
    cmsys::Glob glob;
    glob.SetRecurseThroughSymlinks(false);
    if (glob.FindFiles(pattern)) {
      for (std::string const& txt : glob.GetFiles()) {
        std::string ver;
        cmsys::ifstream fin(txt.c_str());
        if (fin && std::getline(fin, ver)) {
          // Strip trailing whitespace.
          ver = ver.substr(0, ver.find_first_not_of("0123456789."));
          // If the three-component version matches, translate it to
          // that used by the "Microsoft.VCToolsVersion.*.txt" file name.
          if (ver == version) {
            cmsys::RegularExpression extractVersion(
              "VCToolsVersion\\.([0-9.]+)\\.txt$");
            if (extractVersion.find(txt)) {
              version = extractVersion.match(1);
              break;
            }
          }
        }
      }
    }
  } else if (twoComponentRegex.find(version)) {
    std::string const& twoComponent = twoComponentRegex.match(1);
    std::string pattern =
      cmStrCat(instancePath, "/VC/Auxiliary/Build/"_s, twoComponent,
               "*/Microsoft.VCToolsVersion."_s, twoComponent, "*.txt"_s);
    cmsys::Glob glob;
    glob.SetRecurseThroughSymlinks(false);
    if (glob.FindFiles(pattern) && !glob.GetFiles().empty()) {
      // Since we are only using the first two components of the
      // toolset version, we require a single match.
      if (glob.GetFiles().size() == 1) {
        std::string const& txt = glob.GetFiles()[0];
        std::string ver;
        cmsys::ifstream fin(txt.c_str());
        if (fin && std::getline(fin, ver)) {
          // Strip trailing whitespace.
          ver = ver.substr(0, ver.find_first_not_of("0123456789."));
          // We assume the version is correct, since it is the only one that
          // matched.
          cmsys::RegularExpression extractVersion(
            "VCToolsVersion\\.([0-9.]+)\\.txt$");
          if (extractVersion.find(txt)) {
            version = extractVersion.match(1);
          }
        }
      } else {
        props = cmStrCat(instancePath, "/VC/Auxiliary/Build/"_s);
        return AuxToolset::PropsIndeterminate;
      }
    }
  }

  if (cmSystemTools::VersionCompareGreaterEq(version, "14.20")) {
    props = cmStrCat(instancePath, "/VC/Auxiliary/Build."_s, version,
                     "/Microsoft.VCToolsVersion."_s, version, ".props"_s);
    if (cmSystemTools::PathExists(props)) {
      return AuxToolset::PropsExist;
    }
  }
  props = cmStrCat(instancePath, "/VC/Auxiliary/Build/"_s, version,
                   "/Microsoft.VCToolsVersion."_s, version, ".props"_s);
  if (cmSystemTools::PathExists(props)) {
    return AuxToolset::PropsExist;
  }

  // Accept the toolset version that is default in the current VS version
  // by matching the name later VS versions will use for the SxS props files.
  std::string vcToolsetVersion;
  if (this->vsSetupAPIHelper.GetVCToolsetVersion(vcToolsetVersion)) {
    // Accept an exact-match (three-component version).
    if (version == vcToolsetVersion) {
      return AuxToolset::Default;
    }

    // Accept known SxS props file names using four version components
    // in VS versions later than the current.
    if (version == "14.28.16.9" && vcToolsetVersion == "14.28.29910") {
      return AuxToolset::Default;
    }
    if (version == "14.29.16.10" && vcToolsetVersion == "14.29.30037") {
      return AuxToolset::Default;
    }
    if (version == "14.29.16.11" && vcToolsetVersion == "14.29.30133") {
      return AuxToolset::Default;
    }

    // The first two components of the default toolset version typically
    // match the name used by later VS versions for the SxS props files.
    cmsys::RegularExpression twoComponent("^([0-9]+\\.[0-9]+)");
    if (twoComponent.find(version)) {
      std::string const versionPrefix = cmStrCat(twoComponent.match(1), '.');
      if (cmHasPrefix(vcToolsetVersion, versionPrefix)) {
        return AuxToolset::Default;
      }
    }
  }

  return AuxToolset::PropsMissing;
}

bool cmGlobalVisualStudioVersionedGenerator::InitializePlatformWindows(
  cmMakefile* mf)
{
  // If the Win 8.1 SDK is installed then we can select a SDK matching
  // the target Windows version.
  if (this->IsWin81SDKInstalled()) {
    // VS 2019 does not default to 8.1 so specify it explicitly when needed.
    if (this->Version >= cmGlobalVisualStudioGenerator::VSVersion::VS16 &&
        !cmSystemTools::VersionCompareGreater(this->SystemVersion, "8.1")) {
      this->SetWindowsTargetPlatformVersion("8.1", mf);
      return this->VerifyNoGeneratorPlatformVersion(
        mf, "with the Windows 8.1 SDK installed");
    }
    return cmGlobalVisualStudio14Generator::InitializePlatformWindows(mf);
  }
  // Otherwise we must choose a Win 10 SDK even if we are not targeting
  // Windows 10.
  return this->SelectWindows10SDK(mf);
}

bool cmGlobalVisualStudioVersionedGenerator::SelectWindowsStoreToolset(
  std::string& toolset) const
{
  if (cmHasLiteralPrefix(this->SystemVersion, "10.0")) {
    if (this->IsWindowsStoreToolsetInstalled() &&
        this->IsWindowsDesktopToolsetInstalled()) {
      toolset = VSVersionToToolset(this->Version);
      return true;
    }
    return false;
  }
  return this->cmGlobalVisualStudio14Generator::SelectWindowsStoreToolset(
    toolset);
}

bool cmGlobalVisualStudioVersionedGenerator::IsWindowsDesktopToolsetInstalled()
  const
{
  return vsSetupAPIHelper.IsVSInstalled();
}

bool cmGlobalVisualStudioVersionedGenerator::IsWindowsStoreToolsetInstalled()
  const
{
  return vsSetupAPIHelper.IsWin10SDKInstalled();
}

bool cmGlobalVisualStudioVersionedGenerator::IsWin81SDKInstalled() const
{
  // Does the VS installer tool know about one?
  if (vsSetupAPIHelper.IsWin81SDKInstalled()) {
    return true;
  }

  // Does the registry know about one (e.g. from VS 2015)?
  std::string win81Root;
  if (cmSystemTools::ReadRegistryValue(
        "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\"
        "Windows Kits\\Installed Roots;KitsRoot81",
        win81Root, cmSystemTools::KeyWOW64_32) ||
      cmSystemTools::ReadRegistryValue(
        "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\"
        "Windows Kits\\Installed Roots;KitsRoot81",
        win81Root, cmSystemTools::KeyWOW64_32)) {
    return cmSystemTools::FileExists(win81Root + "/include/um/windows.h",
                                     true);
  }
  return false;
}

std::string
cmGlobalVisualStudioVersionedGenerator::GetWindows10SDKMaxVersionDefault(
  cmMakefile*) const
{
  return std::string();
}

cm::optional<std::string>
cmGlobalVisualStudioVersionedGenerator::FindMSBuildCommandEarly(cmMakefile* mf)
{
  std::string instance = mf->GetSafeDefinition("CMAKE_GENERATOR_INSTANCE");
  if (!this->SetGeneratorInstance(instance, mf)) {
    cmSystemTools::SetFatalErrorOccurred();
    return {};
  }
  return this->cmGlobalVisualStudio14Generator::FindMSBuildCommandEarly(mf);
}

std::string cmGlobalVisualStudioVersionedGenerator::FindMSBuildCommand()
{
  std::string msbuild;

  // Ask Visual Studio Installer tool.
  std::string vs;
  if (vsSetupAPIHelper.GetVSInstanceInfo(vs)) {
    if (this->Version >= cmGlobalVisualStudioGenerator::VSVersion::VS17) {
      if (VSIsArm64Host()) {
        if (VSHasDotNETFrameworkArm64()) {
          msbuild = vs + "/MSBuild/Current/Bin/arm64/MSBuild.exe";
          if (cmSystemTools::FileExists(msbuild)) {
            return msbuild;
          }
        }
        if (VSIsWindows11OrGreater()) {
          msbuild = vs + "/MSBuild/Current/Bin/amd64/MSBuild.exe";
          if (cmSystemTools::FileExists(msbuild)) {
            return msbuild;
          }
        }
      } else {
        msbuild = vs + "/MSBuild/Current/Bin/amd64/MSBuild.exe";
        if (cmSystemTools::FileExists(msbuild)) {
          return msbuild;
        }
      }
    }
    msbuild = vs + "/MSBuild/Current/Bin/MSBuild.exe";
    if (cmSystemTools::FileExists(msbuild)) {
      return msbuild;
    }
    msbuild = vs + "/MSBuild/15.0/Bin/MSBuild.exe";
    if (cmSystemTools::FileExists(msbuild)) {
      return msbuild;
    }
  }

  msbuild = "MSBuild.exe";
  return msbuild;
}

std::string cmGlobalVisualStudioVersionedGenerator::FindDevEnvCommand()
{
  std::string devenv;

  // Ask Visual Studio Installer tool.
  std::string vs;
  if (vsSetupAPIHelper.GetVSInstanceInfo(vs)) {
    devenv = vs + "/Common7/IDE/devenv.com";
    if (cmSystemTools::FileExists(devenv)) {
      return devenv;
    }
  }

  devenv = "devenv.com";
  return devenv;
}
