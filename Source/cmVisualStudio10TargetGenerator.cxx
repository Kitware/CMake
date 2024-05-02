/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmVisualStudio10TargetGenerator.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <set>
#include <sstream>

#include <cm/memory>
#include <cm/optional>
#include <cm/string_view>
#include <cm/vector>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "windows.h"
// include wincrypt.h after windows.h
#include <wincrypt.h>

#include "cmsys/FStream.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cmComputeLinkInformation.h"
#include "cmCryptoHash.h"
#include "cmCustomCommand.h"
#include "cmCustomCommandGenerator.h"
#include "cmFileSet.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmGlobalVisualStudio10Generator.h"
#include "cmGlobalVisualStudio7Generator.h"
#include "cmGlobalVisualStudioGenerator.h"
#include "cmLinkLineDeviceComputer.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmLocalVisualStudio10Generator.h"
#include "cmLocalVisualStudio7Generator.h"
#include "cmLocalVisualStudioGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPropertyMap.h"
#include "cmSourceFile.h"
#include "cmSourceFileLocation.h"
#include "cmSourceFileLocationKind.h"
#include "cmSourceGroup.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmValue.h"
#include "cmVisualStudioGeneratorOptions.h"

struct cmIDEFlagTable;

static void ConvertToWindowsSlash(std::string& s);

static std::string cmVS10EscapeXML(std::string arg)
{
  cmSystemTools::ReplaceString(arg, "&", "&amp;");
  cmSystemTools::ReplaceString(arg, "<", "&lt;");
  cmSystemTools::ReplaceString(arg, ">", "&gt;");
  return arg;
}

static std::string cmVS10EscapeAttr(std::string arg)
{
  cmSystemTools::ReplaceString(arg, "&", "&amp;");
  cmSystemTools::ReplaceString(arg, "<", "&lt;");
  cmSystemTools::ReplaceString(arg, ">", "&gt;");
  cmSystemTools::ReplaceString(arg, "\"", "&quot;");
  cmSystemTools::ReplaceString(arg, "\n", "&#10;");
  return arg;
}

struct cmVisualStudio10TargetGenerator::Elem
{
  std::ostream& S;
  const int Indent;
  bool HasElements = false;
  bool HasContent = false;
  std::string Tag;

  Elem(std::ostream& s, std::string tag)
    : S(s)
    , Indent(0)
    , Tag(std::move(tag))
  {
    this->StartElement();
  }
  Elem(const Elem&) = delete;
  Elem(Elem& par, cm::string_view tag)
    : S(par.S)
    , Indent(par.Indent + 1)
    , Tag(std::string(tag))
  {
    par.SetHasElements();
    this->StartElement();
  }
  void SetHasElements()
  {
    if (!HasElements) {
      this->S << '>';
      HasElements = true;
    }
  }
  std::ostream& WriteString(const char* line);
  void StartElement() { this->WriteString("<") << this->Tag; }
  void Element(cm::string_view tag, std::string val)
  {
    Elem(*this, tag).Content(std::move(val));
  }
  Elem& Attribute(const char* an, std::string av)
  {
    this->S << ' ' << an << "=\"" << cmVS10EscapeAttr(std::move(av)) << '"';
    return *this;
  }
  void Content(std::string val)
  {
    if (!this->HasContent) {
      this->S << '>';
      this->HasContent = true;
    }
    this->S << cmVS10EscapeXML(std::move(val));
  }
  ~Elem()
  {
    // Do not emit element which has not been started
    if (Tag.empty()) {
      return;
    }

    if (HasElements) {
      this->WriteString("</") << this->Tag << '>';
    } else if (HasContent) {
      this->S << "</" << this->Tag << '>';
    } else {
      this->S << " />";
    }
  }

  void WritePlatformConfigTag(const std::string& tag, const std::string& cond,
                              const std::string& content);
};

class cmVS10GeneratorOptions : public cmVisualStudioGeneratorOptions
{
public:
  using Elem = cmVisualStudio10TargetGenerator::Elem;
  cmVS10GeneratorOptions(cmLocalVisualStudioGenerator* lg, Tool tool,
                         cmVS7FlagTable const* table,
                         cmVisualStudio10TargetGenerator* g = nullptr)
    : cmVisualStudioGeneratorOptions(lg, tool, table)
    , TargetGenerator(g)
  {
  }

  void OutputFlag(std::ostream& /*fout*/, int /*indent*/,
                  const std::string& tag, const std::string& content) override
  {
    if (!this->GetConfiguration().empty()) {
      // if there are configuration specific flags, then
      // use the configuration specific tag for PreprocessorDefinitions
      const std::string cond =
        this->TargetGenerator->CalcCondition(this->GetConfiguration());
      this->Parent->WritePlatformConfigTag(tag, cond, content);
    } else {
      this->Parent->Element(tag, content);
    }
  }

private:
  cmVisualStudio10TargetGenerator* const TargetGenerator;
  Elem* Parent = nullptr;
  friend cmVisualStudio10TargetGenerator::OptionsHelper;
};

struct cmVisualStudio10TargetGenerator::OptionsHelper
{
  cmVS10GeneratorOptions& O;
  OptionsHelper(cmVS10GeneratorOptions& o, Elem& e)
    : O(o)
  {
    O.Parent = &e;
  }
  ~OptionsHelper() { O.Parent = nullptr; }

  void OutputPreprocessorDefinitions(const std::string& lang)
  {
    O.OutputPreprocessorDefinitions(O.Parent->S, O.Parent->Indent + 1, lang);
  }
  void OutputAdditionalIncludeDirectories(const std::string& lang)
  {
    O.OutputAdditionalIncludeDirectories(O.Parent->S, O.Parent->Indent + 1,
                                         lang);
  }
  void OutputFlagMap() { O.OutputFlagMap(O.Parent->S, O.Parent->Indent + 1); }
  void PrependInheritedString(std::string const& key)
  {
    O.PrependInheritedString(key);
  }
};

static std::string cmVS10EscapeComment(std::string const& comment)
{
  // MSBuild takes the CDATA of a <Message></Message> element and just
  // does "echo $CDATA" with no escapes.  We must encode the string.
  // http://technet.microsoft.com/en-us/library/cc772462%28WS.10%29.aspx
  std::string echoable;
  for (char c : comment) {
    switch (c) {
      case '\r':
        break;
      case '\n':
        echoable += '\t';
        break;
      case '"': /* no break */
      case '|': /* no break */
      case '&': /* no break */
      case '<': /* no break */
      case '>': /* no break */
      case '^':
        echoable += '^'; /* no break */
        CM_FALLTHROUGH;
      default:
        echoable += c;
        break;
    }
  }
  return echoable;
}

static bool cmVS10IsTargetsFile(std::string const& path)
{
  std::string const ext = cmSystemTools::GetFilenameLastExtension(path);
  return cmSystemTools::Strucmp(ext.c_str(), ".targets") == 0;
}

static VsProjectType computeProjectType(cmGeneratorTarget const* t)
{
  if (t->IsCSharpOnly()) {
    return VsProjectType::csproj;
  }
  return VsProjectType::vcxproj;
}

static std::string computeProjectFileExtension(VsProjectType projectType)
{
  switch (projectType) {
    case VsProjectType::csproj:
      return ".csproj";
    case VsProjectType::proj:
      return ".proj";
    default:
      return ".vcxproj";
  }
}

static std::string computeProjectFileExtension(cmGeneratorTarget const* t)
{
  return computeProjectFileExtension(computeProjectType(t));
}

cmVisualStudio10TargetGenerator::cmVisualStudio10TargetGenerator(
  cmGeneratorTarget* target, cmGlobalVisualStudio10Generator* gg)
  : GeneratorTarget(target)
  , Makefile(target->Target->GetMakefile())
  , Platform(gg->GetPlatformName())
  , Name(target->GetName())
  , GUID(gg->GetGUID(this->Name))
  , GlobalGenerator(gg)
  , LocalGenerator(
      (cmLocalVisualStudio10Generator*)target->GetLocalGenerator())
{
  this->Configurations =
    this->Makefile->GetGeneratorConfigs(cmMakefile::ExcludeEmptyConfig);
  this->NsightTegra = gg->IsNsightTegra();
  this->Android = gg->TargetsAndroid();
  auto scanProp = target->GetProperty("CXX_SCAN_FOR_MODULES");
  for (auto const& config : this->Configurations) {
    if (scanProp.IsSet()) {
      this->ScanSourceForModuleDependencies[config] = scanProp.IsOn();
    } else {
      this->ScanSourceForModuleDependencies[config] =
        target->NeedCxxDyndep(config) ==
        cmGeneratorTarget::CxxModuleSupport::Enabled;
    }
  }
  for (unsigned int& version : this->NsightTegraVersion) {
    version = 0;
  }
  sscanf(gg->GetNsightTegraVersion().c_str(), "%u.%u.%u.%u",
         &this->NsightTegraVersion[0], &this->NsightTegraVersion[1],
         &this->NsightTegraVersion[2], &this->NsightTegraVersion[3]);
  this->MSTools = !this->NsightTegra && !this->Android;
  this->Managed = false;
  this->TargetCompileAsWinRT = false;
  this->IsMissingFiles = false;
  this->DefaultArtifactDir =
    cmStrCat(this->LocalGenerator->GetCurrentBinaryDirectory(), '/',
             this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget));
  this->InSourceBuild = (this->Makefile->GetCurrentSourceDirectory() ==
                         this->Makefile->GetCurrentBinaryDirectory());
  this->ClassifyAllConfigSources();
}

cmVisualStudio10TargetGenerator::~cmVisualStudio10TargetGenerator() = default;

std::string cmVisualStudio10TargetGenerator::CalcCondition(
  const std::string& config) const
{
  std::ostringstream oss;
  oss << "'$(Configuration)|$(Platform)'=='" << config << '|' << this->Platform
      << '\'';
  // handle special case for 32 bit C# targets
  if (this->ProjectType == VsProjectType::csproj &&
      this->Platform == "Win32"_s) {
    oss << " Or "
           "'$(Configuration)|$(Platform)'=='"
        << config
        << "|x86"
           "'";
  }
  return oss.str();
}

void cmVisualStudio10TargetGenerator::Elem::WritePlatformConfigTag(
  const std::string& tag, const std::string& cond, const std::string& content)
{
  Elem(*this, tag).Attribute("Condition", cond).Content(content);
}

std::ostream& cmVisualStudio10TargetGenerator::Elem::WriteString(
  const char* line)
{
  this->S << '\n';
  this->S.fill(' ');
  this->S.width(this->Indent * 2);
  // write an empty string to get the fill level indent to print
  this->S << "";
  this->S << line;
  return this->S;
}

#define VS10_CXX_DEFAULT_PROPS "$(VCTargetsPath)\\Microsoft.Cpp.Default.props"
#define VS10_CXX_PROPS "$(VCTargetsPath)\\Microsoft.Cpp.props"
#define VS10_CXX_USER_PROPS                                                   \
  "$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props"
#define VS10_CXX_TARGETS "$(VCTargetsPath)\\Microsoft.Cpp.targets"

#define VS10_CSharp_DEFAULT_PROPS                                             \
  "$(MSBuildExtensionsPath)\\$(MSBuildToolsVersion)\\Microsoft.Common.props"
// This does not seem to exist by default, it's just provided for consistency
// in case users want to have default custom props for C# targets
#define VS10_CSharp_USER_PROPS                                                \
  "$(UserRootDir)\\Microsoft.CSharp.$(Platform).user.props"
#define VS10_CSharp_TARGETS "$(MSBuildToolsPath)\\Microsoft.CSharp.targets"

#define VS10_CSharp_NETCF_TARGETS                                             \
  "$(MSBuildExtensionsPath)\\Microsoft\\$(TargetFrameworkIdentifier)\\"       \
  "$(TargetFrameworkTargetsVersion)\\Microsoft.$(TargetFrameworkIdentifier)"  \
  ".CSharp.targets"

void cmVisualStudio10TargetGenerator::Generate()
{
  if (this->GeneratorTarget->IsSynthetic()) {
    this->GeneratorTarget->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Target \"", this->GeneratorTarget->GetName(),
               "\" contains C++ modules intended for BMI-only compilation. "
               "This is not yet supported by the Visual Studio generator."));
    return;
  }

  for (std::string const& config : this->Configurations) {
    this->GeneratorTarget->CheckCxxModuleStatus(config);
  }

  this->ProjectType = computeProjectType(this->GeneratorTarget);
  this->Managed = this->ProjectType == VsProjectType::csproj;
  const std::string ProjectFileExtension =
    computeProjectFileExtension(this->ProjectType);

  if (this->ProjectType == VsProjectType::csproj &&
      this->GeneratorTarget->GetType() == cmStateEnums::STATIC_LIBRARY) {
    std::string message =
      cmStrCat("The C# target \"", this->GeneratorTarget->GetName(),
               "\" is of type STATIC_LIBRARY. This is discouraged (and may be "
               "disabled in future). Make it a SHARED library instead.");
    this->Makefile->IssueMessage(MessageType::DEPRECATION_WARNING, message);
  }

  if (this->Android &&
      this->GeneratorTarget->GetType() == cmStateEnums::EXECUTABLE &&
      !this->GeneratorTarget->Target->IsAndroidGuiExecutable()) {
    this->GlobalGenerator->AddAndroidExecutableWarning(this->Name);
  }

  // Tell the global generator the name of the project file
  this->GeneratorTarget->Target->SetProperty("GENERATOR_FILE_NAME",
                                             this->Name);
  this->GeneratorTarget->Target->SetProperty("GENERATOR_FILE_NAME_EXT",
                                             ProjectFileExtension);
  this->DotNetHintReferences.clear();
  this->AdditionalUsingDirectories.clear();
  if (this->GeneratorTarget->GetType() <= cmStateEnums::OBJECT_LIBRARY) {
    if (!this->ComputeClOptions()) {
      return;
    }
    if (!this->ComputeRcOptions()) {
      return;
    }
    if (!this->ComputeCudaOptions()) {
      return;
    }
    if (!this->ComputeCudaLinkOptions()) {
      return;
    }
    if (!this->ComputeMarmasmOptions()) {
      return;
    }
    if (!this->ComputeMasmOptions()) {
      return;
    }
    if (!this->ComputeNasmOptions()) {
      return;
    }
    if (!this->ComputeLinkOptions()) {
      return;
    }
    if (!this->ComputeLibOptions()) {
      return;
    }
  }
  std::string path =
    cmStrCat(this->LocalGenerator->GetCurrentBinaryDirectory(), '/',
             this->Name, ProjectFileExtension);
  cmGeneratedFileStream BuildFileStream(path);
  const std::string& PathToProjectFile = path;
  BuildFileStream.SetCopyIfDifferent(true);

  // Write the encoding header into the file
  char magic[] = { char(0xEF), char(0xBB), char(0xBF) };
  BuildFileStream.write(magic, 3);

  if (this->ProjectType == VsProjectType::csproj &&
      this->GeneratorTarget->IsDotNetSdkTarget() &&
      this->GlobalGenerator->GetVersion() >=
        cmGlobalVisualStudioGenerator::VSVersion::VS16) {
    this->WriteSdkStyleProjectFile(BuildFileStream);
  } else {
    this->WriteClassicMsBuildProjectFile(BuildFileStream);
  }

  if (BuildFileStream.Close()) {
    this->GlobalGenerator->FileReplacedDuringGenerate(PathToProjectFile);
  }

  // The groups are stored in a separate file for VS 10
  this->WriteGroups();

  // Update cache with project-specific entries.
  this->UpdateCache();
}

void cmVisualStudio10TargetGenerator::WriteClassicMsBuildProjectFile(
  cmGeneratedFileStream& BuildFileStream)
{
  BuildFileStream << R"(<?xml version="1.0" encoding=")"
                  << this->GlobalGenerator->Encoding() << "\"?>";
  {
    Elem e0(BuildFileStream, "Project");
    e0.Attribute("DefaultTargets", "Build");
    const char* toolsVersion = this->GlobalGenerator->GetToolsVersion();
    if (this->GlobalGenerator->GetVersion() ==
          cmGlobalVisualStudioGenerator::VSVersion::VS12 &&
        this->GlobalGenerator->TargetsWindowsCE()) {
      toolsVersion = "4.0";
    }
    e0.Attribute("ToolsVersion", toolsVersion);
    e0.Attribute("xmlns",
                 "http://schemas.microsoft.com/developer/msbuild/2003");

    if (this->NsightTegra) {
      Elem e1(e0, "PropertyGroup");
      e1.Attribute("Label", "NsightTegraProject");
      const unsigned int nsightTegraMajorVersion = this->NsightTegraVersion[0];
      const unsigned int nsightTegraMinorVersion = this->NsightTegraVersion[1];
      if (nsightTegraMajorVersion >= 2) {
        if (nsightTegraMajorVersion > 3 ||
            (nsightTegraMajorVersion == 3 && nsightTegraMinorVersion >= 1)) {
          e1.Element("NsightTegraProjectRevisionNumber", "11");
        } else {
          // Nsight Tegra 2.0 uses project revision 9.
          e1.Element("NsightTegraProjectRevisionNumber", "9");
        }
        // Tell newer versions to upgrade silently when loading.
        e1.Element("NsightTegraUpgradeOnceWithoutPrompt", "true");
      } else {
        // Require Nsight Tegra 1.6 for JCompile support.
        e1.Element("NsightTegraProjectRevisionNumber", "7");
      }
    }

    if (const char* hostArch =
          this->GlobalGenerator->GetPlatformToolsetHostArchitecture()) {
      Elem e1(e0, "PropertyGroup");
      e1.Element("PreferredToolArchitecture", hostArch);
    }

    // The ALL_BUILD, PACKAGE, and ZERO_CHECK projects transitively include
    // Microsoft.Common.CurrentVersion.targets which triggers Target
    // ResolveNugetPackageAssets when SDK-style targets are in the project.
    // However, these projects have no nuget packages to reference and the
    // build fails.
    // Setting ResolveNugetPackages to false skips this target and the build
    // succeeds.
    cm::string_view targetName{ this->GeneratorTarget->GetName() };
    if (targetName == "ALL_BUILD"_s || targetName == "PACKAGE"_s ||
        targetName == CMAKE_CHECK_BUILD_SYSTEM_TARGET) {
      Elem e1(e0, "PropertyGroup");
      e1.Element("ResolveNugetPackages", "false");
    }

    if (this->ProjectType != VsProjectType::csproj) {
      this->WriteProjectConfigurations(e0);
    }

    {
      Elem e1(e0, "PropertyGroup");
      this->WriteCommonPropertyGroupGlobals(e1);

      if ((this->MSTools || this->Android) &&
          this->GeneratorTarget->IsInBuildSystem()) {
        this->WriteApplicationTypeSettings(e1);
        this->VerifyNecessaryFiles();
      }

      cmValue vsProjectName =
        this->GeneratorTarget->GetProperty("VS_SCC_PROJECTNAME");
      cmValue vsLocalPath =
        this->GeneratorTarget->GetProperty("VS_SCC_LOCALPATH");
      cmValue vsProvider =
        this->GeneratorTarget->GetProperty("VS_SCC_PROVIDER");

      if (vsProjectName && vsLocalPath && vsProvider) {
        e1.Element("SccProjectName", *vsProjectName);
        e1.Element("SccLocalPath", *vsLocalPath);
        e1.Element("SccProvider", *vsProvider);

        cmValue vsAuxPath =
          this->GeneratorTarget->GetProperty("VS_SCC_AUXPATH");
        if (vsAuxPath) {
          e1.Element("SccAuxPath", *vsAuxPath);
        }
      }

      if (this->GeneratorTarget->GetPropertyAsBool("VS_WINRT_COMPONENT")) {
        e1.Element("WinMDAssembly", "true");
      }

      e1.Element("Platform", this->Platform);
      cmValue projLabel = this->GeneratorTarget->GetProperty("PROJECT_LABEL");
      e1.Element("ProjectName", projLabel ? *projLabel : this->Name);
      {
        cm::optional<std::string> targetFramework;
        cm::optional<std::string> targetFrameworkVersion;
        cm::optional<std::string> targetFrameworkIdentifier;
        cm::optional<std::string> targetFrameworkTargetsVersion;
        if (cmValue tf =
              this->GeneratorTarget->GetProperty("DOTNET_TARGET_FRAMEWORK")) {
          targetFramework = *tf;
        } else if (cmValue vstfVer = this->GeneratorTarget->GetProperty(
                     "VS_DOTNET_TARGET_FRAMEWORK_VERSION")) {
          // FIXME: Someday, add a deprecation warning for VS_* property.
          targetFrameworkVersion = *vstfVer;
        } else if (cmValue tfVer = this->GeneratorTarget->GetProperty(
                     "DOTNET_TARGET_FRAMEWORK_VERSION")) {
          targetFrameworkVersion = *tfVer;
        } else if (this->ProjectType == VsProjectType::csproj) {
          targetFrameworkVersion =
            this->GlobalGenerator->GetTargetFrameworkVersion();
        }
        if (this->ProjectType == VsProjectType::vcxproj &&
            this->GlobalGenerator->TargetsWindowsCE()) {
          e1.Element("EnableRedirectPlatform", "true");
          e1.Element("RedirectPlatformValue", this->Platform);
        }
        if (this->ProjectType == VsProjectType::csproj) {
          if (this->GlobalGenerator->TargetsWindowsCE()) {
            // FIXME: These target VS_TARGET_FRAMEWORK* target properties
            // are undocumented settings only ever supported for WinCE.
            // We need a better way to control these in general.
            if (cmValue tfId = this->GeneratorTarget->GetProperty(
                  "VS_TARGET_FRAMEWORK_IDENTIFIER")) {
              targetFrameworkIdentifier = *tfId;
            }
            if (cmValue tfTargetsVer = this->GeneratorTarget->GetProperty(
                  "VS_TARGET_FRAMEWORKS_TARGET_VERSION")) {
              targetFrameworkTargetsVersion = *tfTargetsVer;
            }
          }
          if (!targetFrameworkIdentifier) {
            targetFrameworkIdentifier =
              this->GlobalGenerator->GetTargetFrameworkIdentifier();
          }
          if (!targetFrameworkTargetsVersion) {
            targetFrameworkTargetsVersion =
              this->GlobalGenerator->GetTargetFrameworkTargetsVersion();
          }
        }
        if (targetFramework) {
          if (targetFramework->find(';') != std::string::npos) {
            e1.Element("TargetFrameworks", *targetFramework);
          } else {
            e1.Element("TargetFramework", *targetFramework);
          }
        }
        if (targetFrameworkVersion) {
          e1.Element("TargetFrameworkVersion", *targetFrameworkVersion);
        }
        if (targetFrameworkIdentifier) {
          e1.Element("TargetFrameworkIdentifier", *targetFrameworkIdentifier);
        }
        if (targetFrameworkTargetsVersion) {
          e1.Element("TargetFrameworkTargetsVersion",
                     *targetFrameworkTargetsVersion);
        }
        if (!this->GlobalGenerator->GetPlatformToolsetCudaCustomDirString()
               .empty()) {
          e1.Element(
            "CudaToolkitCustomDir",
            cmStrCat(
              this->GlobalGenerator->GetPlatformToolsetCudaCustomDirString(),
              this->GlobalGenerator
                ->GetPlatformToolsetCudaNvccSubdirString()));
        }
      }

      // Disable the project upgrade prompt that is displayed the first time a
      // project using an older toolset version is opened in a newer version of
      // the IDE (respected by VS 2013 and above).
      if (this->GlobalGenerator->GetVersion() >=
          cmGlobalVisualStudioGenerator::VSVersion::VS12) {
        e1.Element("VCProjectUpgraderObjectName", "NoUpgrade");
      }

      if (const char* vcTargetsPath =
            this->GlobalGenerator->GetCustomVCTargetsPath()) {
        e1.Element("VCTargetsPath", vcTargetsPath);
      }

      if (this->Managed) {
        if (this->LocalGenerator->GetVersion() >=
            cmGlobalVisualStudioGenerator::VSVersion::VS17) {
          e1.Element("ManagedAssembly", "true");
        }
        std::string outputType;
        switch (this->GeneratorTarget->GetType()) {
          case cmStateEnums::OBJECT_LIBRARY:
          case cmStateEnums::STATIC_LIBRARY:
          case cmStateEnums::SHARED_LIBRARY:
            outputType = "Library";
            break;
          case cmStateEnums::MODULE_LIBRARY:
            outputType = "Module";
            break;
          case cmStateEnums::EXECUTABLE: {
            auto const win32 =
              this->GeneratorTarget->GetSafeProperty("WIN32_EXECUTABLE");
            if (win32.find("$<") != std::string::npos) {
              this->Makefile->IssueMessage(
                MessageType::FATAL_ERROR,
                cmStrCat(
                  "Target \"", this->GeneratorTarget->GetName(),
                  "\" has a generator expression in its WIN32_EXECUTABLE "
                  "property. This is not supported on managed executables."));
              return;
            }
            if (cmIsOn(win32)) {
              outputType = "WinExe";
            } else {
              outputType = "Exe";
            }
          } break;
          case cmStateEnums::UTILITY:
          case cmStateEnums::INTERFACE_LIBRARY:
          case cmStateEnums::GLOBAL_TARGET:
            outputType = "Utility";
            break;
          case cmStateEnums::UNKNOWN_LIBRARY:
            break;
        }
        e1.Element("OutputType", outputType);
        e1.Element("AppDesignerFolder", "Properties");
      }
    }

    cmValue startupObject =
      this->GeneratorTarget->GetProperty("VS_DOTNET_STARTUP_OBJECT");

    if (startupObject && this->Managed) {
      Elem e1(e0, "PropertyGroup");
      e1.Element("StartupObject", *startupObject);
    }

    switch (this->ProjectType) {
      case VsProjectType::vcxproj: {
        Elem(e0, "Import").Attribute("Project", VS10_CXX_DEFAULT_PROPS);
        std::string const& props =
          this->GlobalGenerator->GetPlatformToolsetVersionProps();
        if (!props.empty()) {
          Elem(e0, "Import").Attribute("Project", props);
        }
      } break;
      case VsProjectType::csproj:
        Elem(e0, "Import")
          .Attribute("Project", VS10_CSharp_DEFAULT_PROPS)
          .Attribute("Condition", "Exists('" VS10_CSharp_DEFAULT_PROPS "')");
        break;
      default:
        break;
    }

    this->WriteProjectConfigurationValues(e0);

    if (this->ProjectType == VsProjectType::vcxproj) {
      Elem(e0, "Import").Attribute("Project", VS10_CXX_PROPS);
    }
    {
      Elem e1(e0, "ImportGroup");
      e1.Attribute("Label", "ExtensionSettings");
      e1.SetHasElements();

      if (this->GlobalGenerator->IsCudaEnabled()) {
        auto customDir =
          this->GlobalGenerator->GetPlatformToolsetCudaCustomDirString();
        std::string cudaPath = customDir.empty()
          ? "$(VCTargetsPath)\\BuildCustomizations\\"
          : cmStrCat(customDir,
                     this->GlobalGenerator
                       ->GetPlatformToolsetCudaVSIntegrationSubdirString(),
                     R"(extras\visual_studio_integration\MSBuildExtensions\)");
        Elem(e1, "Import")
          .Attribute("Project",
                     cmStrCat(std::move(cudaPath), "CUDA ",
                              this->GlobalGenerator->GetPlatformToolsetCuda(),
                              ".props"));
      }
      if (this->GlobalGenerator->IsMarmasmEnabled()) {
        Elem(e1, "Import")
          .Attribute("Project",
                     "$(VCTargetsPath)\\BuildCustomizations\\marmasm.props");
      }
      if (this->GlobalGenerator->IsMasmEnabled()) {
        Elem(e1, "Import")
          .Attribute("Project",
                     "$(VCTargetsPath)\\BuildCustomizations\\masm.props");
      }
      if (this->GlobalGenerator->IsNasmEnabled()) {
        // Always search in the standard modules location.
        std::string propsTemplate =
          GetCMakeFilePath("Templates/MSBuild/nasm.props.in");

        std::string propsLocal =
          cmStrCat(this->DefaultArtifactDir, "\\nasm.props");
        ConvertToWindowsSlash(propsLocal);
        this->Makefile->ConfigureFile(propsTemplate, propsLocal, false, true,
                                      true);
        Elem(e1, "Import").Attribute("Project", propsLocal);
      }
    }
    {
      Elem e1(e0, "ImportGroup");
      e1.Attribute("Label", "PropertySheets");
      std::string props;
      switch (this->ProjectType) {
        case VsProjectType::vcxproj:
          props = VS10_CXX_USER_PROPS;
          break;
        case VsProjectType::csproj:
          props = VS10_CSharp_USER_PROPS;
          break;
        default:
          break;
      }
      if (cmValue p = this->GeneratorTarget->GetProperty("VS_USER_PROPS")) {
        props = *p;
      }
      if (!props.empty()) {
        ConvertToWindowsSlash(props);
        Elem(e1, "Import")
          .Attribute("Project", props)
          .Attribute("Condition", cmStrCat("exists('", props, "')"))
          .Attribute("Label", "LocalAppDataPlatform");
      }

      this->WritePlatformExtensions(e1);
    }

    this->WriteDotNetDocumentationFile(e0);
    Elem(e0, "PropertyGroup").Attribute("Label", "UserMacros");
    this->WriteWinRTPackageCertificateKeyFile(e0);
    this->WritePathAndIncrementalLinkOptions(e0);
    this->WritePublicProjectContentOptions(e0);
    this->WriteCEDebugProjectConfigurationValues(e0);
    this->WriteItemDefinitionGroups(e0);
    this->WriteCustomCommands(e0);
    this->WriteAllSources(e0);
    this->WriteDotNetReferences(e0);
    this->WritePackageReferences(e0);
    this->WriteImports(e0);
    this->WriteEmbeddedResourceGroup(e0);
    this->WriteXamlFilesGroup(e0);
    this->WriteWinRTReferences(e0);
    this->WriteProjectReferences(e0);
    this->WriteSDKReferences(e0);
    switch (this->ProjectType) {
      case VsProjectType::vcxproj:
        Elem(e0, "Import").Attribute("Project", VS10_CXX_TARGETS);
        break;
      case VsProjectType::csproj:
        if (this->GlobalGenerator->TargetsWindowsCE()) {
          Elem(e0, "Import").Attribute("Project", VS10_CSharp_NETCF_TARGETS);
        } else {
          Elem(e0, "Import").Attribute("Project", VS10_CSharp_TARGETS);
        }
        break;
      default:
        break;
    }

    this->WriteTargetSpecificReferences(e0);
    {
      Elem e1(e0, "ImportGroup");
      e1.Attribute("Label", "ExtensionTargets");
      e1.SetHasElements();
      this->WriteTargetsFileReferences(e1);
      if (this->GlobalGenerator->IsCudaEnabled()) {
        auto customDir =
          this->GlobalGenerator->GetPlatformToolsetCudaCustomDirString();
        std::string cudaPath = customDir.empty()
          ? "$(VCTargetsPath)\\BuildCustomizations\\"
          : cmStrCat(customDir,
                     this->GlobalGenerator
                       ->GetPlatformToolsetCudaVSIntegrationSubdirString(),
                     R"(extras\visual_studio_integration\MSBuildExtensions\)");
        Elem(e1, "Import")
          .Attribute("Project",
                     cmStrCat(std::move(cudaPath), "CUDA ",
                              this->GlobalGenerator->GetPlatformToolsetCuda(),
                              ".targets"));
      }
      if (this->GlobalGenerator->IsMarmasmEnabled()) {
        Elem(e1, "Import")
          .Attribute("Project",
                     "$(VCTargetsPath)\\BuildCustomizations\\marmasm.targets");
      }
      if (this->GlobalGenerator->IsMasmEnabled()) {
        Elem(e1, "Import")
          .Attribute("Project",
                     "$(VCTargetsPath)\\BuildCustomizations\\masm.targets");
      }
      if (this->GlobalGenerator->IsNasmEnabled()) {
        std::string nasmTargets =
          GetCMakeFilePath("Templates/MSBuild/nasm.targets");
        Elem(e1, "Import").Attribute("Project", nasmTargets);
      }
    }
    if (this->ProjectType == VsProjectType::vcxproj &&
        this->HaveCustomCommandDepfile) {
      std::string depfileTargets =
        GetCMakeFilePath("Templates/MSBuild/CustomBuildDepFile.targets");
      Elem(e0, "Import").Attribute("Project", depfileTargets);
    }
    if (this->ProjectType == VsProjectType::csproj) {
      for (std::string const& c : this->Configurations) {
        Elem e1(e0, "PropertyGroup");
        e1.Attribute("Condition",
                     cmStrCat("'$(Configuration)' == '", c, '\''));
        e1.SetHasElements();
        this->WriteEvents(e1, c);
      }
      // make sure custom commands are executed before build (if necessary)
      {
        Elem e1(e0, "PropertyGroup");
        std::ostringstream oss;
        oss << "\n";
        for (std::string const& i : this->CSharpCustomCommandNames) {
          oss << "      " << i << ";\n";
        }
        oss << "      "
               "$(BuildDependsOn)\n";
        e1.Element("BuildDependsOn", oss.str());
      }
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteSdkStyleProjectFile(
  cmGeneratedFileStream& BuildFileStream)
{
  if (this->ProjectType != VsProjectType::csproj ||
      !this->GeneratorTarget->IsDotNetSdkTarget()) {
    std::string message =
      cmStrCat("The target \"", this->GeneratorTarget->GetName(),
               "\" is not eligible for .Net SDK style project.");
    this->Makefile->IssueMessage(MessageType::INTERNAL_ERROR, message);
    return;
  }

  if (this->HasCustomCommands()) {
    std::string message = cmStrCat(
      "The target \"", this->GeneratorTarget->GetName(),
      "\" does not currently support add_custom_command as the Visual Studio "
      "generators have not yet learned how to generate custom commands in "
      ".Net SDK-style projects.");
    this->Makefile->IssueMessage(MessageType::FATAL_ERROR, message);
    return;
  }

  Elem e0(BuildFileStream, "Project");
  e0.Attribute("Sdk", *this->GeneratorTarget->GetProperty("DOTNET_SDK"));

  {
    Elem e1(e0, "PropertyGroup");
    this->WriteCommonPropertyGroupGlobals(e1);

    e1.Element("EnableDefaultItems", "false");
    // Disable the project upgrade prompt that is displayed the first time a
    // project using an older toolset version is opened in a newer version
    // of the IDE.
    e1.Element("VCProjectUpgraderObjectName", "NoUpgrade");
    e1.Element("ManagedAssembly", "true");

    cmValue targetFramework =
      this->GeneratorTarget->GetProperty("DOTNET_TARGET_FRAMEWORK");
    if (targetFramework) {
      if (targetFramework->find(';') != std::string::npos) {
        e1.Element("TargetFrameworks", *targetFramework);
      } else {
        e1.Element("TargetFramework", *targetFramework);
        e1.Element("AppendTargetFrameworkToOutputPath", "false");
      }
    } else {
      e1.Element("TargetFramework", "net5.0");
      e1.Element("AppendTargetFrameworkToOutputPath", "false");
    }

    std::string outputType;
    switch (this->GeneratorTarget->GetType()) {
      case cmStateEnums::OBJECT_LIBRARY:
      case cmStateEnums::STATIC_LIBRARY:
      case cmStateEnums::MODULE_LIBRARY:
        this->Makefile->IssueMessage(
          MessageType::FATAL_ERROR,
          cmStrCat("Target \"", this->GeneratorTarget->GetName(),
                   "\" is of a type not supported for managed binaries."));
        return;
      case cmStateEnums::SHARED_LIBRARY:
        outputType = "Library";
        break;
      case cmStateEnums::EXECUTABLE: {
        auto const win32 =
          this->GeneratorTarget->GetSafeProperty("WIN32_EXECUTABLE");
        if (win32.find("$<") != std::string::npos) {
          this->Makefile->IssueMessage(
            MessageType::FATAL_ERROR,
            cmStrCat("Target \"", this->GeneratorTarget->GetName(),
                     "\" has a generator expression in its WIN32_EXECUTABLE "
                     "property. This is not supported on managed "
                     "executables."));
          return;
        }
        if (cmIsOn(win32)) {
          outputType = "WinExe";
        } else {
          outputType = "Exe";
        }
      } break;
      case cmStateEnums::UTILITY:
      case cmStateEnums::INTERFACE_LIBRARY:
      case cmStateEnums::GLOBAL_TARGET:
        outputType = "Utility";
        break;
      case cmStateEnums::UNKNOWN_LIBRARY:
        break;
    }
    e1.Element("OutputType", outputType);

    cmValue startupObject =
      this->GeneratorTarget->GetProperty("VS_DOTNET_STARTUP_OBJECT");
    if (startupObject) {
      e1.Element("StartupObject", *startupObject);
    }
  }

  for (const std::string& config : this->Configurations) {
    Elem e1(e0, "PropertyGroup");
    e1.Attribute("Condition",
                 cmStrCat("'$(Configuration)' == '", config, '\''));
    e1.SetHasElements();
    this->WriteEvents(e1, config);

    std::string outDir =
      cmStrCat(this->GeneratorTarget->GetDirectory(config), '/');
    ConvertToWindowsSlash(outDir);
    e1.Element("OutputPath", outDir);

    Options& o = *(this->ClOptions[config]);
    OptionsHelper oh(o, e1);
    oh.OutputFlagMap();
  }

  this->WriteDotNetDocumentationFile(e0);
  this->WriteAllSources(e0);
  this->WriteEmbeddedResourceGroup(e0);
  this->WriteXamlFilesGroup(e0);
  this->WriteDotNetReferences(e0);
  this->WritePackageReferences(e0);
  this->WriteProjectReferences(e0);
}

void cmVisualStudio10TargetGenerator::WriteCommonPropertyGroupGlobals(Elem& e1)
{
  e1.Attribute("Label", "Globals");
  e1.Element("ProjectGuid", cmStrCat('{', this->GUID, '}'));

  cmValue vsProjectTypes =
    this->GeneratorTarget->GetProperty("VS_GLOBAL_PROJECT_TYPES");
  if (vsProjectTypes) {
    const char* tagName = "ProjectTypes";
    if (this->ProjectType == VsProjectType::csproj) {
      tagName = "ProjectTypeGuids";
    }
    e1.Element(tagName, *vsProjectTypes);
  }

  cmValue vsGlobalKeyword =
    this->GeneratorTarget->GetProperty("VS_GLOBAL_KEYWORD");
  if (!vsGlobalKeyword) {
    if (this->GlobalGenerator->TargetsAndroid()) {
      e1.Element("Keyword", "Android");
    } else {
      e1.Element("Keyword", "Win32Proj");
    }
  } else {
    e1.Element("Keyword", *vsGlobalKeyword);
  }

  cmValue vsGlobalRootNamespace =
    this->GeneratorTarget->GetProperty("VS_GLOBAL_ROOTNAMESPACE");
  if (vsGlobalRootNamespace) {
    e1.Element("RootNamespace", *vsGlobalRootNamespace);
  }

  std::vector<std::string> keys = this->GeneratorTarget->GetPropertyKeys();
  for (std::string const& keyIt : keys) {
    static const cm::string_view prefix = "VS_GLOBAL_";
    if (!cmHasPrefix(keyIt, prefix)) {
      continue;
    }
    cm::string_view globalKey = cm::string_view(keyIt).substr(prefix.length());
    // Skip invalid or separately-handled properties.
    if (globalKey.empty() || globalKey == "PROJECT_TYPES"_s ||
        globalKey == "ROOTNAMESPACE"_s || globalKey == "KEYWORD"_s) {
      continue;
    }
    cmValue value = this->GeneratorTarget->GetProperty(keyIt);
    if (!value) {
      continue;
    }
    e1.Element(globalKey, *value);
  }
}

bool cmVisualStudio10TargetGenerator::HasCustomCommands() const
{
  if (!this->GeneratorTarget->GetPreBuildCommands().empty() ||
      !this->GeneratorTarget->GetPreLinkCommands().empty() ||
      !this->GeneratorTarget->GetPostBuildCommands().empty()) {
    return true;
  }

  auto const& config_sources = this->GeneratorTarget->GetAllConfigSources();
  return std::any_of(config_sources.begin(), config_sources.end(),
                     [](cmGeneratorTarget::AllConfigSource const& si) {
                       return si.Source->GetCustomCommand();
                     });
}

void cmVisualStudio10TargetGenerator::WritePackageReferences(Elem& e0)
{
  std::vector<std::string> packageReferences =
    this->GeneratorTarget->GetPackageReferences();

  if (!packageReferences.empty()) {
    Elem e1(e0, "ItemGroup");
    for (std::string const& ri : packageReferences) {
      size_t versionIndex = ri.find_last_of('_');
      if (versionIndex != std::string::npos) {
        Elem e2(e1, "PackageReference");
        e2.Attribute("Include", ri.substr(0, versionIndex));
        e2.Attribute("Version", ri.substr(versionIndex + 1));
      }
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteDotNetReferences(Elem& e0)
{
  cmList references;
  if (cmValue vsDotNetReferences =
        this->GeneratorTarget->GetProperty("VS_DOTNET_REFERENCES")) {
    references.assign(*vsDotNetReferences);
  }
  cmPropertyMap const& props = this->GeneratorTarget->Target->GetProperties();
  for (auto const& i : props.GetList()) {
    static const cm::string_view vsDnRef = "VS_DOTNET_REFERENCE_";
    if (cmHasPrefix(i.first, vsDnRef)) {
      std::string path = i.second;
      if (!cmsys::SystemTools::FileIsFullPath(path)) {
        path =
          cmStrCat(this->Makefile->GetCurrentSourceDirectory(), '/', path);
      }
      ConvertToWindowsSlash(path);
      this->DotNetHintReferences[""].emplace_back(
        DotNetHintReference(i.first.substr(vsDnRef.length()), path));
    }
  }
  if (!references.empty() || !this->DotNetHintReferences.empty()) {
    Elem e1(e0, "ItemGroup");
    for (auto const& ri : references) {
      // if the entry from VS_DOTNET_REFERENCES is an existing file, generate
      // a new hint-reference and name it from the filename
      if (cmsys::SystemTools::FileExists(ri, true)) {
        std::string name =
          cmsys::SystemTools::GetFilenameWithoutLastExtension(ri);
        std::string path = ri;
        ConvertToWindowsSlash(path);
        this->DotNetHintReferences[""].emplace_back(
          DotNetHintReference(name, path));
      } else {
        this->WriteDotNetReference(e1, ri, "", "");
      }
    }
    for (const auto& h : this->DotNetHintReferences) {
      // DotNetHintReferences is also populated from AddLibraries().
      // The configuration specific hint references are added there.
      for (const auto& i : h.second) {
        this->WriteDotNetReference(e1, i.first, i.second, h.first);
      }
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteDotNetReference(
  Elem& e1, std::string const& ref, std::string const& hint,
  std::string const& config)
{
  Elem e2(e1, "Reference");
  // If 'config' is not empty, the reference is only added for the given
  // configuration. This is used when referencing imported managed assemblies.
  // See also cmVisualStudio10TargetGenerator::AddLibraries().
  if (!config.empty()) {
    e2.Attribute("Condition", this->CalcCondition(config));
  }
  e2.Attribute("Include", ref);
  e2.Element("CopyLocalSatelliteAssemblies", "true");
  e2.Element("ReferenceOutputAssembly", "true");
  if (!hint.empty()) {
    const char* privateReference = "True";
    if (cmValue value = this->GeneratorTarget->GetProperty(
          "VS_DOTNET_REFERENCES_COPY_LOCAL")) {
      if (cmIsOff(*value)) {
        privateReference = "False";
      }
    }
    e2.Element("Private", privateReference);
    e2.Element("HintPath", hint);
  }
  this->WriteDotNetReferenceCustomTags(e2, ref);
}

void cmVisualStudio10TargetGenerator::WriteImports(Elem& e0)
{
  cmValue imports =
    this->GeneratorTarget->Target->GetProperty("VS_PROJECT_IMPORT");
  if (imports) {
    cmList argsSplit{ *imports };
    for (auto& path : argsSplit) {
      if (!cmsys::SystemTools::FileIsFullPath(path)) {
        path =
          cmStrCat(this->Makefile->GetCurrentSourceDirectory(), '/', path);
      }
      ConvertToWindowsSlash(path);
      Elem e1(e0, "Import");
      e1.Attribute("Project", path);
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteDotNetReferenceCustomTags(
  Elem& e2, std::string const& ref)
{

  static const std::string refpropPrefix = "VS_DOTNET_REFERENCEPROP_";
  static const std::string refpropInfix = "_TAG_";
  const std::string refPropFullPrefix =
    cmStrCat(refpropPrefix, ref, refpropInfix);
  using CustomTags = std::map<std::string, std::string>;
  CustomTags tags;
  cmPropertyMap const& props = this->GeneratorTarget->Target->GetProperties();
  for (const auto& i : props.GetList()) {
    if (cmHasPrefix(i.first, refPropFullPrefix) && !i.second.empty()) {
      tags[i.first.substr(refPropFullPrefix.length())] = i.second;
    }
  }
  for (auto const& tag : tags) {
    e2.Element(tag.first, tag.second);
  }
}

void cmVisualStudio10TargetGenerator::WriteDotNetDocumentationFile(Elem& e0)
{
  std::string const& documentationFile =
    this->GeneratorTarget->GetSafeProperty("VS_DOTNET_DOCUMENTATION_FILE");

  if (this->ProjectType == VsProjectType::csproj &&
      !documentationFile.empty()) {
    Elem e1(e0, "PropertyGroup");
    Elem e2(e1, "DocumentationFile");
    e2.Content(documentationFile);
  }
}

void cmVisualStudio10TargetGenerator::WriteEmbeddedResourceGroup(Elem& e0)
{
  if (!this->ResxObjs.empty()) {
    Elem e1(e0, "ItemGroup");
    std::string srcDir = this->Makefile->GetCurrentSourceDirectory();
    ConvertToWindowsSlash(srcDir);
    for (cmSourceFile const* oi : this->ResxObjs) {
      std::string obj = oi->GetFullPath();
      ConvertToWindowsSlash(obj);
      bool useRelativePath = false;
      if (this->ProjectType == VsProjectType::csproj && this->InSourceBuild) {
        // If we do an in-source build and the resource file is in a
        // subdirectory
        // of the .csproj file, we have to use relative pathnames, otherwise
        // visual studio does not show the file in the IDE. Sorry.
        if (cmHasPrefix(obj, srcDir)) {
          obj = this->ConvertPath(obj, true);
          ConvertToWindowsSlash(obj);
          useRelativePath = true;
        }
      }
      Elem e2(e1, "EmbeddedResource");
      e2.Attribute("Include", obj);

      if (this->ProjectType != VsProjectType::csproj) {
        std::string hFileName =
          cmStrCat(obj.substr(0, obj.find_last_of('.')), ".h");
        e2.Element("DependentUpon", hFileName);

        for (std::string const& c : this->Configurations) {
          std::string s;
          if (this->GeneratorTarget->GetProperty("VS_GLOBAL_ROOTNAMESPACE") ||
              // Handle variant of VS_GLOBAL_<variable> for RootNamespace.
              this->GeneratorTarget->GetProperty("VS_GLOBAL_RootNamespace")) {
            s = "$(RootNamespace).";
          }
          s += "%(Filename).resources";
          e2.WritePlatformConfigTag("LogicalName", this->CalcCondition(c), s);
        }
      } else {
        std::string binDir = this->Makefile->GetCurrentBinaryDirectory();
        ConvertToWindowsSlash(binDir);
        // If the resource was NOT added using a relative path (which should
        // be the default), we have to provide a link here
        if (!useRelativePath) {
          std::string link = this->GetCSharpSourceLink(oi);
          if (link.empty()) {
            link = cmsys::SystemTools::GetFilenameName(obj);
          }
          e2.Element("Link", link);
        }
        // Determine if this is a generated resource from a .Designer.cs file
        std::string designerResource = cmStrCat(
          cmSystemTools::GetFilenamePath(oi->GetFullPath()), '/',
          cmSystemTools::GetFilenameWithoutLastExtension(oi->GetFullPath()),
          ".Designer.cs");
        if (cmsys::SystemTools::FileExists(designerResource)) {
          std::string generator = "PublicResXFileCodeGenerator";
          if (cmValue g = oi->GetProperty("VS_RESOURCE_GENERATOR")) {
            generator = *g;
          }
          if (!generator.empty()) {
            e2.Element("Generator", generator);
            if (cmHasPrefix(designerResource, srcDir)) {
              designerResource.erase(0, srcDir.length());
            } else if (cmHasPrefix(designerResource, binDir)) {
              designerResource.erase(0, binDir.length());
            } else {
              designerResource =
                cmsys::SystemTools::GetFilenameName(designerResource);
            }
            ConvertToWindowsSlash(designerResource);
            e2.Element("LastGenOutput", designerResource);
          }
        }
        const cmPropertyMap& props = oi->GetProperties();
        for (const std::string& p : props.GetKeys()) {
          static const cm::string_view propNamePrefix = "VS_CSHARP_";
          if (cmHasPrefix(p, propNamePrefix)) {
            cm::string_view tagName =
              cm::string_view(p).substr(propNamePrefix.length());
            if (!tagName.empty()) {
              cmValue value = props.GetPropertyValue(p);
              if (cmNonempty(value)) {
                e2.Element(tagName, *value);
              }
            }
          }
        }
      }
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteXamlFilesGroup(Elem& e0)
{
  if (!this->XamlObjs.empty()) {
    Elem e1(e0, "ItemGroup");
    for (cmSourceFile const* oi : this->XamlObjs) {
      std::string obj = oi->GetFullPath();
      std::string xamlType;
      cmValue xamlTypeProperty = oi->GetProperty("VS_XAML_TYPE");
      if (xamlTypeProperty) {
        xamlType = *xamlTypeProperty;
      } else {
        xamlType = "Page";
      }

      Elem e2(e1, xamlType);
      this->WriteSource(e2, oi);
      e2.SetHasElements();
      e2.Element("SubType", "Designer");
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteTargetSpecificReferences(Elem& e0)
{
  if (this->MSTools) {
    if (this->GlobalGenerator->TargetsWindowsPhone() &&
        this->GlobalGenerator->GetSystemVersion() == "8.0"_s) {
      Elem(e0, "Import")
        .Attribute("Project",
                   "$(MSBuildExtensionsPath)\\Microsoft\\WindowsPhone\\v"
                   "$(TargetPlatformVersion)\\Microsoft.Cpp.WindowsPhone."
                   "$(TargetPlatformVersion).targets");
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteTargetsFileReferences(Elem& e1)
{
  for (TargetsFileAndConfigs const& tac : this->TargetsFileAndConfigsVec) {
    std::ostringstream oss;
    oss << "Exists('" << tac.File << "')";
    if (!tac.Configs.empty()) {
      oss << " And (";
      for (size_t j = 0; j < tac.Configs.size(); ++j) {
        if (j > 0) {
          oss << " Or ";
        }
        oss << "'$(Configuration)'=='" << tac.Configs[j] << '\'';
      }
      oss << ')';
    }

    Elem(e1, "Import")
      .Attribute("Project", tac.File)
      .Attribute("Condition", oss.str());
  }
}

void cmVisualStudio10TargetGenerator::WriteWinRTReferences(Elem& e0)
{
  cmList references;
  if (cmValue vsWinRTReferences =
        this->GeneratorTarget->GetProperty("VS_WINRT_REFERENCES")) {
    references.assign(*vsWinRTReferences);
  }

  if (this->GlobalGenerator->TargetsWindowsPhone() &&
      this->GlobalGenerator->GetSystemVersion() == "8.0"_s &&
      references.empty()) {
    references.push_back(std::string{ "platform.winmd" });
  }
  if (!references.empty()) {
    Elem e1(e0, "ItemGroup");
    for (auto const& ri : references) {
      Elem e2(e1, "Reference");
      e2.Attribute("Include", ri);
      e2.Element("IsWinMDFile", "true");
    }
  }
}

// ConfigurationType Application, Utility StaticLibrary DynamicLibrary

void cmVisualStudio10TargetGenerator::WriteProjectConfigurations(Elem& e0)
{
  Elem e1(e0, "ItemGroup");
  e1.Attribute("Label", "ProjectConfigurations");
  for (std::string const& c : this->Configurations) {
    Elem e2(e1, "ProjectConfiguration");
    e2.Attribute("Include", cmStrCat(c, '|', this->Platform));
    e2.Element("Configuration", c);
    e2.Element("Platform", this->Platform);
  }
}

void cmVisualStudio10TargetGenerator::WriteProjectConfigurationValues(Elem& e0)
{
  for (std::string const& c : this->Configurations) {
    Elem e1(e0, "PropertyGroup");
    e1.Attribute("Condition", this->CalcCondition(c));
    e1.Attribute("Label", "Configuration");

    if (this->ProjectType != VsProjectType::csproj) {
      std::string configType;
      if (cmValue vsConfigurationType =
            this->GeneratorTarget->GetProperty("VS_CONFIGURATION_TYPE")) {
        configType = cmGeneratorExpression::Evaluate(*vsConfigurationType,
                                                     this->LocalGenerator, c);
      } else {
        switch (this->GeneratorTarget->GetType()) {
          case cmStateEnums::SHARED_LIBRARY:
          case cmStateEnums::MODULE_LIBRARY:
            configType = "DynamicLibrary";
            break;
          case cmStateEnums::OBJECT_LIBRARY:
          case cmStateEnums::STATIC_LIBRARY:
            configType = "StaticLibrary";
            break;
          case cmStateEnums::EXECUTABLE:
            if (this->NsightTegra &&
                !this->GeneratorTarget->Target->IsAndroidGuiExecutable()) {
              // Android executables are .so too.
              configType = "DynamicLibrary";
            } else if (this->Android) {
              configType = "DynamicLibrary";
            } else {
              configType = "Application";
            }
            break;
          case cmStateEnums::UTILITY:
          case cmStateEnums::INTERFACE_LIBRARY:
          case cmStateEnums::GLOBAL_TARGET:
            if (this->NsightTegra) {
              // Tegra-Android platform does not understand "Utility".
              configType = "StaticLibrary";
            } else {
              configType = "Utility";
            }
            break;
          case cmStateEnums::UNKNOWN_LIBRARY:
            break;
        }
      }
      e1.Element("ConfigurationType", configType);
    }

    if (this->MSTools) {
      if (!this->Managed) {
        this->WriteMSToolConfigurationValues(e1, c);
      } else {
        this->WriteMSToolConfigurationValuesManaged(e1, c);
      }
    } else if (this->NsightTegra) {
      this->WriteNsightTegraConfigurationValues(e1, c);
    } else if (this->Android) {
      this->WriteAndroidConfigurationValues(e1, c);
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteCEDebugProjectConfigurationValues(
  Elem& e0)
{
  if (!this->GlobalGenerator->TargetsWindowsCE()) {
    return;
  }
  cmValue additionalFiles =
    this->GeneratorTarget->GetProperty("DEPLOYMENT_ADDITIONAL_FILES");
  cmValue remoteDirectory =
    this->GeneratorTarget->GetProperty("DEPLOYMENT_REMOTE_DIRECTORY");
  if (!(additionalFiles || remoteDirectory)) {
    return;
  }
  for (std::string const& c : this->Configurations) {
    Elem e1(e0, "PropertyGroup");
    e1.Attribute("Condition", this->CalcCondition(c));

    if (remoteDirectory) {
      e1.Element("RemoteDirectory", *remoteDirectory);
    }
    if (additionalFiles) {
      e1.Element("CEAdditionalFiles", *additionalFiles);
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteMSToolConfigurationValues(
  Elem& e1, std::string const& config)
{
  cmGlobalVisualStudio10Generator* gg = this->GlobalGenerator;
  cmValue mfcFlag = this->Makefile->GetDefinition("CMAKE_MFC_FLAG");
  if (mfcFlag) {
    std::string const mfcFlagValue =
      cmGeneratorExpression::Evaluate(*mfcFlag, this->LocalGenerator, config);

    std::string useOfMfcValue = "false";
    if (this->GeneratorTarget->GetType() <= cmStateEnums::OBJECT_LIBRARY) {
      if (mfcFlagValue == "1"_s) {
        useOfMfcValue = "Static";
      } else if (mfcFlagValue == "2"_s) {
        useOfMfcValue = "Dynamic";
      }
    }
    e1.Element("UseOfMfc", useOfMfcValue);
  }

  if ((this->GeneratorTarget->GetType() <= cmStateEnums::OBJECT_LIBRARY &&
       this->ClOptions[config]->UsingUnicode()) ||
      this->GeneratorTarget->GetPropertyAsBool("VS_WINRT_COMPONENT") ||
      this->GlobalGenerator->TargetsWindowsPhone() ||
      this->GlobalGenerator->TargetsWindowsStore() ||
      this->GeneratorTarget->GetPropertyAsBool("VS_WINRT_EXTENSIONS")) {
    e1.Element("CharacterSet", "Unicode");
  } else if (this->GeneratorTarget->GetType() <=
               cmStateEnums::OBJECT_LIBRARY &&
             this->ClOptions[config]->UsingSBCS()) {
    e1.Element("CharacterSet", "NotSet");
  } else {
    e1.Element("CharacterSet", "MultiByte");
  }
  if (cmValue projectToolsetOverride =
        this->GeneratorTarget->GetProperty("VS_PLATFORM_TOOLSET")) {
    e1.Element("PlatformToolset", *projectToolsetOverride);
  } else if (const char* toolset = gg->GetPlatformToolset()) {
    e1.Element("PlatformToolset", toolset);
  }
  if (this->GeneratorTarget->GetPropertyAsBool("VS_WINRT_COMPONENT") ||
      this->GeneratorTarget->GetPropertyAsBool("VS_WINRT_EXTENSIONS")) {
    e1.Element("WindowsAppContainer", "true");
  }
  if (this->IPOEnabledConfigurations.count(config) > 0) {
    e1.Element("WholeProgramOptimization", "true");
  }
  if (this->ASanEnabledConfigurations.find(config) !=
      this->ASanEnabledConfigurations.end()) {
    e1.Element("EnableAsan", "true");
  }
  if (this->FuzzerEnabledConfigurations.find(config) !=
      this->FuzzerEnabledConfigurations.end()) {
    e1.Element("EnableFuzzer", "true");
  }
  {
    auto s = this->SpectreMitigation.find(config);
    if (s != this->SpectreMitigation.end()) {
      e1.Element("SpectreMitigation", s->second);
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteMSToolConfigurationValuesManaged(
  Elem& e1, std::string const& config)
{
  if (this->GeneratorTarget->GetType() > cmStateEnums::OBJECT_LIBRARY) {
    return;
  }

  cmGlobalVisualStudio10Generator* gg = this->GlobalGenerator;

  Options& o = *(this->ClOptions[config]);

  if (o.IsDebug()) {
    e1.Element("DebugSymbols", "true");
    e1.Element("DefineDebug", "true");
  }

  std::string outDir =
    cmStrCat(this->GeneratorTarget->GetDirectory(config), '/');
  ConvertToWindowsSlash(outDir);
  e1.Element("OutputPath", outDir);

  if (o.HasFlag("Platform")) {
    e1.Element("PlatformTarget", o.GetFlag("Platform"));
    o.RemoveFlag("Platform");
  }

  if (cmValue projectToolsetOverride =
        this->GeneratorTarget->GetProperty("VS_PLATFORM_TOOLSET")) {
    e1.Element("PlatformToolset", *projectToolsetOverride);
  } else if (const char* toolset = gg->GetPlatformToolset()) {
    e1.Element("PlatformToolset", toolset);
  }

  std::string postfixName =
    cmStrCat(cmSystemTools::UpperCase(config), "_POSTFIX");
  std::string assemblyName = this->GeneratorTarget->GetOutputName(
    config, cmStateEnums::RuntimeBinaryArtifact);
  if (cmValue postfix = this->GeneratorTarget->GetProperty(postfixName)) {
    assemblyName += *postfix;
  }
  e1.Element("AssemblyName", assemblyName);

  if (cmStateEnums::EXECUTABLE == this->GeneratorTarget->GetType()) {
    e1.Element("StartAction", "Program");
    e1.Element("StartProgram", cmStrCat(outDir, assemblyName, ".exe"));
  }

  OptionsHelper oh(o, e1);
  oh.OutputFlagMap();
}

//----------------------------------------------------------------------------
void cmVisualStudio10TargetGenerator::WriteNsightTegraConfigurationValues(
  Elem& e1, std::string const&)
{
  cmGlobalVisualStudio10Generator* gg = this->GlobalGenerator;
  const char* toolset = gg->GetPlatformToolset();
  e1.Element("NdkToolchainVersion", toolset ? toolset : "Default");
  if (cmValue minApi = this->GeneratorTarget->GetProperty("ANDROID_API_MIN")) {
    e1.Element("AndroidMinAPI", cmStrCat("android-", *minApi));
  }
  if (cmValue api = this->GeneratorTarget->GetProperty("ANDROID_API")) {
    e1.Element("AndroidTargetAPI", cmStrCat("android-", *api));
  }

  if (cmValue cpuArch = this->GeneratorTarget->GetProperty("ANDROID_ARCH")) {
    e1.Element("AndroidArch", *cpuArch);
  }

  if (cmValue stlType =
        this->GeneratorTarget->GetProperty("ANDROID_STL_TYPE")) {
    e1.Element("AndroidStlType", *stlType);
  }
}

void cmVisualStudio10TargetGenerator::WriteAndroidConfigurationValues(
  Elem& e1, std::string const&)
{
  cmGlobalVisualStudio10Generator* gg = this->GlobalGenerator;
  if (cmValue projectToolsetOverride =
        this->GeneratorTarget->GetProperty("VS_PLATFORM_TOOLSET")) {
    e1.Element("PlatformToolset", *projectToolsetOverride);
  } else if (const char* toolset = gg->GetPlatformToolset()) {
    e1.Element("PlatformToolset", toolset);
  }
  if (cmValue stlType =
        this->GeneratorTarget->GetProperty("ANDROID_STL_TYPE")) {
    if (*stlType != "none"_s) {
      e1.Element("UseOfStl", *stlType);
    }
  }
  std::string const& apiLevel = gg->GetSystemVersion();
  if (!apiLevel.empty()) {
    e1.Element("AndroidAPILevel", cmStrCat("android-", apiLevel));
  }
}

void cmVisualStudio10TargetGenerator::WriteCustomCommands(Elem& e0)
{
  this->CSharpCustomCommandNames.clear();

  cmSourceFile const* srcCMakeLists =
    this->LocalGenerator->CreateVCProjBuildRule();

  for (cmGeneratorTarget::AllConfigSource const& si :
       this->GeneratorTarget->GetAllConfigSources()) {
    if (si.Source == srcCMakeLists) {
      // Skip explicit reference to CMakeLists.txt source.
      continue;
    }
    this->WriteCustomCommand(e0, si.Source);
  }

  // Add CMakeLists.txt file with rule to re-run CMake for user convenience.
  if (this->GeneratorTarget->GetType() != cmStateEnums::GLOBAL_TARGET &&
      this->GeneratorTarget->GetName() != CMAKE_CHECK_BUILD_SYSTEM_TARGET) {
    if (srcCMakeLists) {
      // Write directly rather than through WriteCustomCommand because
      // we do not want the de-duplication and it has no dependencies.
      if (cmCustomCommand const* command = srcCMakeLists->GetCustomCommand()) {
        this->WriteCustomRule(e0, srcCMakeLists, *command);
      }
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteCustomCommand(
  Elem& e0, cmSourceFile const* sf)
{
  if (this->LocalGenerator->GetSourcesVisited(this->GeneratorTarget)
        .insert(sf)
        .second) {
    if (std::vector<cmSourceFile*> const* depends =
          this->GeneratorTarget->GetSourceDepends(sf)) {
      for (cmSourceFile const* di : *depends) {
        this->WriteCustomCommand(e0, di);
      }
    }
    if (cmCustomCommand const* command = sf->GetCustomCommand()) {
      // C# projects write their <Target> within WriteCustomRule()
      this->WriteCustomRule(e0, sf, *command);
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteCustomRule(
  Elem& e0, cmSourceFile const* source, cmCustomCommand const& command)
{
  std::string sourcePath = source->GetFullPath();
  // VS 10 will always rebuild a custom command attached to a .rule
  // file that doesn't exist so create the file explicitly.
  if (source->GetPropertyAsBool("__CMAKE_RULE")) {
    if (!cmSystemTools::FileExists(sourcePath)) {
      // Make sure the path exists for the file
      std::string path = cmSystemTools::GetFilenamePath(sourcePath);
      cmSystemTools::MakeDirectory(path);
      cmsys::ofstream fout(sourcePath.c_str());
      if (fout) {
        fout << "# generated from CMake\n";
        fout.flush();
        fout.close();
        // Force given file to have a very old timestamp, thus
        // preventing dependent rebuilds.
        this->ForceOld(sourcePath);
      } else {
        cmSystemTools::Error(cmStrCat("Could not create file: [", sourcePath,
                                      "]  ",
                                      cmSystemTools::GetLastSystemError()));
      }
    }
  }
  cmLocalVisualStudio7Generator* lg = this->LocalGenerator;

  std::unique_ptr<Elem> spe1;
  std::unique_ptr<Elem> spe2;
  if (this->ProjectType != VsProjectType::csproj) {
    spe1 = cm::make_unique<Elem>(e0, "ItemGroup");
    spe2 = cm::make_unique<Elem>(*spe1, "CustomBuild");
    this->WriteSource(*spe2, source);
    spe2->SetHasElements();
    if (command.GetStdPipesUTF8()) {
      this->WriteStdOutEncodingUtf8(*spe2);
    }
  } else {
    Elem e1(e0, "ItemGroup");
    Elem e2(e1, "None");
    this->WriteSource(e2, source);
    e2.SetHasElements();
  }
  for (std::string const& c : this->Configurations) {
    cmCustomCommandGenerator ccg(command, c, lg, true);
    std::string comment = lg->ConstructComment(ccg);
    comment = cmVS10EscapeComment(comment);
    std::string script = lg->ConstructScript(ccg);
    bool symbolic = false;
    // input files for custom command
    std::stringstream additional_inputs;
    {
      const char* sep = "";
      if (this->ProjectType == VsProjectType::csproj) {
        // csproj files do not attach the command to a specific file
        // so the primary input must be listed explicitly.
        additional_inputs << source->GetFullPath();
        sep = ";";
      }

      // Avoid listing an input more than once.
      std::set<std::string> unique_inputs;
      // The source is either implicit an input or has been added above.
      unique_inputs.insert(source->GetFullPath());

      for (std::string const& d : ccg.GetDepends()) {
        std::string dep;
        if (lg->GetRealDependency(d, c, dep)) {
          if (!unique_inputs.insert(dep).second) {
            // already listed
            continue;
          }
          ConvertToWindowsSlash(dep);
          additional_inputs << sep << dep;
          sep = ";";
          if (!symbolic) {
            if (cmSourceFile* sf = this->Makefile->GetSource(
                  dep, cmSourceFileLocationKind::Known)) {
              symbolic = sf->GetPropertyAsBool("SYMBOLIC");
            }
          }
        }
      }
      if (this->ProjectType != VsProjectType::csproj) {
        additional_inputs << sep << "%(AdditionalInputs)";
      }
    }
    // output files for custom command
    std::stringstream outputs;
    {
      const char* sep = "";
      for (std::string const& o : ccg.GetOutputs()) {
        std::string out = o;
        ConvertToWindowsSlash(out);
        outputs << sep << out;
        sep = ";";
        if (!symbolic) {
          if (cmSourceFile* sf = this->Makefile->GetSource(
                o, cmSourceFileLocationKind::Known)) {
            symbolic = sf->GetPropertyAsBool("SYMBOLIC");
          }
        }
      }
    }
    script += lg->FinishConstructScript(this->ProjectType);
    if (this->ProjectType == VsProjectType::csproj) {
      cmCryptoHash hasher(cmCryptoHash::AlgoMD5);
      std::string name =
        cmStrCat("CustomCommand_", c, '_', hasher.HashString(sourcePath));
      this->WriteCustomRuleCSharp(e0, c, name, script, additional_inputs.str(),
                                  outputs.str(), comment, ccg);
    } else {
      BuildInParallel buildInParallel = BuildInParallel::No;
      if (command.GetCMP0147Status() == cmPolicies::NEW &&
          !command.GetUsesTerminal() &&
          !(command.HasMainDependency() && source->GetIsGenerated())) {
        buildInParallel = BuildInParallel::Yes;
      }
      this->WriteCustomRuleCpp(*spe2, c, script, additional_inputs.str(),
                               outputs.str(), comment, ccg, symbolic,
                               buildInParallel);
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteCustomRuleCpp(
  Elem& e2, std::string const& config, std::string const& script,
  std::string const& additional_inputs, std::string const& outputs,
  std::string const& comment, cmCustomCommandGenerator const& ccg,
  bool symbolic, BuildInParallel buildInParallel)
{
  const std::string cond = this->CalcCondition(config);
  if (buildInParallel == BuildInParallel::Yes &&
      this->GlobalGenerator->IsBuildInParallelSupported()) {
    e2.WritePlatformConfigTag("BuildInParallel", cond, "true");
  }
  e2.WritePlatformConfigTag("Message", cond, comment);
  e2.WritePlatformConfigTag("Command", cond, script);
  e2.WritePlatformConfigTag("AdditionalInputs", cond, additional_inputs);
  e2.WritePlatformConfigTag("Outputs", cond, outputs);
  // Turn off linking of custom command outputs.
  e2.WritePlatformConfigTag("LinkObjects", cond, "false");
  if (symbolic &&
      this->LocalGenerator->GetVersion() >=
        cmGlobalVisualStudioGenerator::VSVersion::VS16) {
    // VS >= 16.4 warn if outputs are not created, but one of our
    // outputs is marked SYMBOLIC and not expected to be created.
    e2.WritePlatformConfigTag("VerifyInputsAndOutputsExist", cond, "false");
  }

  std::string depfile = ccg.GetFullDepfile();
  if (!depfile.empty()) {
    this->HaveCustomCommandDepfile = true;
    std::string internal_depfile = ccg.GetInternalDepfile();
    ConvertToWindowsSlash(internal_depfile);
    e2.WritePlatformConfigTag("DepFileAdditionalInputsFile", cond,
                              internal_depfile);
  }
}

void cmVisualStudio10TargetGenerator::WriteCustomRuleCSharp(
  Elem& e0, std::string const& config, std::string const& name,
  std::string const& script, std::string const& inputs,
  std::string const& outputs, std::string const& comment,
  cmCustomCommandGenerator const& ccg)
{
  if (!ccg.GetFullDepfile().empty()) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("CSharp target \"", this->GeneratorTarget->GetName(),
               "\" does not support add_custom_command DEPFILE."));
  }
  this->CSharpCustomCommandNames.insert(name);
  Elem e1(e0, "Target");
  e1.Attribute("Condition", this->CalcCondition(config));
  e1.S << "\n    Name=\"" << name << "\"";
  e1.S << "\n    Inputs=\"" << cmVS10EscapeAttr(inputs) << "\"";
  e1.S << "\n    Outputs=\"" << cmVS10EscapeAttr(outputs) << "\"";
  if (!comment.empty()) {
    Elem(e1, "Exec").Attribute("Command", cmStrCat("echo ", comment));
  }
  Elem(e1, "Exec").Attribute("Command", script);
}

std::string cmVisualStudio10TargetGenerator::ConvertPath(
  std::string const& path, bool forceRelative)
{
  return forceRelative
    ? cmSystemTools::RelativePath(
        this->LocalGenerator->GetCurrentBinaryDirectory(), path)
    : path;
}

static void ConvertToWindowsSlash(std::string& s)
{
  // first convert all of the slashes
  for (auto& ch : s) {
    if (ch == '/') {
      ch = '\\';
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteGroups()
{
  if (this->ProjectType == VsProjectType::csproj) {
    return;
  }

  // collect up group information
  std::vector<cmSourceGroup> sourceGroups = this->Makefile->GetSourceGroups();

  std::vector<cmGeneratorTarget::AllConfigSource> const& sources =
    this->GeneratorTarget->GetAllConfigSources();

  std::set<cmSourceGroup const*> groupsUsed;
  for (cmGeneratorTarget::AllConfigSource const& si : sources) {
    std::string const& source = si.Source->GetFullPath();
    cmSourceGroup* sourceGroup =
      this->Makefile->FindSourceGroup(source, sourceGroups);
    groupsUsed.insert(sourceGroup);
  }

  if (cmSourceFile const* srcCMakeLists =
        this->LocalGenerator->CreateVCProjBuildRule()) {
    std::string const& source = srcCMakeLists->GetFullPath();
    cmSourceGroup* sourceGroup =
      this->Makefile->FindSourceGroup(source, sourceGroups);
    groupsUsed.insert(sourceGroup);
  }

  this->AddMissingSourceGroups(groupsUsed, sourceGroups);

  // Write out group file
  std::string path = cmStrCat(
    this->LocalGenerator->GetCurrentBinaryDirectory(), '/', this->Name,
    computeProjectFileExtension(this->GeneratorTarget), ".filters");
  cmGeneratedFileStream fout(path);
  fout.SetCopyIfDifferent(true);
  char magic[] = { char(0xEF), char(0xBB), char(0xBF) };
  fout.write(magic, 3);

  fout << R"(<?xml version="1.0" encoding=")"
       << this->GlobalGenerator->Encoding() << "\"?>";
  {
    Elem e0(fout, "Project");
    e0.Attribute("ToolsVersion", this->GlobalGenerator->GetToolsVersion());
    e0.Attribute("xmlns",
                 "http://schemas.microsoft.com/developer/msbuild/2003");

    for (auto const& ti : this->Tools) {
      this->WriteGroupSources(e0, ti.first, ti.second, sourceGroups);
    }

    // Added files are images and the manifest.
    if (!this->AddedFiles.empty()) {
      Elem e1(e0, "ItemGroup");
      e1.SetHasElements();
      for (std::string const& oi : this->AddedFiles) {
        std::string fileName =
          cmSystemTools::LowerCase(cmSystemTools::GetFilenameName(oi));
        if (fileName == "wmappmanifest.xml"_s) {
          Elem e2(e1, "XML");
          e2.Attribute("Include", oi);
          e2.Element("Filter", "Resource Files");
        } else if (cmSystemTools::GetFilenameExtension(fileName) ==
                   ".appxmanifest") {
          Elem e2(e1, "AppxManifest");
          e2.Attribute("Include", oi);
          e2.Element("Filter", "Resource Files");
        } else if (cmSystemTools::GetFilenameExtension(fileName) == ".pfx"_s) {
          Elem e2(e1, "None");
          e2.Attribute("Include", oi);
          e2.Element("Filter", "Resource Files");
        } else {
          Elem e2(e1, "Image");
          e2.Attribute("Include", oi);
          e2.Element("Filter", "Resource Files");
        }
      }
    }

    if (!this->ResxObjs.empty()) {
      Elem e1(e0, "ItemGroup");
      for (cmSourceFile const* oi : this->ResxObjs) {
        std::string obj = oi->GetFullPath();
        ConvertToWindowsSlash(obj);
        Elem e2(e1, "EmbeddedResource");
        e2.Attribute("Include", obj);
        e2.Element("Filter", "Resource Files");
      }
    }
    {
      Elem e1(e0, "ItemGroup");
      e1.SetHasElements();
      std::vector<cmSourceGroup const*> groupsVec(groupsUsed.begin(),
                                                  groupsUsed.end());
      std::sort(groupsVec.begin(), groupsVec.end(),
                [](cmSourceGroup const* l, cmSourceGroup const* r) {
                  return l->GetFullName() < r->GetFullName();
                });
      for (cmSourceGroup const* sg : groupsVec) {
        std::string const& name = sg->GetFullName();
        if (!name.empty()) {
          std::string guidName = cmStrCat("SG_Filter_", name);
          std::string guid = this->GlobalGenerator->GetGUID(guidName);
          Elem e2(e1, "Filter");
          e2.Attribute("Include", name);
          e2.Element("UniqueIdentifier", cmStrCat('{', guid, '}'));
        }
      }

      if (!this->ResxObjs.empty() || !this->AddedFiles.empty()) {
        std::string guidName = "SG_Filter_Resource Files";
        std::string guid = this->GlobalGenerator->GetGUID(guidName);
        Elem e2(e1, "Filter");
        e2.Attribute("Include", "Resource Files");
        e2.Element("UniqueIdentifier", cmStrCat('{', guid, '}'));
        e2.Element("Extensions",
                   "rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;"
                   "gif;jpg;jpeg;jpe;resx;tiff;tif;png;wav;mfcribbon-ms");
      }
    }
  }
  fout << '\n';

  if (fout.Close()) {
    this->GlobalGenerator->FileReplacedDuringGenerate(path);
  }
}

// Add to groupsUsed empty source groups that have non-empty children.
void cmVisualStudio10TargetGenerator::AddMissingSourceGroups(
  std::set<cmSourceGroup const*>& groupsUsed,
  const std::vector<cmSourceGroup>& allGroups)
{
  for (cmSourceGroup const& current : allGroups) {
    std::vector<cmSourceGroup> const& children = current.GetGroupChildren();
    if (children.empty()) {
      continue; // the group is really empty
    }

    this->AddMissingSourceGroups(groupsUsed, children);

    if (groupsUsed.count(&current) > 0) {
      continue; // group has already been added to set
    }

    // check if it least one of the group's descendants is not empty
    // (at least one child must already have been added)
    auto child_it = children.begin();
    while (child_it != children.end()) {
      if (groupsUsed.count(&(*child_it)) > 0) {
        break; // found a child that was already added => add current group too
      }
      child_it++;
    }

    if (child_it == children.end()) {
      continue; // no descendants have source files => ignore this group
    }

    groupsUsed.insert(&current);
  }
}

void cmVisualStudio10TargetGenerator::WriteGroupSources(
  Elem& e0, std::string const& name, ToolSources const& sources,
  std::vector<cmSourceGroup>& sourceGroups)
{
  Elem e1(e0, "ItemGroup");
  e1.SetHasElements();
  for (ToolSource const& s : sources) {
    cmSourceFile const* sf = s.SourceFile;
    std::string const& source = sf->GetFullPath();
    cmSourceGroup* sourceGroup =
      this->Makefile->FindSourceGroup(source, sourceGroups);
    std::string const& filter = sourceGroup->GetFullName();
    std::string path = this->ConvertPath(source, s.RelativePath);
    ConvertToWindowsSlash(path);
    Elem e2(e1, name);
    e2.Attribute("Include", path);
    if (!filter.empty()) {
      e2.Element("Filter", filter);
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteHeaderSource(
  Elem& e1, cmSourceFile const* sf, ConfigToSettings const& toolSettings)
{
  std::string const& fileName = sf->GetFullPath();
  Elem e2(e1, "ClInclude");
  this->WriteSource(e2, sf);
  if (this->IsResxHeader(fileName)) {
    e2.Element("FileType", "CppForm");
  } else if (this->IsXamlHeader(fileName)) {
    e2.Element("DependentUpon",
               fileName.substr(0, fileName.find_last_of('.')));
  }
  this->FinishWritingSource(e2, toolSettings);
}

void cmVisualStudio10TargetGenerator::ParseSettingsProperty(
  const std::string& settingsPropertyValue, ConfigToSettings& toolSettings)
{
  if (!settingsPropertyValue.empty()) {
    cmGeneratorExpression ge(*this->LocalGenerator->GetCMakeInstance());

    std::unique_ptr<cmCompiledGeneratorExpression> cge =
      ge.Parse(settingsPropertyValue);

    for (const std::string& config : this->Configurations) {
      std::string evaluated = cge->Evaluate(this->LocalGenerator, config);

      cmList settings{ evaluated };
      for (const auto& setting : settings) {
        const std::string::size_type assignment = setting.find('=');
        if (assignment != std::string::npos) {
          const std::string propName = setting.substr(0, assignment);
          const std::string propValue = setting.substr(assignment + 1);

          if (!propValue.empty()) {
            toolSettings[config][propName] = propValue;
          }
        }
      }
    }
  }
}

bool cmVisualStudio10TargetGenerator::PropertyIsSameInAllConfigs(
  const ConfigToSettings& toolSettings, const std::string& propName)
{
  std::string firstPropValue;
  for (const auto& configToSettings : toolSettings) {
    const std::unordered_map<std::string, std::string>& settings =
      configToSettings.second;

    if (firstPropValue.empty()) {
      if (settings.find(propName) != settings.end()) {
        firstPropValue = settings.find(propName)->second;
      }
    }

    if (settings.find(propName) == settings.end()) {
      return false;
    }

    if (settings.find(propName)->second != firstPropValue) {
      return false;
    }
  }

  return true;
}

void cmVisualStudio10TargetGenerator::WriteExtraSource(
  Elem& e1, cmSourceFile const* sf, ConfigToSettings& toolSettings)
{
  bool toolHasSettings = false;
  const char* tool = "None";
  std::string settingsGenerator;
  std::string settingsLastGenOutput;
  std::string sourceLink;
  std::string subType;
  std::string copyToOutDir;
  std::string includeInVsix;
  std::string ext = cmSystemTools::LowerCase(sf->GetExtension());

  if (this->ProjectType == VsProjectType::csproj && !this->InSourceBuild) {
    toolHasSettings = true;
  }
  if (ext == "hlsl"_s) {
    tool = "FXCompile";
    // Figure out the type of shader compiler to use.
    if (cmValue st = sf->GetProperty("VS_SHADER_TYPE")) {
      for (const std::string& config : this->Configurations) {
        toolSettings[config]["ShaderType"] = *st;
      }
    }
    // Figure out which entry point to use if any
    if (cmValue se = sf->GetProperty("VS_SHADER_ENTRYPOINT")) {
      for (const std::string& config : this->Configurations) {
        toolSettings[config]["EntryPointName"] = *se;
      }
    }
    // Figure out which shader model to use if any
    if (cmValue sm = sf->GetProperty("VS_SHADER_MODEL")) {
      for (const std::string& config : this->Configurations) {
        toolSettings[config]["ShaderModel"] = *sm;
      }
    }
    // Figure out which output header file to use if any
    if (cmValue ohf = sf->GetProperty("VS_SHADER_OUTPUT_HEADER_FILE")) {
      for (const std::string& config : this->Configurations) {
        toolSettings[config]["HeaderFileOutput"] = *ohf;
      }
    }
    // Figure out which variable name to use if any
    if (cmValue vn = sf->GetProperty("VS_SHADER_VARIABLE_NAME")) {
      for (const std::string& config : this->Configurations) {
        toolSettings[config]["VariableName"] = *vn;
      }
    }
    // Figure out if there's any additional flags to use
    if (cmValue saf = sf->GetProperty("VS_SHADER_FLAGS")) {
      cmGeneratorExpression ge(*this->LocalGenerator->GetCMakeInstance());
      std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(*saf);

      for (const std::string& config : this->Configurations) {
        std::string evaluated = cge->Evaluate(this->LocalGenerator, config);

        if (!evaluated.empty()) {
          toolSettings[config]["AdditionalOptions"] = evaluated;
        }
      }
    }
    // Figure out if debug information should be generated
    if (cmValue sed = sf->GetProperty("VS_SHADER_ENABLE_DEBUG")) {
      cmGeneratorExpression ge(*this->LocalGenerator->GetCMakeInstance());
      std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(*sed);

      for (const std::string& config : this->Configurations) {
        std::string evaluated = cge->Evaluate(this->LocalGenerator, config);

        if (!evaluated.empty()) {
          toolSettings[config]["EnableDebuggingInformation"] =
            cmIsOn(evaluated) ? "true" : "false";
        }
      }
    }
    // Figure out if optimizations should be disabled
    if (cmValue sdo = sf->GetProperty("VS_SHADER_DISABLE_OPTIMIZATIONS")) {
      cmGeneratorExpression ge(*this->LocalGenerator->GetCMakeInstance());
      std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(*sdo);

      for (const std::string& config : this->Configurations) {
        std::string evaluated = cge->Evaluate(this->LocalGenerator, config);

        if (!evaluated.empty()) {
          toolSettings[config]["DisableOptimizations"] =
            cmIsOn(evaluated) ? "true" : "false";
        }
      }
    }
    if (cmValue sofn = sf->GetProperty("VS_SHADER_OBJECT_FILE_NAME")) {
      for (const std::string& config : this->Configurations) {
        toolSettings[config]["ObjectFileOutput"] = *sofn;
      }
    }
  } else if (ext == "jpg"_s || ext == "png"_s) {
    tool = "Image";
  } else if (ext == "resw"_s) {
    tool = "PRIResource";
  } else if (ext == "xml"_s) {
    tool = "XML";
  } else if (ext == "natvis"_s) {
    tool = "Natvis";
  } else if (ext == "settings"_s) {
    settingsLastGenOutput =
      cmsys::SystemTools::GetFilenameName(sf->GetFullPath());
    std::size_t pos = settingsLastGenOutput.find(".settings");
    settingsLastGenOutput.replace(pos, 9, ".Designer.cs");
    settingsGenerator = "SettingsSingleFileGenerator";
    toolHasSettings = true;
  } else if (ext == "vsixmanifest"_s) {
    subType = "Designer";
  }
  if (cmValue c = sf->GetProperty("VS_COPY_TO_OUT_DIR")) {
    tool = "Content";
    copyToOutDir = *c;
    toolHasSettings = true;
  }
  if (sf->GetPropertyAsBool("VS_INCLUDE_IN_VSIX")) {
    includeInVsix = "True";
    tool = "Content";
    toolHasSettings = true;
  }

  // Collect VS_CSHARP_* property values (if some are set)
  std::map<std::string, std::string> sourceFileTags;
  this->GetCSharpSourceProperties(sf, sourceFileTags);

  if (this->NsightTegra) {
    // Nsight Tegra needs specific file types to check up-to-dateness.
    std::string name = cmSystemTools::LowerCase(sf->GetLocation().GetName());
    if (name == "androidmanifest.xml"_s || name == "build.xml"_s ||
        name == "proguard.cfg"_s || name == "proguard-project.txt"_s ||
        ext == "properties"_s) {
      tool = "AndroidBuild";
    } else if (ext == "java"_s) {
      tool = "JCompile";
    } else if (ext == "asm"_s || ext == "s"_s) {
      tool = "ClCompile";
    }
  }

  cmValue toolOverride = sf->GetProperty("VS_TOOL_OVERRIDE");
  if (cmNonempty(toolOverride)) {
    tool = toolOverride->c_str();
  }

  std::string deployContent;
  std::string deployLocation;
  if (this->GlobalGenerator->TargetsWindowsPhone() ||
      this->GlobalGenerator->TargetsWindowsStore()) {
    cmValue content = sf->GetProperty("VS_DEPLOYMENT_CONTENT");
    if (cmNonempty(content)) {
      toolHasSettings = true;
      deployContent = *content;

      cmValue location = sf->GetProperty("VS_DEPLOYMENT_LOCATION");
      if (cmNonempty(location)) {
        deployLocation = *location;
      }
    }
  }

  if (ParsedToolTargetSettings.find(tool) == ParsedToolTargetSettings.end()) {
    cmValue toolTargetProperty = this->GeneratorTarget->Target->GetProperty(
      cmStrCat("VS_SOURCE_SETTINGS_", tool));
    ConfigToSettings toolTargetSettings;
    if (toolTargetProperty) {
      ParseSettingsProperty(*toolTargetProperty, toolTargetSettings);
    }

    ParsedToolTargetSettings[tool] = toolTargetSettings;
  }

  for (const auto& configToSetting : ParsedToolTargetSettings[tool]) {
    for (const auto& setting : configToSetting.second) {
      toolSettings[configToSetting.first][setting.first] = setting.second;
    }
  }

  if (!toolSettings.empty()) {
    toolHasSettings = true;
  }

  Elem e2(e1, tool);
  this->WriteSource(e2, sf);
  if (toolHasSettings) {
    e2.SetHasElements();

    this->FinishWritingSource(e2, toolSettings);

    if (!deployContent.empty()) {
      cmGeneratorExpression ge(*this->LocalGenerator->GetCMakeInstance());
      std::unique_ptr<cmCompiledGeneratorExpression> cge =
        ge.Parse(deployContent);
      // Deployment location cannot be set on a configuration basis
      if (!deployLocation.empty()) {
        e2.Element("Link",
                   cmStrCat(deployLocation, "\\%(FileName)%(Extension)"));
      }
      for (auto& config : this->Configurations) {
        if (cge->Evaluate(this->LocalGenerator, config) == "1"_s) {
          e2.WritePlatformConfigTag(
            "DeploymentContent",
            cmStrCat("'$(Configuration)|$(Platform)'=='", config, '|',
                     this->Platform, '\''),
            "true");
        } else {
          e2.WritePlatformConfigTag(
            "ExcludedFromBuild",
            cmStrCat("'$(Configuration)|$(Platform)'=='", config, '|',
                     this->Platform, '\''),
            "true");
        }
      }
    }

    if (!settingsGenerator.empty()) {
      e2.Element("Generator", settingsGenerator);
    }
    if (!settingsLastGenOutput.empty()) {
      e2.Element("LastGenOutput", settingsLastGenOutput);
    }
    if (!subType.empty()) {
      e2.Element("SubType", subType);
    }
    if (!copyToOutDir.empty()) {
      e2.Element("CopyToOutputDirectory", copyToOutDir);
    }
    if (!includeInVsix.empty()) {
      e2.Element("IncludeInVSIX", includeInVsix);
    }
    // write source file specific tags
    this->WriteCSharpSourceProperties(e2, sourceFileTags);
  }
}

void cmVisualStudio10TargetGenerator::WriteSource(Elem& e2,
                                                  cmSourceFile const* sf)
{
  // Visual Studio tools append relative paths to the current dir, as in:
  //
  //  c:\path\to\current\dir\..\..\..\relative\path\to\source.c
  //
  // and fail if this exceeds the maximum allowed path length.  Our path
  // conversion uses full paths when possible to allow deeper trees.
  // However, CUDA 8.0 msbuild rules fail on absolute paths so for CUDA
  // we must use relative paths.
  bool forceRelative = sf->GetLanguage() == "CUDA"_s;
  std::string sourceFile = this->ConvertPath(sf->GetFullPath(), forceRelative);
  ConvertToWindowsSlash(sourceFile);
  e2.Attribute("Include", sourceFile);

  if (this->ProjectType == VsProjectType::csproj && !this->InSourceBuild) {
    // For out of source projects we have to provide a link (if not specified
    // via property) for every source file (besides .cs files) otherwise they
    // will not be visible in VS at all.
    // First we check if the file is in a source group, then we check if the
    // file path is relative to current source- or binary-dir, otherwise it is
    // added with the plain filename without any path. This means the file will
    // show up at root-level of the csproj (where CMakeLists.txt etc. are).
    std::string link = this->GetCSharpSourceLink(sf);
    if (link.empty()) {
      link = cmsys::SystemTools::GetFilenameName(sf->GetFullPath());
    }
    e2.Element("Link", link);
  }

  ToolSource toolSource = { sf, forceRelative };
  this->Tools[e2.Tag].push_back(toolSource);
}

void cmVisualStudio10TargetGenerator::WriteAllSources(Elem& e0)
{
  if (this->GeneratorTarget->GetType() == cmStateEnums::GLOBAL_TARGET) {
    return;
  }

  const bool haveUnityBuild =
    this->GeneratorTarget->GetPropertyAsBool("UNITY_BUILD");

  if (haveUnityBuild && this->GlobalGenerator->GetSupportsUnityBuilds()) {
    Elem e1(e0, "PropertyGroup");
    e1.Element("EnableUnitySupport", "true");
  }

  Elem e1(e0, "ItemGroup");
  e1.SetHasElements();

  std::vector<size_t> all_configs;
  for (size_t ci = 0; ci < this->Configurations.size(); ++ci) {
    all_configs.push_back(ci);
  }

  std::vector<cmGeneratorTarget::AllConfigSource> const& sources =
    this->GeneratorTarget->GetAllConfigSources();

  cmSourceFile const* srcCMakeLists =
    this->LocalGenerator->CreateVCProjBuildRule();

  for (cmGeneratorTarget::AllConfigSource const& si : sources) {
    if (si.Source == srcCMakeLists) {
      // Skip explicit reference to CMakeLists.txt source.
      continue;
    }

    ConfigToSettings toolSettings;
    for (const auto& config : this->Configurations) {
      toolSettings[config];
    }
    if (cmValue p = si.Source->GetProperty("VS_SETTINGS")) {
      ParseSettingsProperty(*p, toolSettings);
    }

    const char* tool = nullptr;
    switch (si.Kind) {
      case cmGeneratorTarget::SourceKindAppManifest:
        tool = "AppxManifest";
        break;
      case cmGeneratorTarget::SourceKindCertificate:
        tool = "None";
        break;
      case cmGeneratorTarget::SourceKindCustomCommand:
        // Handled elsewhere.
        break;
      case cmGeneratorTarget::SourceKindExternalObject:
        tool = "Object";
        break;
      case cmGeneratorTarget::SourceKindExtra:
        this->WriteExtraSource(e1, si.Source, toolSettings);
        break;
      case cmGeneratorTarget::SourceKindHeader:
        this->WriteHeaderSource(e1, si.Source, toolSettings);
        break;
      case cmGeneratorTarget::SourceKindIDL:
        tool = "Midl";
        break;
      case cmGeneratorTarget::SourceKindManifest:
        // Handled elsewhere.
        break;
      case cmGeneratorTarget::SourceKindModuleDefinition:
        tool = "None";
        break;
      case cmGeneratorTarget::SourceKindCxxModuleSource:
      case cmGeneratorTarget::SourceKindUnityBatched:
      case cmGeneratorTarget::SourceKindObjectSource: {
        const std::string& lang = si.Source->GetLanguage();
        if (lang == "C"_s || lang == "CXX"_s) {
          tool = "ClCompile";
        } else if (lang == "ASM_MARMASM"_s &&
                   this->GlobalGenerator->IsMarmasmEnabled()) {
          tool = "MARMASM";
        } else if (lang == "ASM_MASM"_s &&
                   this->GlobalGenerator->IsMasmEnabled()) {
          tool = "MASM";
        } else if (lang == "ASM_NASM"_s &&
                   this->GlobalGenerator->IsNasmEnabled()) {
          tool = "NASM";
        } else if (lang == "RC"_s) {
          tool = "ResourceCompile";
        } else if (lang == "CSharp"_s) {
          tool = "Compile";
        } else if (lang == "CUDA"_s &&
                   this->GlobalGenerator->IsCudaEnabled()) {
          tool = "CudaCompile";
        } else {
          tool = "None";
        }
      } break;
      case cmGeneratorTarget::SourceKindResx:
        this->ResxObjs.push_back(si.Source);
        break;
      case cmGeneratorTarget::SourceKindXaml:
        this->XamlObjs.push_back(si.Source);
        break;
    }

    std::string config;
    if (!this->Configurations.empty()) {
      config = this->Configurations[si.Configs[0]];
    }
    auto const* fs =
      this->GeneratorTarget->GetFileSetForSource(config, si.Source);
    if (tool) {
      // Compute set of configurations to exclude, if any.
      std::vector<size_t> const& include_configs = si.Configs;
      std::vector<size_t> exclude_configs;
      std::set_difference(all_configs.begin(), all_configs.end(),
                          include_configs.begin(), include_configs.end(),
                          std::back_inserter(exclude_configs));

      Elem e2(e1, tool);
      bool isCSharp = (si.Source->GetLanguage() == "CSharp"_s);
      if (isCSharp && !exclude_configs.empty()) {
        std::stringstream conditions;
        bool firstConditionSet{ false };
        for (const auto& ci : include_configs) {
          if (firstConditionSet) {
            conditions << " Or ";
          }
          conditions << "('$(Configuration)|$(Platform)'=='"
                     << this->Configurations[ci] << '|' << this->Platform
                     << "')";
          firstConditionSet = true;
        }
        e2.Attribute("Condition", conditions.str());
      }
      this->WriteSource(e2, si.Source);

      bool useNativeUnityBuild = false;
      if (haveUnityBuild && this->GlobalGenerator->GetSupportsUnityBuilds()) {
        // Magic value taken from cmGlobalVisualStudioVersionedGenerator.cxx
        static const std::string vs15 = "141";
        std::string toolset =
          this->GlobalGenerator->GetPlatformToolsetString();
        cmSystemTools::ReplaceString(toolset, "v", "");

        if (toolset.empty() ||
            cmSystemTools::VersionCompareGreaterEq(toolset, vs15)) {
          useNativeUnityBuild = true;
        }
      }

      if (haveUnityBuild && strcmp(tool, "ClCompile") == 0 &&
          si.Source->GetProperty("UNITY_SOURCE_FILE")) {
        if (useNativeUnityBuild) {
          e2.Attribute(
            "IncludeInUnityFile",
            si.Source->GetPropertyAsBool("SKIP_UNITY_BUILD_INCLUSION")
              ? "false"
              : "true");
          e2.Attribute("CustomUnityFile", "true");

          std::string unityDir = cmSystemTools::GetFilenamePath(
            *si.Source->GetProperty("UNITY_SOURCE_FILE"));
          e2.Attribute("UnityFilesDirectory", unityDir);
        } else {
          // Visual Studio versions prior to 2017 15.8 do not know about unity
          // builds, thus we exclude the files already part of unity sources.
          if (!si.Source->GetPropertyAsBool("SKIP_UNITY_BUILD_INCLUSION")) {
            exclude_configs = all_configs;
          }
        }
      }

      if (si.Kind == cmGeneratorTarget::SourceKindObjectSource ||
          si.Kind == cmGeneratorTarget::SourceKindUnityBatched) {
        this->OutputSourceSpecificFlags(e2, si.Source);
      } else if (fs && fs->GetType() == "CXX_MODULES"_s) {
        this->GeneratorTarget->Makefile->IssueMessage(
          MessageType::FATAL_ERROR,
          cmStrCat("Target \"", this->GeneratorTarget->GetName(),
                   "\" has source file\n  ", si.Source->GetFullPath(),
                   "\nin a \"FILE_SET TYPE CXX_MODULES\" but it is not "
                   "scheduled for compilation."));
      }
      if (si.Source->GetPropertyAsBool("SKIP_PRECOMPILE_HEADERS")) {
        e2.Element("PrecompiledHeader", "NotUsing");
      }
      if (!isCSharp && !exclude_configs.empty()) {
        this->WriteExcludeFromBuild(e2, exclude_configs);
      }

      this->FinishWritingSource(e2, toolSettings);
    } else if (fs && fs->GetType() == "CXX_MODULES"_s) {
      this->GeneratorTarget->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("Target \"", this->GeneratorTarget->GetName(),
                 "\" has source file\n  ", si.Source->GetFullPath(),
                 "\nin a \"FILE_SET TYPE CXX_MODULES\" but it is not "
                 "scheduled for compilation."));
    }
  }

  if (this->IsMissingFiles) {
    this->WriteMissingFiles(e1);
  }
}

void cmVisualStudio10TargetGenerator::FinishWritingSource(
  Elem& e2, ConfigToSettings const& toolSettings)
{
  std::vector<std::string> writtenSettings;
  for (const auto& configSettings : toolSettings) {
    for (const auto& setting : configSettings.second) {

      if (std::find(writtenSettings.begin(), writtenSettings.end(),
                    setting.first) != writtenSettings.end()) {
        continue;
      }

      if (PropertyIsSameInAllConfigs(toolSettings, setting.first)) {
        e2.Element(setting.first, setting.second);
        writtenSettings.push_back(setting.first);
      } else {
        e2.WritePlatformConfigTag(setting.first,
                                  cmStrCat("'$(Configuration)|$(Platform)'=='",
                                           configSettings.first, '|',
                                           this->Platform, '\''),
                                  setting.second);
      }
    }
  }
}

void cmVisualStudio10TargetGenerator::OutputSourceSpecificFlags(
  Elem& e2, cmSourceFile const* source)
{
  cmSourceFile const& sf = *source;

  std::string objectName;
  if (this->GeneratorTarget->HasExplicitObjectName(&sf)) {
    objectName = this->GeneratorTarget->GetObjectName(&sf);
  }
  std::string flags;
  bool configDependentFlags = false;
  std::string options;
  bool configDependentOptions = false;
  std::string defines;
  bool configDependentDefines = false;
  std::string includes;
  bool configDependentIncludes = false;
  if (cmValue cflags = sf.GetProperty("COMPILE_FLAGS")) {
    configDependentFlags =
      cmGeneratorExpression::Find(*cflags) != std::string::npos;
    flags += *cflags;
  }
  if (cmValue coptions = sf.GetProperty("COMPILE_OPTIONS")) {
    configDependentOptions =
      cmGeneratorExpression::Find(*coptions) != std::string::npos;
    options += *coptions;
  }
  if (cmValue cdefs = sf.GetProperty("COMPILE_DEFINITIONS")) {
    configDependentDefines =
      cmGeneratorExpression::Find(*cdefs) != std::string::npos;
    defines += *cdefs;
  }
  if (cmValue cincludes = sf.GetProperty("INCLUDE_DIRECTORIES")) {
    configDependentIncludes =
      cmGeneratorExpression::Find(*cincludes) != std::string::npos;
    includes += *cincludes;
  }

  // Force language if the file extension does not match.
  // Note that MSVC treats the upper-case '.C' extension as C and not C++.
  std::string const ext = sf.GetExtension();
  std::string const extLang = ext == "C"_s
    ? "C"
    : this->GlobalGenerator->GetLanguageFromExtension(ext.c_str());
  std::string lang = this->LocalGenerator->GetSourceFileLanguage(sf);
  const char* compileAs = nullptr;
  if (lang != extLang) {
    if (lang == "CXX"_s) {
      // force a C++ file type
      compileAs = "CompileAsCpp";
    } else if (lang == "C"_s) {
      // force to c
      compileAs = "CompileAsC";
    }
  }

  bool noWinRT = this->TargetCompileAsWinRT && lang == "C"_s;
  // for the first time we need a new line if there is something
  // produced here.
  if (!objectName.empty()) {
    if (lang == "CUDA"_s) {
      e2.Element("CompileOut", cmStrCat("$(IntDir)/", objectName));
    } else {
      e2.Element("ObjectFileName", cmStrCat("$(IntDir)/", objectName));
    }
  }

  if (lang == "ASM_NASM"_s) {
    if (cmValue objectDeps = sf.GetProperty("OBJECT_DEPENDS")) {
      cmList depends{ *objectDeps };
      for (auto& d : depends) {
        ConvertToWindowsSlash(d);
      }
      e2.Element("AdditionalDependencies", depends.join(";"));
    }
  }

  bool isCppModule = false;

  for (std::string const& config : this->Configurations) {
    this->GeneratorTarget->NeedCxxModuleSupport(lang, config);

    std::string configUpper = cmSystemTools::UpperCase(config);
    std::string configDefines = defines;
    std::string defPropName = cmStrCat("COMPILE_DEFINITIONS_", configUpper);
    if (cmValue ccdefs = sf.GetProperty(defPropName)) {
      if (!configDefines.empty()) {
        configDefines += ';';
      }
      configDependentDefines |=
        cmGeneratorExpression::Find(*ccdefs) != std::string::npos;
      configDefines += *ccdefs;
    }

    bool const shouldScanForModules = lang == "CXX"_s &&
      this->GeneratorTarget->NeedDyndepForSource(lang, config, source);
    auto const* fs =
      this->GeneratorTarget->GetFileSetForSource(config, source);
    const char* compileAsPerConfig = compileAs;
    if (fs && fs->GetType() == "CXX_MODULES"_s) {
      if (lang == "CXX"_s) {
        if (fs->GetType() == "CXX_MODULES"_s) {
          isCppModule = true;
          if (shouldScanForModules &&
              this->GlobalGenerator->IsScanDependenciesSupported()) {
            // ScanSourceForModuleDependencies uses 'cl /scanDependencies' and
            // can distinguish module interface units and internal partitions.
            compileAsPerConfig = "CompileAsCpp";
          } else {
            compileAsPerConfig = "CompileAsCppModule";
          }
        } else {
          compileAsPerConfig = "CompileAsHeaderUnit";
        }
      } else {
        this->Makefile->IssueMessage(
          MessageType::FATAL_ERROR,
          cmStrCat(
            "Target \"", this->GeneratorTarget->Target->GetName(),
            "\" contains the source\n  ", source->GetFullPath(),
            "\nin a file set of type \"", fs->GetType(),
            R"(" but the source is not classified as a "CXX" source.)"));
      }
    }

    // We have pch state in the following situation:
    // 1. We have SKIP_PRECOMPILE_HEADERS == true
    // 2. We are creating the pre-compiled header
    // 3. We are a different language than the linker language AND pch is
    //    enabled.
    std::string const& linkLanguage =
      this->GeneratorTarget->GetLinkerLanguage(config);
    std::string const& pchSource =
      this->GeneratorTarget->GetPchSource(config, lang);
    const bool skipPCH =
      pchSource.empty() || sf.GetPropertyAsBool("SKIP_PRECOMPILE_HEADERS");
    const bool makePCH = (sf.GetFullPath() == pchSource);
    const bool useSharedPCH = !skipPCH && (lang == linkLanguage);
    const bool useDifferentLangPCH = !skipPCH && (lang != linkLanguage);
    const bool useNoPCH = skipPCH && (lang != linkLanguage) &&
      !this->GeneratorTarget->GetPchHeader(config, linkLanguage).empty();
    const bool needsPCHFlags =
      (makePCH || useSharedPCH || useDifferentLangPCH || useNoPCH);

    // if we have flags or defines for this config then
    // use them
    if (!flags.empty() || !options.empty() || !configDefines.empty() ||
        !includes.empty() || compileAsPerConfig || noWinRT ||
        !options.empty() || needsPCHFlags ||
        (shouldScanForModules !=
         this->ScanSourceForModuleDependencies[config])) {
      cmGlobalVisualStudio10Generator* gg = this->GlobalGenerator;
      cmIDEFlagTable const* flagtable = nullptr;
      const std::string& srclang = source->GetLanguage();
      if (srclang == "C"_s || srclang == "CXX"_s) {
        flagtable = gg->GetClFlagTable();
      } else if (srclang == "ASM_MARMASM"_s &&
                 this->GlobalGenerator->IsMarmasmEnabled()) {
        flagtable = gg->GetMarmasmFlagTable();
      } else if (srclang == "ASM_MASM"_s &&
                 this->GlobalGenerator->IsMasmEnabled()) {
        flagtable = gg->GetMasmFlagTable();
      } else if (lang == "ASM_NASM"_s &&
                 this->GlobalGenerator->IsNasmEnabled()) {
        flagtable = gg->GetNasmFlagTable();
      } else if (srclang == "RC"_s) {
        flagtable = gg->GetRcFlagTable();
      } else if (srclang == "CSharp"_s) {
        flagtable = gg->GetCSharpFlagTable();
      }
      cmGeneratorExpressionInterpreter genexInterpreter(
        this->LocalGenerator, config, this->GeneratorTarget, lang);
      cmVS10GeneratorOptions clOptions(
        this->LocalGenerator, cmVisualStudioGeneratorOptions::Compiler,
        flagtable, this);
      if (compileAsPerConfig) {
        clOptions.AddFlag("CompileAs", compileAsPerConfig);
      }
      if (shouldScanForModules !=
          this->ScanSourceForModuleDependencies[config]) {
        clOptions.AddFlag("ScanSourceForModuleDependencies",
                          shouldScanForModules ? "true" : "false");
      }
      if (noWinRT) {
        clOptions.AddFlag("CompileAsWinRT", "false");
      }
      if (configDependentFlags) {
        clOptions.Parse(genexInterpreter.Evaluate(flags, "COMPILE_FLAGS"));
      } else {
        clOptions.Parse(flags);
      }

      if (needsPCHFlags) {
        // Add precompile headers compile options.
        if (makePCH) {
          clOptions.AddFlag("PrecompiledHeader", "Create");
          std::string pchHeader =
            this->GeneratorTarget->GetPchHeader(config, lang);
          clOptions.AddFlag("PrecompiledHeaderFile", pchHeader);
          std::string pchFile =
            this->GeneratorTarget->GetPchFile(config, lang);
          clOptions.AddFlag("PrecompiledHeaderOutputFile", pchFile);
          clOptions.AddFlag("ForcedIncludeFiles", pchHeader);
        } else if (useNoPCH) {
          clOptions.AddFlag("PrecompiledHeader", "NotUsing");
        } else if (useSharedPCH) {
          std::string pchHeader =
            this->GeneratorTarget->GetPchHeader(config, lang);
          clOptions.AddFlag("ForcedIncludeFiles", pchHeader);
        } else if (useDifferentLangPCH) {
          clOptions.AddFlag("PrecompiledHeader", "Use");
          std::string pchHeader =
            this->GeneratorTarget->GetPchHeader(config, lang);
          clOptions.AddFlag("PrecompiledHeaderFile", pchHeader);
          std::string pchFile =
            this->GeneratorTarget->GetPchFile(config, lang);
          clOptions.AddFlag("PrecompiledHeaderOutputFile", pchFile);
          clOptions.AddFlag("ForcedIncludeFiles", pchHeader);
        }
      }

      if (!options.empty()) {
        std::string expandedOptions;
        if (configDependentOptions) {
          this->LocalGenerator->AppendCompileOptions(
            expandedOptions,
            genexInterpreter.Evaluate(options, "COMPILE_OPTIONS"));
        } else {
          this->LocalGenerator->AppendCompileOptions(expandedOptions, options);
        }
        clOptions.Parse(expandedOptions);
      }
      if (clOptions.HasFlag("DisableSpecificWarnings")) {
        clOptions.AppendFlag("DisableSpecificWarnings",
                             "%(DisableSpecificWarnings)");
      }
      if (clOptions.HasFlag("ForcedIncludeFiles")) {
        clOptions.AppendFlag("ForcedIncludeFiles", "%(ForcedIncludeFiles)");
      }
      if (configDependentDefines) {
        clOptions.AddDefines(
          genexInterpreter.Evaluate(configDefines, "COMPILE_DEFINITIONS"));
      } else {
        clOptions.AddDefines(configDefines);
      }
      std::vector<std::string> includeList;
      if (configDependentIncludes) {
        this->LocalGenerator->AppendIncludeDirectories(
          includeList,
          genexInterpreter.Evaluate(includes, "INCLUDE_DIRECTORIES"), *source);
      } else {
        this->LocalGenerator->AppendIncludeDirectories(includeList, includes,
                                                       *source);
      }
      clOptions.AddIncludes(includeList);
      clOptions.SetConfiguration(config);
      OptionsHelper oh(clOptions, e2);
      oh.PrependInheritedString("AdditionalOptions");
      oh.OutputAdditionalIncludeDirectories(lang);
      oh.OutputFlagMap();
      oh.OutputPreprocessorDefinitions(lang);
    }
  }

  if (isCppModule && !objectName.empty()) {
    std::string baseName = cmStrCat("$(IntDir)/", objectName);
    cmStripSuffixIfExists(baseName, ".obj");
    e2.Element("ModuleOutputFile", cmStrCat(baseName, ".ifc"));
    e2.Element("ModuleDependenciesFile", cmStrCat(baseName, ".module.json"));
  }

  if (this->IsXamlSource(source->GetFullPath())) {
    const std::string& fileName = source->GetFullPath();
    e2.Element("DependentUpon",
               fileName.substr(0, fileName.find_last_of('.')));
  }
  if (this->ProjectType == VsProjectType::csproj) {
    using CsPropMap = std::map<std::string, std::string>;
    CsPropMap sourceFileTags;
    this->GetCSharpSourceProperties(&sf, sourceFileTags);
    // write source file specific tags
    if (!sourceFileTags.empty()) {
      this->WriteCSharpSourceProperties(e2, sourceFileTags);
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteExcludeFromBuild(
  Elem& e2, std::vector<size_t> const& exclude_configs)
{
  for (size_t ci : exclude_configs) {
    e2.WritePlatformConfigTag("ExcludedFromBuild",
                              cmStrCat("'$(Configuration)|$(Platform)'=='",
                                       this->Configurations[ci], '|',
                                       this->Platform, '\''),
                              "true");
  }
}

void cmVisualStudio10TargetGenerator::WritePathAndIncrementalLinkOptions(
  Elem& e0)
{
  cmStateEnums::TargetType ttype = this->GeneratorTarget->GetType();
  if (ttype > cmStateEnums::INTERFACE_LIBRARY) {
    return;
  }
  if (this->ProjectType == VsProjectType::csproj) {
    return;
  }

  Elem e1(e0, "PropertyGroup");
  e1.Element("_ProjectFileVersion", "10.0.20506.1");
  for (std::string const& config : this->Configurations) {
    const std::string cond = this->CalcCondition(config);

    if (ttype >= cmStateEnums::UTILITY) {
      e1.WritePlatformConfigTag(
        "IntDir", cond, R"($(Platform)\$(Configuration)\$(ProjectName)\)");
    } else {
      if (ttype == cmStateEnums::SHARED_LIBRARY ||
          ttype == cmStateEnums::MODULE_LIBRARY ||
          ttype == cmStateEnums::EXECUTABLE) {
        auto linker = this->GeneratorTarget->GetLinkerTool(config);
        if (!linker.empty()) {
          ConvertToWindowsSlash(linker);
          e1.WritePlatformConfigTag("LinkToolExe", cond, linker);
        }
      }

      std::string intermediateDir = cmStrCat(
        this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget), '/',
        config, '/');
      std::string outDir;
      std::string targetNameFull;
      if (ttype == cmStateEnums::OBJECT_LIBRARY) {
        outDir = intermediateDir;
        targetNameFull = cmStrCat(this->GeneratorTarget->GetName(), ".lib");
      } else {
        outDir = cmStrCat(this->GeneratorTarget->GetDirectory(config), '/');
        targetNameFull = this->GeneratorTarget->GetFullName(config);
      }
      ConvertToWindowsSlash(intermediateDir);
      ConvertToWindowsSlash(outDir);

      e1.WritePlatformConfigTag("OutDir", cond, outDir);

      e1.WritePlatformConfigTag("IntDir", cond, intermediateDir);

      if (cmValue sdkExecutableDirectories = this->Makefile->GetDefinition(
            "CMAKE_VS_SDK_EXECUTABLE_DIRECTORIES")) {
        e1.WritePlatformConfigTag("ExecutablePath", cond,
                                  *sdkExecutableDirectories);
      }

      if (cmValue sdkIncludeDirectories = this->Makefile->GetDefinition(
            "CMAKE_VS_SDK_INCLUDE_DIRECTORIES")) {
        e1.WritePlatformConfigTag("IncludePath", cond, *sdkIncludeDirectories);
      }

      if (cmValue sdkReferenceDirectories = this->Makefile->GetDefinition(
            "CMAKE_VS_SDK_REFERENCE_DIRECTORIES")) {
        e1.WritePlatformConfigTag("ReferencePath", cond,
                                  *sdkReferenceDirectories);
      }

      if (cmValue sdkLibraryDirectories = this->Makefile->GetDefinition(
            "CMAKE_VS_SDK_LIBRARY_DIRECTORIES")) {
        e1.WritePlatformConfigTag("LibraryPath", cond, *sdkLibraryDirectories);
      }

      if (cmValue sdkLibraryWDirectories = this->Makefile->GetDefinition(
            "CMAKE_VS_SDK_LIBRARY_WINRT_DIRECTORIES")) {
        e1.WritePlatformConfigTag("LibraryWPath", cond,
                                  *sdkLibraryWDirectories);
      }

      if (cmValue sdkSourceDirectories =
            this->Makefile->GetDefinition("CMAKE_VS_SDK_SOURCE_DIRECTORIES")) {
        e1.WritePlatformConfigTag("SourcePath", cond, *sdkSourceDirectories);
      }

      if (cmValue sdkExcludeDirectories = this->Makefile->GetDefinition(
            "CMAKE_VS_SDK_EXCLUDE_DIRECTORIES")) {
        e1.WritePlatformConfigTag("ExcludePath", cond, *sdkExcludeDirectories);
      }

      std::string name =
        cmSystemTools::GetFilenameWithoutLastExtension(targetNameFull);
      e1.WritePlatformConfigTag("TargetName", cond, name);

      std::string ext =
        cmSystemTools::GetFilenameLastExtension(targetNameFull);
      if (ext.empty()) {
        // An empty TargetExt causes a default extension to be used.
        // A single "." appears to be treated as an empty extension.
        ext = ".";
      }
      e1.WritePlatformConfigTag("TargetExt", cond, ext);

      this->OutputLinkIncremental(e1, config);
    }

    if (ttype <= cmStateEnums::UTILITY) {
      if (cmValue workingDir = this->GeneratorTarget->GetProperty(
            "VS_DEBUGGER_WORKING_DIRECTORY")) {
        std::string genWorkingDir = cmGeneratorExpression::Evaluate(
          *workingDir, this->LocalGenerator, config);
        e1.WritePlatformConfigTag("LocalDebuggerWorkingDirectory", cond,
                                  genWorkingDir);
      }

      if (cmValue environment =
            this->GeneratorTarget->GetProperty("VS_DEBUGGER_ENVIRONMENT")) {
        std::string genEnvironment = cmGeneratorExpression::Evaluate(
          *environment, this->LocalGenerator, config);
        e1.WritePlatformConfigTag("LocalDebuggerEnvironment", cond,
                                  genEnvironment);
      }

      if (cmValue debuggerCommand =
            this->GeneratorTarget->GetProperty("VS_DEBUGGER_COMMAND")) {
        std::string genDebuggerCommand = cmGeneratorExpression::Evaluate(
          *debuggerCommand, this->LocalGenerator, config);
        e1.WritePlatformConfigTag("LocalDebuggerCommand", cond,
                                  genDebuggerCommand);
      }

      if (cmValue commandArguments = this->GeneratorTarget->GetProperty(
            "VS_DEBUGGER_COMMAND_ARGUMENTS")) {
        std::string genCommandArguments = cmGeneratorExpression::Evaluate(
          *commandArguments, this->LocalGenerator, config);
        e1.WritePlatformConfigTag("LocalDebuggerCommandArguments", cond,
                                  genCommandArguments);
      }
    }
  }
}

void cmVisualStudio10TargetGenerator::WritePublicProjectContentOptions(
  Elem& e0)
{
  cmStateEnums::TargetType ttype = this->GeneratorTarget->GetType();
  if (ttype != cmStateEnums::SHARED_LIBRARY) {
    return;
  }
  if (this->ProjectType != VsProjectType::vcxproj) {
    return;
  }

  Elem e1(e0, "PropertyGroup");
  for (std::string const& config : this->Configurations) {
    if (this->GeneratorTarget->HaveCxx20ModuleSources() &&
        this->GeneratorTarget->HaveCxxModuleSupport(config) ==
          cmGeneratorTarget::Cxx20SupportLevel::Supported) {
      const std::string cond = this->CalcCondition(config);
      // For DLL projects, we export all BMIs for now
      e1.WritePlatformConfigTag("AllProjectBMIsArePublic", cond, "true");
    }
  }
}

void cmVisualStudio10TargetGenerator::OutputLinkIncremental(
  Elem& e1, std::string const& configName)
{
  if (!this->MSTools) {
    return;
  }
  if (this->ProjectType == VsProjectType::csproj) {
    return;
  }
  // static libraries and things greater than modules do not need
  // to set this option
  if (this->GeneratorTarget->GetType() == cmStateEnums::STATIC_LIBRARY ||
      this->GeneratorTarget->GetType() > cmStateEnums::MODULE_LIBRARY) {
    return;
  }
  Options& linkOptions = *(this->LinkOptions[configName]);
  const std::string cond = this->CalcCondition(configName);

  if (this->IPOEnabledConfigurations.count(configName) == 0) {
    const char* incremental = linkOptions.GetFlag("LinkIncremental");
    e1.WritePlatformConfigTag("LinkIncremental", cond,
                              (incremental ? incremental : "true"));
  }
  linkOptions.RemoveFlag("LinkIncremental");

  const char* manifest = linkOptions.GetFlag("GenerateManifest");
  e1.WritePlatformConfigTag("GenerateManifest", cond,
                            (manifest ? manifest : "true"));
  linkOptions.RemoveFlag("GenerateManifest");

  // Some link options belong here.  Use them now and remove them so that
  // WriteLinkOptions does not use them.
  static const std::vector<std::string> flags{ "LinkDelaySign",
                                               "LinkKeyFile" };
  for (const std::string& flag : flags) {
    if (const char* value = linkOptions.GetFlag(flag)) {
      e1.WritePlatformConfigTag(flag, cond, value);
      linkOptions.RemoveFlag(flag);
    }
  }
}

std::vector<std::string> cmVisualStudio10TargetGenerator::GetIncludes(
  std::string const& config, std::string const& lang) const
{
  std::vector<std::string> includes;
  this->LocalGenerator->GetIncludeDirectories(includes, this->GeneratorTarget,
                                              lang, config);
  for (std::string& i : includes) {
    ConvertToWindowsSlash(i);
  }
  return includes;
}

std::string cmVisualStudio10TargetGenerator::GetTargetOutputName() const
{
  std::string config;
  if (!this->Configurations.empty()) {
    config = this->Configurations[0];
  }
  const auto& nameComponents =
    this->GeneratorTarget->GetFullNameComponents(config);
  return cmStrCat(nameComponents.prefix, nameComponents.base);
}

bool cmVisualStudio10TargetGenerator::ComputeClOptions()
{
  return std::all_of(
    this->Configurations.begin(), this->Configurations.end(),
    [this](std::string const& c) { return this->ComputeClOptions(c); });
}

bool cmVisualStudio10TargetGenerator::ComputeClOptions(
  std::string const& configName)
{
  // much of this was copied from here:
  // copied from cmLocalVisualStudio7Generator.cxx 805
  // TODO: Integrate code below with cmLocalVisualStudio7Generator.

  cmGlobalVisualStudio10Generator* gg = this->GlobalGenerator;
  std::unique_ptr<Options> pOptions;
  switch (this->ProjectType) {
    case VsProjectType::vcxproj:
      pOptions = cm::make_unique<Options>(
        this->LocalGenerator, Options::Compiler, gg->GetClFlagTable());
      break;
    case VsProjectType::csproj:
      pOptions =
        cm::make_unique<Options>(this->LocalGenerator, Options::CSharpCompiler,
                                 gg->GetCSharpFlagTable());
      break;
    default:
      break;
  }
  Options& clOptions = *pOptions;

  std::string flags;
  const std::string& linkLanguage =
    this->GeneratorTarget->GetLinkerLanguage(configName);
  if (linkLanguage.empty()) {
    cmSystemTools::Error(cmStrCat(
      "CMake can not determine linker language for target: ", this->Name));
    return false;
  }

  // Choose a language whose flags to use for ClCompile.
  static const char* clLangs[] = { "CXX", "C", "Fortran" };
  std::string langForClCompile;
  if (this->ProjectType == VsProjectType::csproj) {
    langForClCompile = "CSharp";
  } else if (cm::contains(clLangs, linkLanguage)) {
    langForClCompile = linkLanguage;
  } else {
    std::set<std::string> languages;
    this->GeneratorTarget->GetLanguages(languages, configName);
    for (const char* l : clLangs) {
      if (languages.count(l)) {
        langForClCompile = l;
        break;
      }
    }
  }
  this->LangForClCompile = langForClCompile;
  if (!langForClCompile.empty()) {
    this->LocalGenerator->AddLanguageFlags(flags, this->GeneratorTarget,
                                           cmBuildStep::Compile,
                                           langForClCompile, configName);
    this->LocalGenerator->AddCompileOptions(flags, this->GeneratorTarget,
                                            langForClCompile, configName);
  }

  // Put the IPO enabled configurations into a set.
  if (this->GeneratorTarget->IsIPOEnabled(linkLanguage, configName)) {
    this->IPOEnabledConfigurations.insert(configName);
  }

  // Check if ASan is enabled.
  if (flags.find("/fsanitize=address") != std::string::npos ||
      flags.find("-fsanitize=address") != std::string::npos) {
    this->ASanEnabledConfigurations.insert(configName);
  }

  // Check if (lib)Fuzzer is enabled.
  if (flags.find("/fsanitize=fuzzer") != std::string::npos ||
      flags.find("-fsanitize=fuzzer") != std::string::npos) {
    this->FuzzerEnabledConfigurations.insert(configName);
  }

  // Precompile Headers
  std::string pchHeader =
    this->GeneratorTarget->GetPchHeader(configName, linkLanguage);
  if (this->MSTools && VsProjectType::vcxproj == this->ProjectType &&
      pchHeader.empty()) {
    clOptions.AddFlag("PrecompiledHeader", "NotUsing");
  } else if (this->MSTools && VsProjectType::vcxproj == this->ProjectType &&
             !pchHeader.empty()) {
    clOptions.AddFlag("PrecompiledHeader", "Use");
    clOptions.AddFlag("PrecompiledHeaderFile", pchHeader);
    std::string pchFile =
      this->GeneratorTarget->GetPchFile(configName, linkLanguage);
    clOptions.AddFlag("PrecompiledHeaderOutputFile", pchFile);
  }

  // Get preprocessor definitions for this directory.
  std::string defineFlags = this->Makefile->GetDefineFlags();
  if (this->MSTools) {
    if (this->ProjectType == VsProjectType::vcxproj) {
      clOptions.FixExceptionHandlingDefault();
      if (this->GlobalGenerator->GetVersion() >=
          cmGlobalVisualStudioGenerator::VSVersion::VS15) {
        // Toolsets that come with VS 2017 may now enable UseFullPaths
        // by default and there is no negative /FC option that projects
        // can use to switch it back.  Older toolsets disable this by
        // default anyway so this will not hurt them.  If the project
        // is using an explicit /FC option then parsing flags will
        // replace this setting with "true" below.
        clOptions.AddFlag("UseFullPaths", "false");
      }
      clOptions.AddFlag("AssemblerListingLocation", "$(IntDir)");
    }
  }

  // check for managed C++ assembly compiler flag. This overrides any
  // /clr* compiler flags which may be defined in the flags variable(s).
  if (this->ProjectType != VsProjectType::csproj) {
    // Warn if /clr was added manually. This should not be done
    // anymore, because cmGeneratorTarget may not be aware that the
    // target uses C++/CLI.
    if (flags.find("/clr") != std::string::npos ||
        flags.find("-clr") != std::string::npos ||
        defineFlags.find("/clr") != std::string::npos ||
        defineFlags.find("-clr") != std::string::npos) {
      if (configName == this->Configurations[0]) {
        std::string message =
          cmStrCat("For the target \"", this->GeneratorTarget->GetName(),
                   "\" the /clr compiler flag was added manually. ",
                   "Set usage of C++/CLI by setting COMMON_LANGUAGE_RUNTIME "
                   "target property.");
        this->Makefile->IssueMessage(MessageType::WARNING, message);
      }
    }
    if (cmValue clr =
          this->GeneratorTarget->GetProperty("COMMON_LANGUAGE_RUNTIME")) {
      std::string clrString = *clr;
      if (!clrString.empty()) {
        clrString = cmStrCat(':', clrString);
      }
      flags += cmStrCat(" /clr", clrString);
    }
  }

  // Get includes for this target
  if (!this->LangForClCompile.empty()) {
    auto includeList = this->GetIncludes(configName, this->LangForClCompile);

    auto sysIncludeFlag = this->Makefile->GetDefinition(
      cmStrCat("CMAKE_INCLUDE_SYSTEM_FLAG_", this->LangForClCompile));

    if (sysIncludeFlag) {
      bool gotOneSys = false;
      for (auto i : includeList) {
        cmSystemTools::ConvertToUnixSlashes(i);
        if (this->GeneratorTarget->IsSystemIncludeDirectory(
              i, configName, this->LangForClCompile)) {
          auto flag = cmTrimWhitespace(*sysIncludeFlag);
          if (this->MSTools) {
            cmSystemTools::ReplaceString(flag, "-external:I", "/external:I");
          }
          clOptions.AppendFlagString("AdditionalOptions",
                                     cmStrCat(flag, " \"", i, '"'));
          gotOneSys = true;
        } else {
          clOptions.AddInclude(i);
        }
      }

      if (gotOneSys) {
        if (auto sysIncludeFlagWarning = this->Makefile->GetDefinition(
              cmStrCat("_CMAKE_INCLUDE_SYSTEM_FLAG_", this->LangForClCompile,
                       "_WARNING"))) {
          flags = cmStrCat(flags, ' ', *sysIncludeFlagWarning);
        }
      }
    } else {
      clOptions.AddIncludes(includeList);
    }
  }

  clOptions.Parse(flags);
  clOptions.Parse(defineFlags);
  std::vector<std::string> targetDefines;
  switch (this->ProjectType) {
    case VsProjectType::vcxproj:
      if (!langForClCompile.empty()) {
        this->GeneratorTarget->GetCompileDefinitions(targetDefines, configName,
                                                     langForClCompile);
      }
      break;
    case VsProjectType::csproj:
      this->GeneratorTarget->GetCompileDefinitions(targetDefines, configName,
                                                   "CSharp");
      cm::erase_if(targetDefines, [](std::string const& def) {
        return def.find('=') != std::string::npos;
      });
      break;
    default:
      break;
  }
  clOptions.AddDefines(targetDefines);

  if (this->ProjectType == VsProjectType::csproj) {
    clOptions.AppendFlag("DefineConstants", targetDefines);
  }

  if (this->MSTools) {
    clOptions.SetVerboseMakefile(
      this->Makefile->IsOn("CMAKE_VERBOSE_MAKEFILE"));
  }

  // Add C-specific flags expressible in a ClCompile meant for C++.
  if (langForClCompile == "CXX"_s) {
    std::set<std::string> languages;
    this->GeneratorTarget->GetLanguages(languages, configName);
    if (languages.count("C")) {
      std::string flagsC;
      this->LocalGenerator->AddLanguageFlags(
        flagsC, this->GeneratorTarget, cmBuildStep::Compile, "C", configName);
      this->LocalGenerator->AddCompileOptions(flagsC, this->GeneratorTarget,
                                              "C", configName);
      Options optC(this->LocalGenerator, Options::Compiler,
                   gg->GetClFlagTable());
      optC.Parse(flagsC);
      if (const char* stdC = optC.GetFlag("LanguageStandard_C")) {
        clOptions.AddFlag("LanguageStandard_C", stdC);
      }
    }
  }

  // Add a definition for the configuration name.
  std::string configDefine = cmStrCat("CMAKE_INTDIR=\"", configName, '"');
  clOptions.AddDefine(configDefine);
  if (const std::string* exportMacro =
        this->GeneratorTarget->GetExportMacro()) {
    clOptions.AddDefine(*exportMacro);
  }

  if (this->MSTools) {
    // If we have the VS_WINRT_COMPONENT set then force Compile as WinRT
    if (this->GeneratorTarget->GetPropertyAsBool("VS_WINRT_COMPONENT")) {
      clOptions.AddFlag("CompileAsWinRT", "true");
      // For WinRT components, add the _WINRT_DLL define to produce a lib
      if (this->GeneratorTarget->GetType() == cmStateEnums::SHARED_LIBRARY ||
          this->GeneratorTarget->GetType() == cmStateEnums::MODULE_LIBRARY) {
        clOptions.AddDefine("_WINRT_DLL");
      }
    } else if (this->GlobalGenerator->TargetsWindowsStore() ||
               this->GlobalGenerator->TargetsWindowsPhone() ||
               this->Makefile->IsOn("CMAKE_VS_WINRT_BY_DEFAULT")) {
      if (!clOptions.IsWinRt()) {
        clOptions.AddFlag("CompileAsWinRT", "false");
      }
    }
    if (const char* winRT = clOptions.GetFlag("CompileAsWinRT")) {
      if (cmIsOn(winRT)) {
        this->TargetCompileAsWinRT = true;
      }
    }
  }

  if (this->ProjectType != VsProjectType::csproj &&
      (clOptions.IsManaged() || clOptions.HasFlag("CLRSupport"))) {
    this->Managed = true;
    std::string managedType = clOptions.HasFlag("CompileAsManaged")
      ? clOptions.GetFlag("CompileAsManaged")
      : "Mixed";
    if (managedType == "Safe"_s || managedType == "Pure"_s) {
      // force empty calling convention if safe clr is used
      clOptions.AddFlag("CallingConvention", "");
    }
    // The default values of these flags are incompatible to
    // managed assemblies. We have to force valid values if
    // the target is a managed C++ target.
    clOptions.AddFlag("ExceptionHandling", "Async");
    clOptions.AddFlag("BasicRuntimeChecks", "Default");
  }
  if (this->ProjectType == VsProjectType::csproj) {
    // /nowin32manifest overrides /win32manifest: parameter
    if (clOptions.HasFlag("NoWin32Manifest")) {
      clOptions.RemoveFlag("ApplicationManifest");
    }
  }

  if (const char* s = clOptions.GetFlag("SpectreMitigation")) {
    this->SpectreMitigation[configName] = s;
    clOptions.RemoveFlag("SpectreMitigation");
  }

  // Remove any target-wide -TC or -TP flag added by the project.
  // Such flags are unnecessary and break our model of language selection.
  if (langForClCompile == "C"_s || langForClCompile == "CXX"_s) {
    clOptions.RemoveFlag("CompileAs");
  }

  this->ClOptions[configName] = std::move(pOptions);
  return true;
}

void cmVisualStudio10TargetGenerator::WriteClOptions(
  Elem& e1, std::string const& configName)
{
  Options& clOptions = *(this->ClOptions[configName]);
  if (this->ProjectType == VsProjectType::csproj) {
    return;
  }
  Elem e2(e1, "ClCompile");
  OptionsHelper oh(clOptions, e2);
  oh.PrependInheritedString("AdditionalOptions");
  oh.OutputAdditionalIncludeDirectories(this->LangForClCompile);
  oh.OutputFlagMap();
  oh.OutputPreprocessorDefinitions(this->LangForClCompile);

  if (this->NsightTegra) {
    if (cmValue processMax =
          this->GeneratorTarget->GetProperty("ANDROID_PROCESS_MAX")) {
      e2.Element("ProcessMax", *processMax);
    }
  }

  if (this->Android) {
    e2.Element("ObjectFileName", "$(IntDir)%(filename).o");
  } else if (this->MSTools) {
    cmsys::RegularExpression clangToolset("v[0-9]+_clang_.*");
    const char* toolset = this->GlobalGenerator->GetPlatformToolset();
    cmValue noCompileBatching =
      this->GeneratorTarget->GetProperty("VS_NO_COMPILE_BATCHING");
    if (noCompileBatching.IsOn() || (toolset && clangToolset.find(toolset))) {
      e2.Element("ObjectFileName", "$(IntDir)%(filename).obj");
    } else {
      e2.Element("ObjectFileName", "$(IntDir)");
    }

    // If not in debug mode, write the DebugInformationFormat field
    // without value so PDBs don't get generated uselessly. Each tag
    // goes on its own line because Visual Studio corrects it this
    // way when saving the project after CMake generates it.
    if (!clOptions.IsDebug()) {
      Elem e3(e2, "DebugInformationFormat");
      e3.SetHasElements();
    }

    // Specify the compiler program database file if configured.
    std::string pdb = this->GeneratorTarget->GetCompilePDBPath(configName);
    if (!pdb.empty()) {
      if (this->GlobalGenerator->IsCudaEnabled()) {
        // CUDA does not quote paths with spaces correctly when forwarding
        // this to the host compiler.  Use a relative path to avoid spaces.
        // FIXME: We can likely do this even when CUDA is not involved,
        // but for now we will make a minimal change.
        pdb = this->ConvertPath(pdb, true);
      }
      ConvertToWindowsSlash(pdb);
      e2.Element("ProgramDataBaseFileName", pdb);
    }

    // add AdditionalUsingDirectories
    if (this->AdditionalUsingDirectories.count(configName) > 0) {
      std::string dirs;
      for (auto const& u : this->AdditionalUsingDirectories[configName]) {
        if (!dirs.empty()) {
          dirs.append(";");
        }
        dirs.append(u);
      }
      e2.Element("AdditionalUsingDirectories", dirs);
    }
  }

  e2.Element("ScanSourceForModuleDependencies",
             this->ScanSourceForModuleDependencies[configName] ? "true"
                                                               : "false");
}

bool cmVisualStudio10TargetGenerator::ComputeRcOptions()
{
  return std::all_of(
    this->Configurations.begin(), this->Configurations.end(),
    [this](std::string const& c) { return this->ComputeRcOptions(c); });
}

bool cmVisualStudio10TargetGenerator::ComputeRcOptions(
  std::string const& configName)
{
  cmGlobalVisualStudio10Generator* gg = this->GlobalGenerator;
  auto pOptions = cm::make_unique<Options>(
    this->LocalGenerator, Options::ResourceCompiler, gg->GetRcFlagTable());
  Options& rcOptions = *pOptions;

  std::string CONFIG = cmSystemTools::UpperCase(configName);
  std::string rcConfigFlagsVar = cmStrCat("CMAKE_RC_FLAGS_", CONFIG);
  std::string flags =
    cmStrCat(this->Makefile->GetSafeDefinition("CMAKE_RC_FLAGS"), ' ',
             this->Makefile->GetSafeDefinition(rcConfigFlagsVar));

  rcOptions.Parse(flags);

  // For historical reasons, add the C preprocessor defines to RC.
  Options& clOptions = *(this->ClOptions[configName]);
  rcOptions.AddDefines(clOptions.GetDefines());

  // Get includes for this target
  rcOptions.AddIncludes(this->GetIncludes(configName, "RC"));

  this->RcOptions[configName] = std::move(pOptions);
  return true;
}

void cmVisualStudio10TargetGenerator::WriteRCOptions(
  Elem& e1, std::string const& configName)
{
  if (!this->MSTools) {
    return;
  }
  Elem e2(e1, "ResourceCompile");

  OptionsHelper rcOptions(*(this->RcOptions[configName]), e2);
  rcOptions.OutputPreprocessorDefinitions("RC");
  rcOptions.OutputAdditionalIncludeDirectories("RC");
  rcOptions.PrependInheritedString("AdditionalOptions");
  rcOptions.OutputFlagMap();
}

bool cmVisualStudio10TargetGenerator::ComputeCudaOptions()
{
  if (!this->GlobalGenerator->IsCudaEnabled()) {
    return true;
  }
  return std::all_of(this->Configurations.begin(), this->Configurations.end(),
                     [this](std::string const& c) {
                       return !this->GeneratorTarget->IsLanguageUsed("CUDA",
                                                                     c) ||
                         this->ComputeCudaOptions(c);
                     });
}

bool cmVisualStudio10TargetGenerator::ComputeCudaOptions(
  std::string const& configName)
{
  cmGlobalVisualStudio10Generator* gg = this->GlobalGenerator;
  auto pOptions = cm::make_unique<Options>(
    this->LocalGenerator, Options::CudaCompiler, gg->GetCudaFlagTable());
  Options& cudaOptions = *pOptions;

  auto cudaVersion = this->GlobalGenerator->GetPlatformToolsetCudaString();

  // Get compile flags for CUDA in this directory.
  std::string flags;
  this->LocalGenerator->AddLanguageFlags(
    flags, this->GeneratorTarget, cmBuildStep::Compile, "CUDA", configName);
  this->LocalGenerator->AddCompileOptions(flags, this->GeneratorTarget, "CUDA",
                                          configName);

  // Get preprocessor definitions for this directory.
  std::string defineFlags = this->Makefile->GetDefineFlags();

  cudaOptions.Parse(flags);
  cudaOptions.Parse(defineFlags);
  cudaOptions.ParseFinish();

  // If we haven't explicitly enabled GPU debug information
  // explicitly disable it
  if (!cudaOptions.HasFlag("GPUDebugInfo")) {
    cudaOptions.AddFlag("GPUDebugInfo", "false");
  }

  // The extension on object libraries the CUDA gives isn't
  // consistent with how MSVC generates object libraries for C+, so set
  // the default to not have any extension
  cudaOptions.AddFlag("CompileOut", "$(IntDir)%(Filename).obj");

  if (this->GeneratorTarget->GetPropertyAsBool("CUDA_SEPARABLE_COMPILATION")) {
    cudaOptions.AddFlag("GenerateRelocatableDeviceCode", "true");
  }
  bool notPtxLike = true;
  if (this->GeneratorTarget->GetPropertyAsBool("CUDA_PTX_COMPILATION")) {
    cudaOptions.AddFlag("NvccCompilation", "ptx");
    // We drop the %(Extension) component as CMake expects all PTX files
    // to not have the source file extension at all
    cudaOptions.AddFlag("CompileOut", "$(IntDir)%(Filename).ptx");
    notPtxLike = false;

    if (cmSystemTools::VersionCompare(cmSystemTools::OP_GREATER_EQUAL,
                                      cudaVersion, "9.0") &&
        cmSystemTools::VersionCompare(cmSystemTools::OP_LESS, cudaVersion,
                                      "11.5")) {
      // The DriverApi flag before 11.5 ( verified back to 9.0 ) which controls
      // PTX compilation doesn't propagate user defines causing
      // target_compile_definitions to behave differently for VS +
      // PTX compared to other generators so we patch the rules
      // to normalize behavior
      cudaOptions.AddFlag("DriverApiCommandLineTemplate",
                          "%(BaseCommandLineTemplate) [CompileOut] [FastMath] "
                          "[Defines] \"%(FullPath)\"");
    }
  } else if (this->GeneratorTarget->GetPropertyAsBool(
               "CUDA_CUBIN_COMPILATION")) {
    cudaOptions.AddFlag("NvccCompilation", "cubin");
    cudaOptions.AddFlag("CompileOut", "$(IntDir)%(Filename).cubin");
    notPtxLike = false;
  } else if (this->GeneratorTarget->GetPropertyAsBool(
               "CUDA_FATBIN_COMPILATION")) {
    cudaOptions.AddFlag("NvccCompilation", "fatbin");
    cudaOptions.AddFlag("CompileOut", "$(IntDir)%(Filename).fatbin");
    notPtxLike = false;
  } else if (this->GeneratorTarget->GetPropertyAsBool(
               "CUDA_OPTIX_COMPILATION")) {
    cudaOptions.AddFlag("NvccCompilation", "optix-ir");
    cudaOptions.AddFlag("CompileOut", "$(IntDir)%(Filename).optixir");
    notPtxLike = false;
  }

  if (notPtxLike &&
      cmSystemTools::VersionCompareGreaterEq(
        "8.0", this->GlobalGenerator->GetPlatformToolsetCudaString())) {
    // Explicitly state that we want this file to be treated as a
    // CUDA file no matter what the file extensions is
    // This is only needed for < CUDA 9
    cudaOptions.AppendFlagString("AdditionalOptions", "-x cu");
  }

  // Specify the compiler program database file if configured.
  std::string pdb = this->GeneratorTarget->GetCompilePDBPath(configName);
  if (!pdb.empty()) {
    // CUDA does not make the directory if it is non-standard.
    std::string const pdbDir = cmSystemTools::GetFilenamePath(pdb);
    cmSystemTools::MakeDirectory(pdbDir);
    if (cmSystemTools::VersionCompareGreaterEq(
          "9.2", this->GlobalGenerator->GetPlatformToolsetCudaString())) {
      // CUDA does not have a field for this and does not honor the
      // ProgramDataBaseFileName field in ClCompile.  Work around this
      // limitation by creating the directory and passing the flag ourselves.
      pdb = this->ConvertPath(pdb, true);
      ConvertToWindowsSlash(pdb);
      std::string const clFd = cmStrCat(R"(-Xcompiler="-Fd\")", pdb, R"(\"")");
      cudaOptions.AppendFlagString("AdditionalOptions", clFd);
    }
  }

  // CUDA automatically passes the proper '--machine' flag to nvcc
  // for the current architecture, but does not reflect this default
  // in the user-visible IDE settings.  Set it explicitly.
  if (this->Platform == "x64"_s) {
    cudaOptions.AddFlag("TargetMachinePlatform", "64");
  }

  // Convert the host compiler options to the toolset's abstractions
  // using a secondary flag table.
  cudaOptions.ClearTables();
  cudaOptions.AddTable(gg->GetCudaHostFlagTable());
  cudaOptions.Reparse("AdditionalCompilerOptions");

  // `CUDA 8.0.targets` places AdditionalCompilerOptions before nvcc!
  // Pass them through -Xcompiler in AdditionalOptions instead.
  if (const char* acoPtr = cudaOptions.GetFlag("AdditionalCompilerOptions")) {
    std::string aco = acoPtr;
    cudaOptions.RemoveFlag("AdditionalCompilerOptions");
    if (!aco.empty()) {
      aco = this->LocalGenerator->EscapeForShell(aco, false);
      cudaOptions.AppendFlagString("AdditionalOptions",
                                   cmStrCat("-Xcompiler=", aco));
    }
  }

  cudaOptions.FixCudaCodeGeneration();

  std::vector<std::string> targetDefines;
  this->GeneratorTarget->GetCompileDefinitions(targetDefines, configName,
                                               "CUDA");
  cudaOptions.AddDefines(targetDefines);

  // Add a definition for the configuration name.
  std::string configDefine = cmStrCat("CMAKE_INTDIR=\"", configName, '"');
  cudaOptions.AddDefine(configDefine);
  if (const std::string* exportMacro =
        this->GeneratorTarget->GetExportMacro()) {
    cudaOptions.AddDefine(*exportMacro);
  }

  // Get includes for this target
  cudaOptions.AddIncludes(this->GetIncludes(configName, "CUDA"));
  cudaOptions.AddFlag("UseHostInclude", "false");

  // Add runtime library selection flag.
  std::string const& cudaRuntime =
    this->GeneratorTarget->GetRuntimeLinkLibrary("CUDA", configName);
  if (cudaRuntime == "STATIC"_s) {
    cudaOptions.AddFlag("CudaRuntime", "Static");
  } else if (cudaRuntime == "SHARED"_s) {
    cudaOptions.AddFlag("CudaRuntime", "Shared");
  } else if (cudaRuntime == "NONE"_s) {
    cudaOptions.AddFlag("CudaRuntime", "None");
  }

  this->CudaOptions[configName] = std::move(pOptions);
  return true;
}

void cmVisualStudio10TargetGenerator::WriteCudaOptions(
  Elem& e1, std::string const& configName)
{
  if (!this->MSTools || !this->GlobalGenerator->IsCudaEnabled() ||
      !this->GeneratorTarget->IsLanguageUsed("CUDA", configName)) {
    return;
  }
  Elem e2(e1, "CudaCompile");

  OptionsHelper cudaOptions(*(this->CudaOptions[configName]), e2);
  cudaOptions.OutputAdditionalIncludeDirectories("CUDA");
  cudaOptions.OutputPreprocessorDefinitions("CUDA");
  cudaOptions.PrependInheritedString("AdditionalOptions");
  cudaOptions.OutputFlagMap();
}

bool cmVisualStudio10TargetGenerator::ComputeCudaLinkOptions()
{
  if (!this->GlobalGenerator->IsCudaEnabled()) {
    return true;
  }
  return std::all_of(
    this->Configurations.begin(), this->Configurations.end(),
    [this](std::string const& c) { return this->ComputeCudaLinkOptions(c); });
}

bool cmVisualStudio10TargetGenerator::ComputeCudaLinkOptions(
  std::string const& configName)
{
  cmGlobalVisualStudio10Generator* gg = this->GlobalGenerator;
  auto pOptions = cm::make_unique<Options>(
    this->LocalGenerator, Options::CudaCompiler, gg->GetCudaFlagTable());
  Options& cudaLinkOptions = *pOptions;

  cmGeneratorTarget::DeviceLinkSetter setter(*this->GeneratorTarget);

  // Determine if we need to do a device link
  const bool doDeviceLinking = requireDeviceLinking(
    *this->GeneratorTarget, *this->LocalGenerator, configName);

  cudaLinkOptions.AddFlag("PerformDeviceLink",
                          doDeviceLinking ? "true" : "false");

  // Add extra flags for device linking
  cudaLinkOptions.AppendFlagString(
    "AdditionalOptions",
    this->Makefile->GetSafeDefinition("_CMAKE_CUDA_EXTRA_FLAGS"));
  cudaLinkOptions.AppendFlagString(
    "AdditionalOptions",
    this->Makefile->GetSafeDefinition("_CMAKE_CUDA_EXTRA_DEVICE_LINK_FLAGS"));

  std::vector<std::string> linkOpts;
  std::string linkFlags;
  this->GeneratorTarget->GetLinkOptions(linkOpts, configName, "CUDA");
  // LINK_OPTIONS are escaped.
  this->LocalGenerator->AppendCompileOptions(linkFlags, linkOpts);

  cmComputeLinkInformation* pcli =
    this->GeneratorTarget->GetLinkInformation(configName);
  if (doDeviceLinking && pcli) {

    cmLinkLineDeviceComputer computer(
      this->LocalGenerator,
      this->LocalGenerator->GetStateSnapshot().GetDirectory());
    std::string ignored_;
    this->LocalGenerator->GetDeviceLinkFlags(computer, configName, ignored_,
                                             linkFlags, ignored_, ignored_,
                                             this->GeneratorTarget);

    this->LocalGenerator->AddLanguageFlagsForLinking(
      linkFlags, this->GeneratorTarget, "CUDA", configName);
  }
  cudaLinkOptions.AppendFlagString("AdditionalOptions", linkFlags);

  if (doDeviceLinking) {
    std::vector<std::string> libVec;
    auto const& kinded = this->GeneratorTarget->GetKindedSources(configName);
    // CMake conversion uses full paths when possible to allow deeper trees.
    // However, CUDA 8.0 msbuild rules fail on absolute paths so for CUDA
    // we must use relative paths.
    const bool forceRelative = true;
    for (cmGeneratorTarget::SourceAndKind const& si : kinded.Sources) {
      switch (si.Kind) {
        case cmGeneratorTarget::SourceKindExternalObject: {
          std::string path =
            this->ConvertPath(si.Source.Value->GetFullPath(), forceRelative);
          ConvertToWindowsSlash(path);
          libVec.emplace_back(std::move(path));
        } break;
        default:
          break;
      }
    }
    // For static libraries that have device linking enabled compute
    // the  libraries
    if (this->GeneratorTarget->GetType() == cmStateEnums::STATIC_LIBRARY) {
      cmComputeLinkInformation& cli = *pcli;
      cmLinkLineDeviceComputer computer(
        this->LocalGenerator,
        this->LocalGenerator->GetStateSnapshot().GetDirectory());
      std::vector<BT<std::string>> btLibVec;
      computer.ComputeLinkLibraries(cli, std::string{}, btLibVec);
      for (auto const& item : btLibVec) {
        libVec.emplace_back(item.Value);
      }
    }
    if (!libVec.empty()) {
      cudaLinkOptions.AddFlag("AdditionalDependencies", libVec);
    }
  }

  this->CudaLinkOptions[configName] = std::move(pOptions);
  return true;
}

void cmVisualStudio10TargetGenerator::WriteCudaLinkOptions(
  Elem& e1, std::string const& configName)
{
  // We need to write link options for OBJECT libraries so that
  // we override the default device link behavior ( enabled ) when
  // building object libraries with ptx/optix-ir/etc
  if (this->GeneratorTarget->GetType() > cmStateEnums::OBJECT_LIBRARY) {
    return;
  }

  if (!this->MSTools || !this->GlobalGenerator->IsCudaEnabled()) {
    return;
  }

  Elem e2(e1, "CudaLink");
  OptionsHelper cudaLinkOptions(*(this->CudaLinkOptions[configName]), e2);
  cudaLinkOptions.OutputFlagMap();
}

bool cmVisualStudio10TargetGenerator::ComputeMarmasmOptions()
{
  if (!this->GlobalGenerator->IsMarmasmEnabled()) {
    return true;
  }
  return std::all_of(
    this->Configurations.begin(), this->Configurations.end(),
    [this](std::string const& c) { return this->ComputeMarmasmOptions(c); });
}

bool cmVisualStudio10TargetGenerator::ComputeMarmasmOptions(
  std::string const& configName)
{
  cmGlobalVisualStudio10Generator* gg = this->GlobalGenerator;
  auto pOptions = cm::make_unique<Options>(
    this->LocalGenerator, Options::MarmasmCompiler, gg->GetMarmasmFlagTable());
  Options& marmasmOptions = *pOptions;

  std::string flags;
  this->LocalGenerator->AddLanguageFlags(flags, this->GeneratorTarget,
                                         cmBuildStep::Compile, "ASM_MARMASM",
                                         configName);
  this->LocalGenerator->AddCompileOptions(flags, this->GeneratorTarget,
                                          "ASM_MARMASM", configName);

  marmasmOptions.Parse(flags);

  // Get includes for this target
  marmasmOptions.AddIncludes(this->GetIncludes(configName, "ASM_MARMASM"));

  this->MarmasmOptions[configName] = std::move(pOptions);
  return true;
}

void cmVisualStudio10TargetGenerator::WriteMarmasmOptions(
  Elem& e1, std::string const& configName)
{
  if (!this->MSTools || !this->GlobalGenerator->IsMarmasmEnabled()) {
    return;
  }
  Elem e2(e1, "MARMASM");

  // Preprocessor definitions and includes are shared with clOptions.
  OptionsHelper clOptions(*(this->ClOptions[configName]), e2);
  clOptions.OutputPreprocessorDefinitions("ASM_MARMASM");

  OptionsHelper marmasmOptions(*(this->MarmasmOptions[configName]), e2);
  marmasmOptions.OutputAdditionalIncludeDirectories("ASM_MARMASM");
  marmasmOptions.PrependInheritedString("AdditionalOptions");
  marmasmOptions.OutputFlagMap();
}

bool cmVisualStudio10TargetGenerator::ComputeMasmOptions()
{
  if (!this->GlobalGenerator->IsMasmEnabled()) {
    return true;
  }
  return std::all_of(
    this->Configurations.begin(), this->Configurations.end(),
    [this](std::string const& c) { return this->ComputeMasmOptions(c); });
}

bool cmVisualStudio10TargetGenerator::ComputeMasmOptions(
  std::string const& configName)
{
  cmGlobalVisualStudio10Generator* gg = this->GlobalGenerator;
  auto pOptions = cm::make_unique<Options>(
    this->LocalGenerator, Options::MasmCompiler, gg->GetMasmFlagTable());
  Options& masmOptions = *pOptions;

  // MSBuild enables debug information by default.
  // Disable it explicitly unless a flag parsed below re-enables it.
  masmOptions.AddFlag("GenerateDebugInformation", "false");

  std::string flags;
  this->LocalGenerator->AddLanguageFlags(flags, this->GeneratorTarget,
                                         cmBuildStep::Compile, "ASM_MASM",
                                         configName);
  this->LocalGenerator->AddCompileOptions(flags, this->GeneratorTarget,
                                          "ASM_MASM", configName);

  masmOptions.Parse(flags);

  // Get includes for this target
  masmOptions.AddIncludes(this->GetIncludes(configName, "ASM_MASM"));

  this->MasmOptions[configName] = std::move(pOptions);
  return true;
}

void cmVisualStudio10TargetGenerator::WriteMasmOptions(
  Elem& e1, std::string const& configName)
{
  if (!this->MSTools || !this->GlobalGenerator->IsMasmEnabled()) {
    return;
  }
  Elem e2(e1, "MASM");

  // Preprocessor definitions and includes are shared with clOptions.
  OptionsHelper clOptions(*(this->ClOptions[configName]), e2);
  clOptions.OutputPreprocessorDefinitions("ASM_MASM");

  OptionsHelper masmOptions(*(this->MasmOptions[configName]), e2);
  masmOptions.OutputAdditionalIncludeDirectories("ASM_MASM");
  masmOptions.PrependInheritedString("AdditionalOptions");
  masmOptions.OutputFlagMap();
}

bool cmVisualStudio10TargetGenerator::ComputeNasmOptions()
{
  if (!this->GlobalGenerator->IsNasmEnabled()) {
    return true;
  }
  return std::all_of(
    this->Configurations.begin(), this->Configurations.end(),
    [this](std::string const& c) { return this->ComputeNasmOptions(c); });
}

bool cmVisualStudio10TargetGenerator::ComputeNasmOptions(
  std::string const& configName)
{
  cmGlobalVisualStudio10Generator* gg = this->GlobalGenerator;
  auto pOptions = cm::make_unique<Options>(
    this->LocalGenerator, Options::NasmCompiler, gg->GetNasmFlagTable());
  Options& nasmOptions = *pOptions;

  std::string flags;
  this->LocalGenerator->AddLanguageFlags(flags, this->GeneratorTarget,
                                         cmBuildStep::Compile, "ASM_NASM",
                                         configName);
  this->LocalGenerator->AddCompileOptions(flags, this->GeneratorTarget,
                                          "ASM_NASM", configName);
  flags += " -f";
  flags += this->Makefile->GetSafeDefinition("CMAKE_ASM_NASM_OBJECT_FORMAT");
  nasmOptions.Parse(flags);

  // Get includes for this target
  nasmOptions.AddIncludes(this->GetIncludes(configName, "ASM_NASM"));

  this->NasmOptions[configName] = std::move(pOptions);
  return true;
}

void cmVisualStudio10TargetGenerator::WriteNasmOptions(
  Elem& e1, std::string const& configName)
{
  if (!this->GlobalGenerator->IsNasmEnabled()) {
    return;
  }
  Elem e2(e1, "NASM");

  OptionsHelper nasmOptions(*(this->NasmOptions[configName]), e2);
  nasmOptions.OutputAdditionalIncludeDirectories("ASM_NASM");
  nasmOptions.OutputFlagMap();
  nasmOptions.PrependInheritedString("AdditionalOptions");
  nasmOptions.OutputPreprocessorDefinitions("ASM_NASM");

  // Preprocessor definitions and includes are shared with clOptions.
  OptionsHelper clOptions(*(this->ClOptions[configName]), e2);
  clOptions.OutputPreprocessorDefinitions("ASM_NASM");
}

void cmVisualStudio10TargetGenerator::WriteLibOptions(
  Elem& e1, std::string const& config)
{
  if (this->GeneratorTarget->GetType() != cmStateEnums::STATIC_LIBRARY &&
      this->GeneratorTarget->GetType() != cmStateEnums::OBJECT_LIBRARY) {
    return;
  }

  const std::string& linkLanguage =
    this->GeneratorTarget->GetLinkClosure(config)->LinkerLanguage;

  std::string libflags;
  this->LocalGenerator->GetStaticLibraryFlags(libflags, config, linkLanguage,
                                              this->GeneratorTarget);
  if (!libflags.empty()) {
    Elem e2(e1, "Lib");
    cmGlobalVisualStudio10Generator* gg = this->GlobalGenerator;
    cmVS10GeneratorOptions libOptions(this->LocalGenerator,
                                      cmVisualStudioGeneratorOptions::Linker,
                                      gg->GetLibFlagTable(), this);
    libOptions.Parse(libflags);
    OptionsHelper oh(libOptions, e2);
    oh.PrependInheritedString("AdditionalOptions");
    oh.OutputFlagMap();
  }

  // We cannot generate metadata for static libraries.  WindowsPhone
  // and WindowsStore tools look at GenerateWindowsMetadata in the
  // Link tool options even for static libraries.
  if (this->GlobalGenerator->TargetsWindowsPhone() ||
      this->GlobalGenerator->TargetsWindowsStore()) {
    Elem e2(e1, "Link");
    e2.Element("GenerateWindowsMetadata", "false");
  }
}

void cmVisualStudio10TargetGenerator::WriteManifestOptions(
  Elem& e1, std::string const& config)
{
  if (this->GeneratorTarget->GetType() != cmStateEnums::EXECUTABLE &&
      this->GeneratorTarget->GetType() != cmStateEnums::SHARED_LIBRARY &&
      this->GeneratorTarget->GetType() != cmStateEnums::MODULE_LIBRARY) {
    return;
  }

  std::vector<cmSourceFile const*> manifest_srcs;
  this->GeneratorTarget->GetManifests(manifest_srcs, config);

  cmValue dpiAware = this->GeneratorTarget->GetProperty("VS_DPI_AWARE");

  if (!manifest_srcs.empty() || dpiAware) {
    Elem e2(e1, "Manifest");
    if (!manifest_srcs.empty()) {
      std::ostringstream oss;
      for (cmSourceFile const* mi : manifest_srcs) {
        std::string m = this->ConvertPath(mi->GetFullPath(), false);
        ConvertToWindowsSlash(m);
        oss << m << ";";
      }
      e2.Element("AdditionalManifestFiles", oss.str());
    }
    if (dpiAware) {
      if (*dpiAware == "PerMonitor"_s) {
        e2.Element("EnableDpiAwareness", "PerMonitorHighDPIAware");
      } else if (cmIsOn(*dpiAware)) {
        e2.Element("EnableDpiAwareness", "true");
      } else if (cmIsOff(*dpiAware)) {
        e2.Element("EnableDpiAwareness", "false");
      } else {
        cmSystemTools::Error(
          cmStrCat("Bad parameter for VS_DPI_AWARE: ", *dpiAware));
      }
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteAntBuildOptions(
  Elem& e1, std::string const& configName)
{
  // Look through the sources for AndroidManifest.xml and use
  // its location as the root source directory.
  std::string rootDir = this->LocalGenerator->GetCurrentSourceDirectory();
  {
    for (cmGeneratorTarget::AllConfigSource const& source :
         this->GeneratorTarget->GetAllConfigSources()) {
      if (source.Kind == cmGeneratorTarget::SourceKindExtra &&
          "androidmanifest.xml" ==
            cmSystemTools::LowerCase(source.Source->GetLocation().GetName())) {
        rootDir = source.Source->GetLocation().GetDirectory();
        break;
      }
    }
  }

  // Tell MSBuild to launch Ant.
  Elem e2(e1, "AntBuild");
  {
    std::string antBuildPath = rootDir;
    ConvertToWindowsSlash(antBuildPath);
    e2.Element("AntBuildPath", antBuildPath);
  }

  if (this->GeneratorTarget->GetPropertyAsBool("ANDROID_SKIP_ANT_STEP")) {
    e2.Element("SkipAntStep", "true");
  }

  if (this->GeneratorTarget->GetPropertyAsBool("ANDROID_PROGUARD")) {
    e2.Element("EnableProGuard", "true");
  }

  if (cmValue proGuardConfigLocation =
        this->GeneratorTarget->GetProperty("ANDROID_PROGUARD_CONFIG_PATH")) {
    e2.Element("ProGuardConfigLocation", *proGuardConfigLocation);
  }

  if (cmValue securePropertiesLocation =
        this->GeneratorTarget->GetProperty("ANDROID_SECURE_PROPS_PATH")) {
    e2.Element("SecurePropertiesLocation", *securePropertiesLocation);
  }

  if (cmValue nativeLibDirectoriesExpression =
        this->GeneratorTarget->GetProperty("ANDROID_NATIVE_LIB_DIRECTORIES")) {
    std::string nativeLibDirs = cmGeneratorExpression::Evaluate(
      *nativeLibDirectoriesExpression, this->LocalGenerator, configName);
    e2.Element("NativeLibDirectories", nativeLibDirs);
  }

  if (cmValue nativeLibDependenciesExpression =
        this->GeneratorTarget->GetProperty(
          "ANDROID_NATIVE_LIB_DEPENDENCIES")) {
    std::string nativeLibDeps = cmGeneratorExpression::Evaluate(
      *nativeLibDependenciesExpression, this->LocalGenerator, configName);
    e2.Element("NativeLibDependencies", nativeLibDeps);
  }

  if (cmValue javaSourceDir =
        this->GeneratorTarget->GetProperty("ANDROID_JAVA_SOURCE_DIR")) {
    e2.Element("JavaSourceDir", *javaSourceDir);
  }

  if (cmValue jarDirectoriesExpression =
        this->GeneratorTarget->GetProperty("ANDROID_JAR_DIRECTORIES")) {
    std::string jarDirectories = cmGeneratorExpression::Evaluate(
      *jarDirectoriesExpression, this->LocalGenerator, configName);
    e2.Element("JarDirectories", jarDirectories);
  }

  if (cmValue jarDeps =
        this->GeneratorTarget->GetProperty("ANDROID_JAR_DEPENDENCIES")) {
    e2.Element("JarDependencies", *jarDeps);
  }

  if (cmValue assetsDirectories =
        this->GeneratorTarget->GetProperty("ANDROID_ASSETS_DIRECTORIES")) {
    e2.Element("AssetsDirectories", *assetsDirectories);
  }

  {
    std::string manifest_xml = cmStrCat(rootDir, "/AndroidManifest.xml");
    ConvertToWindowsSlash(manifest_xml);
    e2.Element("AndroidManifestLocation", manifest_xml);
  }

  if (cmValue antAdditionalOptions =
        this->GeneratorTarget->GetProperty("ANDROID_ANT_ADDITIONAL_OPTIONS")) {
    e2.Element("AdditionalOptions",
               cmStrCat(*antAdditionalOptions, " %(AdditionalOptions)"));
  }
}

bool cmVisualStudio10TargetGenerator::ComputeLinkOptions()
{
  if (this->GeneratorTarget->GetType() == cmStateEnums::EXECUTABLE ||
      this->GeneratorTarget->GetType() == cmStateEnums::SHARED_LIBRARY ||
      this->GeneratorTarget->GetType() == cmStateEnums::MODULE_LIBRARY) {
    for (std::string const& c : this->Configurations) {
      if (!this->ComputeLinkOptions(c)) {
        return false;
      }
    }
  }
  return true;
}

bool cmVisualStudio10TargetGenerator::ComputeLinkOptions(
  std::string const& config)
{
  cmGlobalVisualStudio10Generator* gg = this->GlobalGenerator;
  auto pOptions = cm::make_unique<Options>(
    this->LocalGenerator, Options::Linker, gg->GetLinkFlagTable(), this);
  Options& linkOptions = *pOptions;

  cmGeneratorTarget::LinkClosure const* linkClosure =
    this->GeneratorTarget->GetLinkClosure(config);

  const std::string& linkLanguage = linkClosure->LinkerLanguage;
  if (linkLanguage.empty()) {
    cmSystemTools::Error(cmStrCat(
      "CMake can not determine linker language for target: ", this->Name));
    return false;
  }

  std::string CONFIG = cmSystemTools::UpperCase(config);

  const char* linkType = "SHARED";
  if (this->GeneratorTarget->GetType() == cmStateEnums::MODULE_LIBRARY) {
    linkType = "MODULE";
  }
  if (this->GeneratorTarget->GetType() == cmStateEnums::EXECUTABLE) {
    linkType = "EXE";
  }
  std::string flags;
  std::string linkFlagVarBase = cmStrCat("CMAKE_", linkType, "_LINKER_FLAGS");
  flags += ' ';
  flags += this->Makefile->GetRequiredDefinition(linkFlagVarBase);
  std::string linkFlagVar = cmStrCat(linkFlagVarBase, '_', CONFIG);
  flags += ' ';
  flags += this->Makefile->GetRequiredDefinition(linkFlagVar);
  cmValue targetLinkFlags = this->GeneratorTarget->GetProperty("LINK_FLAGS");
  if (targetLinkFlags) {
    flags += ' ';
    flags += *targetLinkFlags;
  }
  std::string flagsProp = cmStrCat("LINK_FLAGS_", CONFIG);
  if (cmValue flagsConfig = this->GeneratorTarget->GetProperty(flagsProp)) {
    flags += ' ';
    flags += *flagsConfig;
  }

  std::vector<std::string> opts;
  this->GeneratorTarget->GetLinkOptions(opts, config, linkLanguage);
  // LINK_OPTIONS are escaped.
  this->LocalGenerator->AppendCompileOptions(flags, opts);

  cmComputeLinkInformation* pcli =
    this->GeneratorTarget->GetLinkInformation(config);
  if (!pcli) {
    cmSystemTools::Error(
      cmStrCat("CMake can not compute cmComputeLinkInformation for target: ",
               this->Name));
    return false;
  }
  cmComputeLinkInformation& cli = *pcli;

  std::vector<std::string> libVec;
  std::vector<std::string> vsTargetVec;
  this->AddLibraries(cli, libVec, vsTargetVec, config);
  std::string standardLibsVar =
    cmStrCat("CMAKE_", linkLanguage, "_STANDARD_LIBRARIES");
  std::string const& libs = this->Makefile->GetSafeDefinition(standardLibsVar);
  cmSystemTools::ParseWindowsCommandLine(libs.c_str(), libVec);
  linkOptions.AddFlag("AdditionalDependencies", libVec);

  // Populate TargetsFileAndConfigsVec
  for (std::string const& ti : vsTargetVec) {
    this->AddTargetsFileAndConfigPair(ti, config);
  }

  std::vector<std::string> const& ldirs = cli.GetDirectories();
  std::vector<std::string> linkDirs;
  for (std::string const& d : ldirs) {
    // first just full path
    linkDirs.push_back(d);
    // next path with configuration type Debug, Release, etc
    linkDirs.emplace_back(cmStrCat(d, "/$(Configuration)"));
  }
  linkDirs.push_back("%(AdditionalLibraryDirectories)");
  linkOptions.AddFlag("AdditionalLibraryDirectories", linkDirs);

  cmGeneratorTarget::Names targetNames;
  if (this->GeneratorTarget->GetType() == cmStateEnums::EXECUTABLE) {
    targetNames = this->GeneratorTarget->GetExecutableNames(config);
  } else {
    targetNames = this->GeneratorTarget->GetLibraryNames(config);
  }

  if (this->MSTools) {
    if (this->GeneratorTarget->IsWin32Executable(config)) {
      if (this->GlobalGenerator->TargetsWindowsCE()) {
        linkOptions.AddFlag("SubSystem", "WindowsCE");
        if (this->GeneratorTarget->GetType() == cmStateEnums::EXECUTABLE) {
          if (this->ClOptions[config]->UsingUnicode()) {
            linkOptions.AddFlag("EntryPointSymbol", "wWinMainCRTStartup");
          } else {
            linkOptions.AddFlag("EntryPointSymbol", "WinMainCRTStartup");
          }
        }
      } else {
        linkOptions.AddFlag("SubSystem", "Windows");
      }
    } else {
      if (this->GlobalGenerator->TargetsWindowsCE()) {
        linkOptions.AddFlag("SubSystem", "WindowsCE");
        if (this->GeneratorTarget->GetType() == cmStateEnums::EXECUTABLE) {
          if (this->ClOptions[config]->UsingUnicode()) {
            linkOptions.AddFlag("EntryPointSymbol", "mainWCRTStartup");
          } else {
            linkOptions.AddFlag("EntryPointSymbol", "mainACRTStartup");
          }
        }
      } else {
        linkOptions.AddFlag("SubSystem", "Console");
      };
    }

    if (cmValue stackVal = this->Makefile->GetDefinition(
          cmStrCat("CMAKE_", linkLanguage, "_STACK_SIZE"))) {
      linkOptions.AddFlag("StackReserveSize", *stackVal);
    }

    linkOptions.AddFlag("GenerateDebugInformation", "false");

    std::string pdb = cmStrCat(this->GeneratorTarget->GetPDBDirectory(config),
                               '/', targetNames.PDB);
    if (!targetNames.ImportLibrary.empty()) {
      std::string imLib =
        cmStrCat(this->GeneratorTarget->GetDirectory(
                   config, cmStateEnums::ImportLibraryArtifact),
                 '/', targetNames.ImportLibrary);

      linkOptions.AddFlag("ImportLibrary", imLib);
    }
    linkOptions.AddFlag("ProgramDataBaseFile", pdb);

    // A Windows Runtime component uses internal .NET metadata,
    // so does not have an import library.
    if (this->GeneratorTarget->GetPropertyAsBool("VS_WINRT_COMPONENT") &&
        this->GeneratorTarget->GetType() != cmStateEnums::EXECUTABLE) {
      linkOptions.AddFlag("GenerateWindowsMetadata", "true");
    } else if (this->GlobalGenerator->TargetsWindowsPhone() ||
               this->GlobalGenerator->TargetsWindowsStore()) {
      // WindowsPhone and WindowsStore components are in an app container
      // and produce WindowsMetadata.  If we are not producing a WINRT
      // component, then do not generate the metadata here.
      linkOptions.AddFlag("GenerateWindowsMetadata", "false");
    }

    if (this->GlobalGenerator->TargetsWindowsPhone() &&
        this->GlobalGenerator->GetSystemVersion() == "8.0"_s) {
      // WindowsPhone 8.0 does not have ole32.
      linkOptions.AppendFlag("IgnoreSpecificDefaultLibraries", "ole32.lib");
    }
  } else if (this->NsightTegra) {
    linkOptions.AddFlag("SoName", targetNames.SharedObject);
  }

  linkOptions.Parse(flags);
  linkOptions.FixManifestUACFlags();

  if (this->MSTools) {
    cmGeneratorTarget::ModuleDefinitionInfo const* mdi =
      this->GeneratorTarget->GetModuleDefinitionInfo(config);
    if (mdi && !mdi->DefFile.empty()) {
      linkOptions.AddFlag("ModuleDefinitionFile", mdi->DefFile);
    }
    linkOptions.AppendFlag("IgnoreSpecificDefaultLibraries",
                           "%(IgnoreSpecificDefaultLibraries)");
  }

  // VS 2015 without all updates has a v140 toolset whose
  // GenerateDebugInformation expects No/Debug instead of false/true.
  if (gg->GetPlatformToolsetNeedsDebugEnum()) {
    if (const char* debug = linkOptions.GetFlag("GenerateDebugInformation")) {
      if (strcmp(debug, "false") == 0) {
        linkOptions.AddFlag("GenerateDebugInformation", "No");
      } else if (strcmp(debug, "true") == 0) {
        linkOptions.AddFlag("GenerateDebugInformation", "Debug");
      }
    }
  }

  // Managed code cannot be linked with /DEBUG:FASTLINK
  if (this->Managed) {
    if (const char* debug = linkOptions.GetFlag("GenerateDebugInformation")) {
      if (strcmp(debug, "DebugFastLink") == 0) {
        linkOptions.AddFlag("GenerateDebugInformation", "Debug");
      }
    }
  }

  this->LinkOptions[config] = std::move(pOptions);
  return true;
}

bool cmVisualStudio10TargetGenerator::ComputeLibOptions()
{
  if (this->GeneratorTarget->GetType() == cmStateEnums::STATIC_LIBRARY) {
    for (std::string const& c : this->Configurations) {
      if (!this->ComputeLibOptions(c)) {
        return false;
      }
    }
  }
  return true;
}

bool cmVisualStudio10TargetGenerator::ComputeLibOptions(
  std::string const& config)
{
  cmComputeLinkInformation* pcli =
    this->GeneratorTarget->GetLinkInformation(config);
  if (!pcli) {
    cmSystemTools::Error(
      cmStrCat("CMake can not compute cmComputeLinkInformation for target: ",
               this->Name));
    return false;
  }

  cmComputeLinkInformation& cli = *pcli;
  using ItemVector = cmComputeLinkInformation::ItemVector;
  const ItemVector& libs = cli.GetItems();
  for (cmComputeLinkInformation::Item const& l : libs) {
    if (l.IsPath == cmComputeLinkInformation::ItemIsPath::Yes &&
        cmVS10IsTargetsFile(l.Value.Value)) {
      std::string path =
        this->LocalGenerator->MaybeRelativeToCurBinDir(l.Value.Value);
      ConvertToWindowsSlash(path);
      this->AddTargetsFileAndConfigPair(path, config);
    }
  }

  return true;
}

void cmVisualStudio10TargetGenerator::WriteLinkOptions(
  Elem& e1, std::string const& config)
{
  if (this->GeneratorTarget->GetType() == cmStateEnums::STATIC_LIBRARY ||
      this->GeneratorTarget->GetType() > cmStateEnums::MODULE_LIBRARY) {
    return;
  }
  if (this->ProjectType == VsProjectType::csproj) {
    return;
  }

  {
    Elem e2(e1, "Link");
    OptionsHelper linkOptions(*(this->LinkOptions[config]), e2);
    linkOptions.PrependInheritedString("AdditionalOptions");
    linkOptions.OutputFlagMap();
  }

  if (!this->GlobalGenerator->NeedLinkLibraryDependencies(
        this->GeneratorTarget)) {
    Elem e2(e1, "ProjectReference");
    e2.Element("LinkLibraryDependencies", "false");
  }
}

void cmVisualStudio10TargetGenerator::AddLibraries(
  const cmComputeLinkInformation& cli, std::vector<std::string>& libVec,
  std::vector<std::string>& vsTargetVec, const std::string& config)
{
  using ItemVector = cmComputeLinkInformation::ItemVector;
  ItemVector const& libs = cli.GetItems();
  for (cmComputeLinkInformation::Item const& l : libs) {
    if (l.Target) {
      auto managedType = l.Target->GetManagedType(config);
      if (managedType != cmGeneratorTarget::ManagedType::Native &&
          this->GeneratorTarget->GetManagedType(config) !=
            cmGeneratorTarget::ManagedType::Native &&
          l.Target->IsImported() &&
          l.Target->GetType() != cmStateEnums::INTERFACE_LIBRARY) {
        auto location = l.Target->GetFullPath(config);
        if (!location.empty()) {
          ConvertToWindowsSlash(location);
          switch (this->ProjectType) {
            case VsProjectType::csproj:
              // If the target we want to "link" to is an imported managed
              // target and this is a C# project, we add a hint reference. This
              // reference is written to project file in
              // WriteDotNetReferences().
              this->DotNetHintReferences[config].push_back(
                DotNetHintReference(l.Target->GetName(), location));
              break;
            case VsProjectType::vcxproj:
              // Add path of assembly to list of using-directories, so the
              // managed assembly can be used by '#using <assembly.dll>' in
              // code.
              this->AdditionalUsingDirectories[config].insert(
                cmSystemTools::GetFilenamePath(location));
              break;
            default:
              // In .proj files, we wouldn't be referencing libraries.
              break;
          }
        }
      }
      // Do not allow C# targets to be added to the LIB listing. LIB files are
      // used for linking C++ dependencies. C# libraries do not have lib files.
      // Instead, they compile down to C# reference libraries (DLL files). The
      // `<ProjectReference>` elements added to the vcxproj are enough for the
      // IDE to deduce the DLL file required by other C# projects that need its
      // reference library.
      if (managedType == cmGeneratorTarget::ManagedType::Managed) {
        continue;
      }
    }

    if (l.IsPath == cmComputeLinkInformation::ItemIsPath::Yes) {
      std::string path =
        this->LocalGenerator->MaybeRelativeToCurBinDir(l.Value.Value);
      ConvertToWindowsSlash(path);
      if (cmVS10IsTargetsFile(l.Value.Value)) {
        vsTargetVec.push_back(path);
      } else {
        libVec.push_back(l.HasFeature() ? l.GetFormattedItem(path).Value
                                        : path);
      }
    } else if (!l.Target ||
               (l.Target->GetType() != cmStateEnums::INTERFACE_LIBRARY &&
                l.Target->GetType() != cmStateEnums::OBJECT_LIBRARY)) {
      libVec.push_back(l.Value.Value);
    }
  }
}

void cmVisualStudio10TargetGenerator::AddTargetsFileAndConfigPair(
  std::string const& targetsFile, std::string const& config)
{
  for (TargetsFileAndConfigs& i : this->TargetsFileAndConfigsVec) {
    if (cmSystemTools::ComparePath(targetsFile, i.File)) {
      if (!cm::contains(i.Configs, config)) {
        i.Configs.push_back(config);
      }
      return;
    }
  }
  TargetsFileAndConfigs entry;
  entry.File = targetsFile;
  entry.Configs.push_back(config);
  this->TargetsFileAndConfigsVec.push_back(entry);
}

void cmVisualStudio10TargetGenerator::WriteMidlOptions(
  Elem& e1, std::string const& configName)
{
  if (!this->MSTools) {
    return;
  }
  if (this->ProjectType == VsProjectType::csproj) {
    return;
  }
  if (this->GeneratorTarget->GetType() > cmStateEnums::UTILITY) {
    return;
  }

  // This processes *any* of the .idl files specified in the project's file
  // list (and passed as the item metadata %(Filename) expressing the rule
  // input filename) into output files at the per-config *build* dir
  // ($(IntDir)) each.
  //
  // IOW, this MIDL section is intended to provide a fully generic syntax
  // content suitable for most cases (read: if you get errors, then it's quite
  // probable that the error is on your side of the .idl setup).
  //
  // Also, note that the marked-as-generated _i.c file in the Visual Studio
  // generator case needs to be referred to as $(IntDir)\foo_i.c at the
  // project's file list, otherwise the compiler-side processing won't pick it
  // up (for non-directory form, it ends up looking in project binary dir
  // only).  Perhaps there's something to be done to make this more automatic
  // on the CMake side?
  std::vector<std::string> const includes =
    this->GetIncludes(configName, "MIDL");
  std::ostringstream oss;
  for (std::string const& i : includes) {
    oss << i << ";";
  }
  oss << "%(AdditionalIncludeDirectories)";

  Elem e2(e1, "Midl");
  e2.Element("AdditionalIncludeDirectories", oss.str());
  e2.Element("OutputDirectory", "$(ProjectDir)/$(IntDir)");
  e2.Element("HeaderFileName", "%(Filename).h");
  e2.Element("TypeLibraryName", "%(Filename).tlb");
  e2.Element("InterfaceIdentifierFileName", "%(Filename)_i.c");
  e2.Element("ProxyFileName", "%(Filename)_p.c");
}

void cmVisualStudio10TargetGenerator::WriteItemDefinitionGroups(Elem& e0)
{
  if (this->ProjectType == VsProjectType::csproj) {
    return;
  }
  for (const std::string& c : this->Configurations) {
    Elem e1(e0, "ItemDefinitionGroup");
    e1.Attribute("Condition", this->CalcCondition(c));

    //    output cl compile flags <ClCompile></ClCompile>
    if (this->GeneratorTarget->GetType() <= cmStateEnums::OBJECT_LIBRARY) {
      this->WriteClOptions(e1, c);
      //    output rc compile flags <ResourceCompile></ResourceCompile>
      this->WriteRCOptions(e1, c);
      this->WriteCudaOptions(e1, c);
      this->WriteMarmasmOptions(e1, c);
      this->WriteMasmOptions(e1, c);
      this->WriteNasmOptions(e1, c);
    }
    //    output midl flags       <Midl></Midl>
    this->WriteMidlOptions(e1, c);
    // write events
    if (this->ProjectType != VsProjectType::csproj) {
      this->WriteEvents(e1, c);
    }
    //    output link flags       <Link></Link>
    this->WriteLinkOptions(e1, c);
    this->WriteCudaLinkOptions(e1, c);
    //    output lib flags       <Lib></Lib>
    this->WriteLibOptions(e1, c);
    //    output manifest flags  <Manifest></Manifest>
    this->WriteManifestOptions(e1, c);
    if (this->NsightTegra &&
        this->GeneratorTarget->Target->IsAndroidGuiExecutable()) {
      this->WriteAntBuildOptions(e1, c);
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteEvents(
  Elem& e1, std::string const& configName)
{
  bool addedPrelink = false;
  cmGeneratorTarget::ModuleDefinitionInfo const* mdi =
    this->GeneratorTarget->GetModuleDefinitionInfo(configName);
  if (mdi && mdi->DefFileGenerated) {
    addedPrelink = true;
    std::vector<cmCustomCommand> commands =
      this->GeneratorTarget->GetPreLinkCommands();
    this->GlobalGenerator->AddSymbolExportCommand(this->GeneratorTarget,
                                                  commands, configName);
    this->WriteEvent(e1, "PreLinkEvent", commands, configName);
  }
  if (!addedPrelink) {
    this->WriteEvent(e1, "PreLinkEvent",
                     this->GeneratorTarget->GetPreLinkCommands(), configName);
  }
  this->WriteEvent(e1, "PreBuildEvent",
                   this->GeneratorTarget->GetPreBuildCommands(), configName);
  this->WriteEvent(e1, "PostBuildEvent",
                   this->GeneratorTarget->GetPostBuildCommands(), configName);
}

void cmVisualStudio10TargetGenerator::WriteEvent(
  Elem& e1, const std::string& name,
  std::vector<cmCustomCommand> const& commands, std::string const& configName)
{
  if (commands.empty()) {
    return;
  }
  cmLocalVisualStudio7Generator* lg = this->LocalGenerator;
  std::string script;
  const char* pre = "";
  std::string comment;
  bool stdPipesUTF8 = false;
  for (cmCustomCommand const& cc : commands) {
    cmCustomCommandGenerator ccg(cc, configName, lg);
    if (!ccg.HasOnlyEmptyCommandLines()) {
      comment += pre;
      comment += lg->ConstructComment(ccg);
      script += pre;
      pre = "\n";
      script += lg->ConstructScript(ccg);

      stdPipesUTF8 = stdPipesUTF8 || cc.GetStdPipesUTF8();
    }
  }
  if (!script.empty()) {
    script += lg->FinishConstructScript(this->ProjectType);
  }
  comment = cmVS10EscapeComment(comment);
  if (this->ProjectType != VsProjectType::csproj) {
    Elem e2(e1, name);
    if (stdPipesUTF8) {
      this->WriteStdOutEncodingUtf8(e2);
    }
    e2.Element("Message", comment);
    e2.Element("Command", script);
  } else {
    std::string strippedComment = comment;
    strippedComment.erase(
      std::remove(strippedComment.begin(), strippedComment.end(), '\t'),
      strippedComment.end());
    std::ostringstream oss;
    if (!comment.empty() && !strippedComment.empty()) {
      oss << "echo " << comment << "\n";
    }
    oss << script << "\n";
    e1.Element(name, oss.str());
  }
}

void cmVisualStudio10TargetGenerator::WriteProjectReferences(Elem& e0)
{
  cmGlobalGenerator::TargetDependSet const& unordered =
    this->GlobalGenerator->GetTargetDirectDepends(this->GeneratorTarget);
  using OrderedTargetDependSet =
    cmGlobalVisualStudioGenerator::OrderedTargetDependSet;
  OrderedTargetDependSet depends(unordered, CMAKE_CHECK_BUILD_SYSTEM_TARGET);
  Elem e1(e0, "ItemGroup");
  e1.SetHasElements();
  for (cmGeneratorTarget const* dt : depends) {
    if (!dt->IsInBuildSystem()) {
      continue;
    }
    // skip fortran targets as they can not be processed by MSBuild
    // the only reference will be in the .sln file
    if (this->GlobalGenerator->TargetIsFortranOnly(dt)) {
      continue;
    }
    cmLocalGenerator* lg = dt->GetLocalGenerator();
    std::string name = dt->GetName();
    std::string path;
    if (cmValue p = dt->GetProperty("EXTERNAL_MSPROJECT")) {
      path = *p;
    } else {
      path = cmStrCat(lg->GetCurrentBinaryDirectory(), '/', dt->GetName(),
                      computeProjectFileExtension(dt));
    }
    ConvertToWindowsSlash(path);
    Elem e2(e1, "ProjectReference");
    e2.Attribute("Include", path);
    e2.Element("Project",
               cmStrCat('{', this->GlobalGenerator->GetGUID(name), '}'));
    e2.Element("Name", name);
    this->WriteDotNetReferenceCustomTags(e2, name);
    if (dt->IsCSharpOnly() || cmHasLiteralSuffix(path, "csproj")) {
      e2.Element("SkipGetTargetFrameworkProperties", "true");
    }
    // Don't reference targets that don't produce any output.
    else if (this->Configurations.empty() ||
             dt->GetManagedType(this->Configurations[0]) ==
               cmGeneratorTarget::ManagedType::Undefined) {
      e2.Element("ReferenceOutputAssembly", "false");
      e2.Element("CopyToOutputDirectory", "Never");
    }
  }
}

void cmVisualStudio10TargetGenerator::WritePlatformExtensions(Elem& e1)
{
  // This only applies to Windows 10 apps
  if (this->GlobalGenerator->TargetsWindowsStore() &&
      cmHasLiteralPrefix(this->GlobalGenerator->GetSystemVersion(), "10.0")) {
    cmValue desktopExtensionsVersion =
      this->GeneratorTarget->GetProperty("VS_DESKTOP_EXTENSIONS_VERSION");
    if (desktopExtensionsVersion) {
      this->WriteSinglePlatformExtension(e1, "WindowsDesktop",
                                         *desktopExtensionsVersion);
    }
    cmValue mobileExtensionsVersion =
      this->GeneratorTarget->GetProperty("VS_MOBILE_EXTENSIONS_VERSION");
    if (mobileExtensionsVersion) {
      this->WriteSinglePlatformExtension(e1, "WindowsMobile",
                                         *mobileExtensionsVersion);
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteSinglePlatformExtension(
  Elem& e1, std::string const& extension, std::string const& version)
{
  const std::string s =
    cmStrCat("$([Microsoft.Build.Utilities.ToolLocationHelper]"
             "::GetPlatformExtensionSDKLocation(`",
             extension, ", Version=", version,
             "`, $(TargetPlatformIdentifier), $(TargetPlatformVersion), null, "
             "$(ExtensionSDKDirectoryRoot), null))"
             "\\DesignTime\\CommonConfiguration\\Neutral\\",
             extension, ".props");

  Elem e2(e1, "Import");
  e2.Attribute("Project", s);
  e2.Attribute("Condition", cmStrCat("exists('", s, "')"));
}

void cmVisualStudio10TargetGenerator::WriteSDKReferences(Elem& e0)
{
  cmList sdkReferences;
  std::unique_ptr<Elem> spe1;
  if (cmValue vsSDKReferences =
        this->GeneratorTarget->GetProperty("VS_SDK_REFERENCES")) {
    sdkReferences.assign(*vsSDKReferences);
    spe1 = cm::make_unique<Elem>(e0, "ItemGroup");
    for (auto const& ri : sdkReferences) {
      Elem(*spe1, "SDKReference").Attribute("Include", ri);
    }
  }

  // This only applies to Windows 10 apps
  if (this->GlobalGenerator->TargetsWindowsStore() &&
      cmHasLiteralPrefix(this->GlobalGenerator->GetSystemVersion(), "10.0")) {
    cmValue desktopExtensionsVersion =
      this->GeneratorTarget->GetProperty("VS_DESKTOP_EXTENSIONS_VERSION");
    cmValue mobileExtensionsVersion =
      this->GeneratorTarget->GetProperty("VS_MOBILE_EXTENSIONS_VERSION");
    cmValue iotExtensionsVersion =
      this->GeneratorTarget->GetProperty("VS_IOT_EXTENSIONS_VERSION");

    if (desktopExtensionsVersion || mobileExtensionsVersion ||
        iotExtensionsVersion) {
      if (!spe1) {
        spe1 = cm::make_unique<Elem>(e0, "ItemGroup");
      }
      if (desktopExtensionsVersion) {
        this->WriteSingleSDKReference(*spe1, "WindowsDesktop",
                                      *desktopExtensionsVersion);
      }
      if (mobileExtensionsVersion) {
        this->WriteSingleSDKReference(*spe1, "WindowsMobile",
                                      *mobileExtensionsVersion);
      }
      if (iotExtensionsVersion) {
        this->WriteSingleSDKReference(*spe1, "WindowsIoT",
                                      *iotExtensionsVersion);
      }
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteSingleSDKReference(
  Elem& e1, std::string const& extension, std::string const& version)
{
  Elem(e1, "SDKReference")
    .Attribute("Include", cmStrCat(extension, ", Version=", version));
}

namespace {
std::string ComputeCertificateThumbprint(const std::string& source)
{
  std::string thumbprint;

  CRYPT_INTEGER_BLOB cryptBlob;
  HCERTSTORE certStore = nullptr;
  PCCERT_CONTEXT certContext = nullptr;

  HANDLE certFile = CreateFileW(
    cmsys::Encoding::ToWide(source.c_str()).c_str(), GENERIC_READ,
    FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

  if (certFile != INVALID_HANDLE_VALUE && certFile != nullptr) {
    DWORD fileSize = GetFileSize(certFile, nullptr);
    if (fileSize != INVALID_FILE_SIZE) {
      auto certData = cm::make_unique<BYTE[]>(fileSize);
      if (certData != nullptr) {
        DWORD dwRead = 0;
        if (ReadFile(certFile, certData.get(), fileSize, &dwRead, nullptr)) {
          cryptBlob.cbData = fileSize;
          cryptBlob.pbData = certData.get();

          // Verify that this is a valid cert
          if (PFXIsPFXBlob(&cryptBlob)) {
            // Open the certificate as a store
            certStore =
              PFXImportCertStore(&cryptBlob, nullptr, CRYPT_EXPORTABLE);
            if (certStore != nullptr) {
              // There should only be 1 cert.
              certContext =
                CertEnumCertificatesInStore(certStore, certContext);
              if (certContext != nullptr) {
                // The hash is 20 bytes
                BYTE hashData[20];
                DWORD hashLength = 20;

                // Buffer to print the hash. Each byte takes 2 chars +
                // terminating character
                char hashPrint[41];
                char* pHashPrint = hashPrint;
                // Get the hash property from the certificate
                if (CertGetCertificateContextProperty(
                      certContext, CERT_HASH_PROP_ID, hashData, &hashLength)) {
                  for (DWORD i = 0; i < hashLength; i++) {
                    // Convert each byte to hexadecimal
                    snprintf(pHashPrint, 3, "%02X", hashData[i]);
                    pHashPrint += 2;
                  }
                  *pHashPrint = '\0';
                  thumbprint = hashPrint;
                }
                CertFreeCertificateContext(certContext);
              }
              CertCloseStore(certStore, 0);
            }
          }
        }
      }
    }
    CloseHandle(certFile);
  }

  return thumbprint;
}
}

void cmVisualStudio10TargetGenerator::WriteWinRTPackageCertificateKeyFile(
  Elem& e0)
{
  if ((this->GlobalGenerator->TargetsWindowsStore() ||
       this->GlobalGenerator->TargetsWindowsPhone()) &&
      (cmStateEnums::EXECUTABLE == this->GeneratorTarget->GetType())) {
    std::string pfxFile;
    for (cmGeneratorTarget::AllConfigSource const& source :
         this->GeneratorTarget->GetAllConfigSources()) {
      if (source.Kind == cmGeneratorTarget::SourceKindCertificate) {
        pfxFile = this->ConvertPath(source.Source->GetFullPath(), false);
        ConvertToWindowsSlash(pfxFile);
        break;
      }
    }

    if (this->IsMissingFiles &&
        !(this->GlobalGenerator->TargetsWindowsPhone() &&
          this->GlobalGenerator->GetSystemVersion() == "8.0"_s)) {
      // Move the manifest to a project directory to avoid clashes
      std::string artifactDir =
        this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
      ConvertToWindowsSlash(artifactDir);
      Elem e1(e0, "PropertyGroup");
      e1.Element("AppxPackageArtifactsDir", cmStrCat(artifactDir, '\\'));
      std::string resourcePriFile =
        cmStrCat(this->DefaultArtifactDir, "/resources.pri");
      ConvertToWindowsSlash(resourcePriFile);
      e1.Element("ProjectPriFullPath", resourcePriFile);

      // If we are missing files and we don't have a certificate and
      // aren't targeting WP8.0, add a default certificate
      if (pfxFile.empty()) {
        std::string templateFolder =
          cmStrCat(cmSystemTools::GetCMakeRoot(), "/Templates/Windows");
        pfxFile =
          cmStrCat(this->DefaultArtifactDir, "/Windows_TemporaryKey.pfx");
        cmSystemTools::CopyAFile(
          cmStrCat(templateFolder, "/Windows_TemporaryKey.pfx"), pfxFile,
          false);
        ConvertToWindowsSlash(pfxFile);
        this->AddedFiles.push_back(pfxFile);
        this->AddedDefaultCertificate = true;
      }

      e1.Element("PackageCertificateKeyFile", pfxFile);
      std::string thumb = ComputeCertificateThumbprint(pfxFile);
      if (!thumb.empty()) {
        e1.Element("PackageCertificateThumbprint", thumb);
      }
    } else if (!pfxFile.empty()) {
      Elem e1(e0, "PropertyGroup");
      e1.Element("PackageCertificateKeyFile", pfxFile);
      std::string thumb = ComputeCertificateThumbprint(pfxFile);
      if (!thumb.empty()) {
        e1.Element("PackageCertificateThumbprint", thumb);
      }
    }
  }
}

void cmVisualStudio10TargetGenerator::ClassifyAllConfigSources()
{
  for (cmGeneratorTarget::AllConfigSource const& source :
       this->GeneratorTarget->GetAllConfigSources()) {
    this->ClassifyAllConfigSource(source);
  }
}

void cmVisualStudio10TargetGenerator::ClassifyAllConfigSource(
  cmGeneratorTarget::AllConfigSource const& acs)
{
  switch (acs.Kind) {
    case cmGeneratorTarget::SourceKindResx: {
      // Build and save the name of the corresponding .h file
      // This relationship will be used later when building the project files.
      // Both names would have been auto generated from Visual Studio
      // where the user supplied the file name and Visual Studio
      // appended the suffix.
      std::string resx = acs.Source->ResolveFullPath();
      std::string hFileName =
        cmStrCat(resx.substr(0, resx.find_last_of('.')), ".h");
      this->ExpectedResxHeaders.insert(hFileName);
    } break;
    case cmGeneratorTarget::SourceKindXaml: {
      // Build and save the name of the corresponding .h and .cpp file
      // This relationship will be used later when building the project files.
      // Both names would have been auto generated from Visual Studio
      // where the user supplied the file name and Visual Studio
      // appended the suffix.
      std::string xaml = acs.Source->ResolveFullPath();
      std::string hFileName = cmStrCat(xaml, ".h");
      std::string cppFileName = cmStrCat(xaml, ".cpp");
      this->ExpectedXamlHeaders.insert(hFileName);
      this->ExpectedXamlSources.insert(cppFileName);
    } break;
    default:
      break;
  }
}

bool cmVisualStudio10TargetGenerator::IsResxHeader(
  const std::string& headerFile)
{
  return this->ExpectedResxHeaders.count(headerFile) > 0;
}

bool cmVisualStudio10TargetGenerator::IsXamlHeader(
  const std::string& headerFile)
{
  return this->ExpectedXamlHeaders.count(headerFile) > 0;
}

bool cmVisualStudio10TargetGenerator::IsXamlSource(
  const std::string& sourceFile)
{
  return this->ExpectedXamlSources.count(sourceFile) > 0;
}

void cmVisualStudio10TargetGenerator::WriteApplicationTypeSettings(Elem& e1)
{
  cmGlobalVisualStudio10Generator* gg = this->GlobalGenerator;
  bool isAppContainer = false;
  bool const isWindowsPhone = this->GlobalGenerator->TargetsWindowsPhone();
  bool const isWindowsStore = this->GlobalGenerator->TargetsWindowsStore();
  bool const isAndroid = this->GlobalGenerator->TargetsAndroid();
  std::string const& rev = this->GlobalGenerator->GetApplicationTypeRevision();
  if (isWindowsPhone || isWindowsStore) {
    e1.Element("ApplicationType",
               (isWindowsPhone ? "Windows Phone" : "Windows Store"));
    e1.Element("DefaultLanguage", "en-US");
    if (rev == "10.0"_s) {
      e1.Element("ApplicationTypeRevision", rev);
      // Visual Studio 14.0 is necessary for building 10.0 apps
      e1.Element("MinimumVisualStudioVersion", "14.0");

      if (this->GeneratorTarget->GetType() < cmStateEnums::UTILITY) {
        isAppContainer = true;
      }
    } else if (rev == "8.1"_s) {
      e1.Element("ApplicationTypeRevision", rev);
      // Visual Studio 12.0 is necessary for building 8.1 apps
      e1.Element("MinimumVisualStudioVersion", "12.0");

      if (this->GeneratorTarget->GetType() < cmStateEnums::UTILITY) {
        isAppContainer = true;
      }
    } else if (rev == "8.0"_s) {
      e1.Element("ApplicationTypeRevision", rev);
      // Visual Studio 11.0 is necessary for building 8.0 apps
      e1.Element("MinimumVisualStudioVersion", "11.0");

      if (isWindowsStore &&
          this->GeneratorTarget->GetType() < cmStateEnums::UTILITY) {
        isAppContainer = true;
      } else if (isWindowsPhone &&
                 this->GeneratorTarget->GetType() ==
                   cmStateEnums::EXECUTABLE) {
        e1.Element("XapOutputs", "true");
        e1.Element("XapFilename",
                   cmStrCat(this->Name, "_$(Configuration)_$(Platform).xap"));
      }
    }
  } else if (isAndroid) {
    e1.Element("ApplicationType", "Android");
    e1.Element("ApplicationTypeRevision",
               gg->GetAndroidApplicationTypeRevision());
  }
  if (isAppContainer) {
    e1.Element("AppContainerApplication", "true");
  } else if (!isAndroid) {
    if (this->Platform == "ARM64"_s) {
      e1.Element("WindowsSDKDesktopARM64Support", "true");
    } else if (this->Platform == "ARM"_s) {
      e1.Element("WindowsSDKDesktopARMSupport", "true");
    }
  }
  std::string const& targetPlatformVersion =
    gg->GetWindowsTargetPlatformVersion();
  if (!targetPlatformVersion.empty()) {
    e1.Element("WindowsTargetPlatformVersion", targetPlatformVersion);
  }
  cmValue targetPlatformMinVersion = this->GeneratorTarget->GetProperty(
    "VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION");
  if (targetPlatformMinVersion) {
    e1.Element("WindowsTargetPlatformMinVersion", *targetPlatformMinVersion);
  } else if (isWindowsStore && rev == "10.0"_s) {
    // If the min version is not set, then use the TargetPlatformVersion
    if (!targetPlatformVersion.empty()) {
      e1.Element("WindowsTargetPlatformMinVersion", targetPlatformVersion);
    }
  }

  // Added IoT Startup Task support
  if (this->GeneratorTarget->GetPropertyAsBool("VS_IOT_STARTUP_TASK")) {
    e1.Element("ContainsStartupTask", "true");
  }
}

void cmVisualStudio10TargetGenerator::VerifyNecessaryFiles()
{
  // For Windows and Windows Phone executables, we will assume that if a
  // manifest is not present that we need to add all the necessary files
  if (this->GeneratorTarget->GetType() == cmStateEnums::EXECUTABLE) {
    std::vector<cmGeneratorTarget::AllConfigSource> manifestSources =
      this->GeneratorTarget->GetAllConfigSources(
        cmGeneratorTarget::SourceKindAppManifest);
    std::string const& v = this->GlobalGenerator->GetSystemVersion();
    if (this->GlobalGenerator->TargetsWindowsPhone()) {
      if (v == "8.0"_s) {
        // Look through the sources for WMAppManifest.xml
        bool foundManifest = false;
        for (cmGeneratorTarget::AllConfigSource const& source :
             this->GeneratorTarget->GetAllConfigSources()) {
          if (source.Kind == cmGeneratorTarget::SourceKindExtra &&
              "wmappmanifest.xml" ==
                cmSystemTools::LowerCase(
                  source.Source->GetLocation().GetName())) {
            foundManifest = true;
            break;
          }
        }
        if (!foundManifest) {
          this->IsMissingFiles = true;
        }
      } else if (v == "8.1"_s) {
        if (manifestSources.empty()) {
          this->IsMissingFiles = true;
        }
      }
    } else if (this->GlobalGenerator->TargetsWindowsStore()) {
      if (manifestSources.empty()) {
        if (v == "8.0"_s) {
          this->IsMissingFiles = true;
        } else if (v == "8.1"_s || cmHasLiteralPrefix(v, "10.0")) {
          this->IsMissingFiles = true;
        }
      }
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteMissingFiles(Elem& e1)
{
  std::string const& v = this->GlobalGenerator->GetSystemVersion();
  if (this->GlobalGenerator->TargetsWindowsPhone()) {
    if (v == "8.0"_s) {
      this->WriteMissingFilesWP80(e1);
    } else if (v == "8.1"_s) {
      this->WriteMissingFilesWP81(e1);
    }
  } else if (this->GlobalGenerator->TargetsWindowsStore()) {
    if (v == "8.0"_s) {
      this->WriteMissingFilesWS80(e1);
    } else if (v == "8.1"_s) {
      this->WriteMissingFilesWS81(e1);
    } else if (cmHasLiteralPrefix(v, "10.0")) {
      this->WriteMissingFilesWS10_0(e1);
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteMissingFilesWP80(Elem& e1)
{
  std::string templateFolder =
    cmStrCat(cmSystemTools::GetCMakeRoot(), "/Templates/Windows");

  // For WP80, the manifest needs to be in the same folder as the project
  // this can cause an overwrite problem if projects aren't organized in
  // folders
  std::string manifestFile = cmStrCat(
    this->LocalGenerator->GetCurrentBinaryDirectory(), "/WMAppManifest.xml");
  std::string artifactDir =
    this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
  ConvertToWindowsSlash(artifactDir);
  std::string artifactDirXML = cmVS10EscapeXML(artifactDir);
  const std::string& targetNameXML = cmVS10EscapeXML(GetTargetOutputName());

  cmGeneratedFileStream fout(manifestFile);
  fout.SetCopyIfDifferent(true);

  /* clang-format off */
  fout <<
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<Deployment"
    " xmlns=\"http://schemas.microsoft.com/windowsphone/2012/deployment\""
    " AppPlatformVersion=\"8.0\">\n"
    "\t<DefaultLanguage xmlns=\"\" code=\"en-US\"/>\n"
    "\t<App xmlns=\"\" ProductID=\"{" << this->GUID << "}\""
    " Title=\"CMake Test Program\" RuntimeType=\"Modern Native\""
    " Version=\"1.0.0.0\" Genre=\"apps.normal\"  Author=\"CMake\""
    " Description=\"Default CMake App\" Publisher=\"CMake\""
    " PublisherID=\"{" << this->GUID << "}\">\n"
    "\t\t<IconPath IsRelative=\"true\" IsResource=\"false\">"
       << artifactDirXML << "\\ApplicationIcon.png</IconPath>\n"
    "\t\t<Capabilities/>\n"
    "\t\t<Tasks>\n"
    "\t\t\t<DefaultTask Name=\"_default\""
    " ImagePath=\"" << targetNameXML << ".exe\" ImageParams=\"\" />\n"
    "\t\t</Tasks>\n"
    "\t\t<Tokens>\n"
    "\t\t\t<PrimaryToken TokenID=\"" << targetNameXML << "Token\""
    " TaskName=\"_default\">\n"
    "\t\t\t\t<TemplateFlip>\n"
    "\t\t\t\t\t<SmallImageURI IsRelative=\"true\" IsResource=\"false\">"
       << artifactDirXML << "\\SmallLogo.png</SmallImageURI>\n"
    "\t\t\t\t\t<Count>0</Count>\n"
    "\t\t\t\t\t<BackgroundImageURI IsRelative=\"true\" IsResource=\"false\">"
       << artifactDirXML << "\\Logo.png</BackgroundImageURI>\n"
    "\t\t\t\t</TemplateFlip>\n"
    "\t\t\t</PrimaryToken>\n"
    "\t\t</Tokens>\n"
    "\t\t<ScreenResolutions>\n"
    "\t\t\t<ScreenResolution Name=\"ID_RESOLUTION_WVGA\" />\n"
    "\t\t</ScreenResolutions>\n"
    "\t</App>\n"
    "</Deployment>\n";
  /* clang-format on */

  std::string sourceFile = this->ConvertPath(manifestFile, false);
  ConvertToWindowsSlash(sourceFile);
  {
    Elem e2(e1, "Xml");
    e2.Attribute("Include", sourceFile);
    e2.Element("SubType", "Designer");
  }
  this->AddedFiles.push_back(sourceFile);

  std::string smallLogo = cmStrCat(this->DefaultArtifactDir, "/SmallLogo.png");
  cmSystemTools::CopyAFile(cmStrCat(templateFolder, "/SmallLogo.png"),
                           smallLogo, false);
  ConvertToWindowsSlash(smallLogo);
  Elem(e1, "Image").Attribute("Include", smallLogo);
  this->AddedFiles.push_back(smallLogo);

  std::string logo = cmStrCat(this->DefaultArtifactDir, "/Logo.png");
  cmSystemTools::CopyAFile(cmStrCat(templateFolder, "/Logo.png"), logo, false);
  ConvertToWindowsSlash(logo);
  Elem(e1, "Image").Attribute("Include", logo);
  this->AddedFiles.push_back(logo);

  std::string applicationIcon =
    cmStrCat(this->DefaultArtifactDir, "/ApplicationIcon.png");
  cmSystemTools::CopyAFile(cmStrCat(templateFolder, "/ApplicationIcon.png"),
                           applicationIcon, false);
  ConvertToWindowsSlash(applicationIcon);
  Elem(e1, "Image").Attribute("Include", applicationIcon);
  this->AddedFiles.push_back(applicationIcon);
}

void cmVisualStudio10TargetGenerator::WriteMissingFilesWP81(Elem& e1)
{
  std::string manifestFile =
    cmStrCat(this->DefaultArtifactDir, "/package.appxManifest");
  std::string artifactDir =
    this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
  ConvertToWindowsSlash(artifactDir);
  std::string artifactDirXML = cmVS10EscapeXML(artifactDir);
  const std::string& targetNameXML = cmVS10EscapeXML(GetTargetOutputName());

  cmGeneratedFileStream fout(manifestFile);
  fout.SetCopyIfDifferent(true);

  /* clang-format off */
  fout <<
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<Package xmlns=\"http://schemas.microsoft.com/appx/2010/manifest\""
    " xmlns:m2=\"http://schemas.microsoft.com/appx/2013/manifest\""
    " xmlns:mp=\"http://schemas.microsoft.com/appx/2014/phone/manifest\">\n"
    "\t<Identity Name=\"" << this->GUID << "\" Publisher=\"CN=CMake\""
    " Version=\"1.0.0.0\" />\n"
    "\t<mp:PhoneIdentity PhoneProductId=\"" << this->GUID << "\""
    " PhonePublisherId=\"00000000-0000-0000-0000-000000000000\"/>\n"
    "\t<Properties>\n"
    "\t\t<DisplayName>" << targetNameXML << "</DisplayName>\n"
    "\t\t<PublisherDisplayName>CMake</PublisherDisplayName>\n"
    "\t\t<Logo>" << artifactDirXML << "\\StoreLogo.png</Logo>\n"
    "\t</Properties>\n"
    "\t<Prerequisites>\n"
    "\t\t<OSMinVersion>6.3.1</OSMinVersion>\n"
    "\t\t<OSMaxVersionTested>6.3.1</OSMaxVersionTested>\n"
    "\t</Prerequisites>\n"
    "\t<Resources>\n"
    "\t\t<Resource Language=\"x-generate\" />\n"
    "\t</Resources>\n"
    "\t<Applications>\n"
    "\t\t<Application Id=\"App\""
    " Executable=\"" << targetNameXML << ".exe\""
    " EntryPoint=\"" << targetNameXML << ".App\">\n"
    "\t\t\t<m2:VisualElements\n"
    "\t\t\t\tDisplayName=\"" << targetNameXML << "\"\n"
    "\t\t\t\tDescription=\"" << targetNameXML << "\"\n"
    "\t\t\t\tBackgroundColor=\"#336699\"\n"
    "\t\t\t\tForegroundText=\"light\"\n"
    "\t\t\t\tSquare150x150Logo=\"" << artifactDirXML << "\\Logo.png\"\n"
    "\t\t\t\tSquare30x30Logo=\"" << artifactDirXML << "\\SmallLogo.png\">\n"
    "\t\t\t\t<m2:DefaultTile ShortName=\"" << targetNameXML << "\">\n"
    "\t\t\t\t\t<m2:ShowNameOnTiles>\n"
    "\t\t\t\t\t\t<m2:ShowOn Tile=\"square150x150Logo\" />\n"
    "\t\t\t\t\t</m2:ShowNameOnTiles>\n"
    "\t\t\t\t</m2:DefaultTile>\n"
    "\t\t\t\t<m2:SplashScreen"
    " Image=\"" << artifactDirXML << "\\SplashScreen.png\" />\n"
    "\t\t\t</m2:VisualElements>\n"
    "\t\t</Application>\n"
    "\t</Applications>\n"
    "</Package>\n";
  /* clang-format on */

  this->WriteCommonMissingFiles(e1, manifestFile);
}

void cmVisualStudio10TargetGenerator::WriteMissingFilesWS80(Elem& e1)
{
  std::string manifestFile =
    cmStrCat(this->DefaultArtifactDir, "/package.appxManifest");
  std::string artifactDir =
    this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
  ConvertToWindowsSlash(artifactDir);
  std::string artifactDirXML = cmVS10EscapeXML(artifactDir);
  const std::string& targetNameXML = cmVS10EscapeXML(GetTargetOutputName());

  cmGeneratedFileStream fout(manifestFile);
  fout.SetCopyIfDifferent(true);

  /* clang-format off */
  fout <<
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<Package xmlns=\"http://schemas.microsoft.com/appx/2010/manifest\">\n"
    "\t<Identity Name=\"" << this->GUID << "\" Publisher=\"CN=CMake\""
    " Version=\"1.0.0.0\" />\n"
    "\t<Properties>\n"
    "\t\t<DisplayName>" << targetNameXML << "</DisplayName>\n"
    "\t\t<PublisherDisplayName>CMake</PublisherDisplayName>\n"
    "\t\t<Logo>" << artifactDirXML << "\\StoreLogo.png</Logo>\n"
    "\t</Properties>\n"
    "\t<Prerequisites>\n"
    "\t\t<OSMinVersion>6.2.1</OSMinVersion>\n"
    "\t\t<OSMaxVersionTested>6.2.1</OSMaxVersionTested>\n"
    "\t</Prerequisites>\n"
    "\t<Resources>\n"
    "\t\t<Resource Language=\"x-generate\" />\n"
    "\t</Resources>\n"
    "\t<Applications>\n"
    "\t\t<Application Id=\"App\""
    " Executable=\"" << targetNameXML << ".exe\""
    " EntryPoint=\"" << targetNameXML << ".App\">\n"
    "\t\t\t<VisualElements"
    " DisplayName=\"" << targetNameXML << "\""
    " Description=\"" << targetNameXML << "\""
    " BackgroundColor=\"#336699\" ForegroundText=\"light\""
    " Logo=\"" << artifactDirXML << "\\Logo.png\""
    " SmallLogo=\"" << artifactDirXML << "\\SmallLogo.png\">\n"
    "\t\t\t\t<DefaultTile ShowName=\"allLogos\""
    " ShortName=\"" << targetNameXML << "\" />\n"
    "\t\t\t\t<SplashScreen"
    " Image=\"" << artifactDirXML << "\\SplashScreen.png\" />\n"
    "\t\t\t</VisualElements>\n"
    "\t\t</Application>\n"
    "\t</Applications>\n"
    "</Package>\n";
  /* clang-format on */

  this->WriteCommonMissingFiles(e1, manifestFile);
}

void cmVisualStudio10TargetGenerator::WriteMissingFilesWS81(Elem& e1)
{
  std::string manifestFile =
    cmStrCat(this->DefaultArtifactDir, "/package.appxManifest");
  std::string artifactDir =
    this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
  ConvertToWindowsSlash(artifactDir);
  std::string artifactDirXML = cmVS10EscapeXML(artifactDir);
  const std::string& targetNameXML = cmVS10EscapeXML(GetTargetOutputName());

  cmGeneratedFileStream fout(manifestFile);
  fout.SetCopyIfDifferent(true);

  /* clang-format off */
  fout <<
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<Package xmlns=\"http://schemas.microsoft.com/appx/2010/manifest\""
    " xmlns:m2=\"http://schemas.microsoft.com/appx/2013/manifest\">\n"
    "\t<Identity Name=\"" << this->GUID << "\" Publisher=\"CN=CMake\""
    " Version=\"1.0.0.0\" />\n"
    "\t<Properties>\n"
    "\t\t<DisplayName>" << targetNameXML << "</DisplayName>\n"
    "\t\t<PublisherDisplayName>CMake</PublisherDisplayName>\n"
    "\t\t<Logo>" << artifactDirXML << "\\StoreLogo.png</Logo>\n"
    "\t</Properties>\n"
    "\t<Prerequisites>\n"
    "\t\t<OSMinVersion>6.3</OSMinVersion>\n"
    "\t\t<OSMaxVersionTested>6.3</OSMaxVersionTested>\n"
    "\t</Prerequisites>\n"
    "\t<Resources>\n"
    "\t\t<Resource Language=\"x-generate\" />\n"
    "\t</Resources>\n"
    "\t<Applications>\n"
    "\t\t<Application Id=\"App\""
    " Executable=\"" << targetNameXML << ".exe\""
    " EntryPoint=\"" << targetNameXML << ".App\">\n"
    "\t\t\t<m2:VisualElements\n"
    "\t\t\t\tDisplayName=\"" << targetNameXML << "\"\n"
    "\t\t\t\tDescription=\"" << targetNameXML << "\"\n"
    "\t\t\t\tBackgroundColor=\"#336699\"\n"
    "\t\t\t\tForegroundText=\"light\"\n"
    "\t\t\t\tSquare150x150Logo=\"" << artifactDirXML << "\\Logo.png\"\n"
    "\t\t\t\tSquare30x30Logo=\"" << artifactDirXML << "\\SmallLogo.png\">\n"
    "\t\t\t\t<m2:DefaultTile ShortName=\"" << targetNameXML << "\">\n"
    "\t\t\t\t\t<m2:ShowNameOnTiles>\n"
    "\t\t\t\t\t\t<m2:ShowOn Tile=\"square150x150Logo\" />\n"
    "\t\t\t\t\t</m2:ShowNameOnTiles>\n"
    "\t\t\t\t</m2:DefaultTile>\n"
    "\t\t\t\t<m2:SplashScreen"
    " Image=\"" << artifactDirXML << "\\SplashScreen.png\" />\n"
    "\t\t\t</m2:VisualElements>\n"
    "\t\t</Application>\n"
    "\t</Applications>\n"
    "</Package>\n";
  /* clang-format on */

  this->WriteCommonMissingFiles(e1, manifestFile);
}

void cmVisualStudio10TargetGenerator::WriteMissingFilesWS10_0(Elem& e1)
{
  std::string manifestFile =
    cmStrCat(this->DefaultArtifactDir, "/package.appxManifest");
  std::string artifactDir =
    this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
  ConvertToWindowsSlash(artifactDir);
  std::string artifactDirXML = cmVS10EscapeXML(artifactDir);
  const std::string& targetNameXML = cmVS10EscapeXML(GetTargetOutputName());

  cmGeneratedFileStream fout(manifestFile);
  fout.SetCopyIfDifferent(true);

  /* clang-format off */
  fout <<
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<Package\n\t"
    "xmlns=\"http://schemas.microsoft.com/appx/manifest/foundation/windows10\""
    "\txmlns:mp=\"http://schemas.microsoft.com/appx/2014/phone/manifest\"\n"
    "\txmlns:uap=\"http://schemas.microsoft.com/appx/manifest/uap/windows10\""
    "\n\tIgnorableNamespaces=\"uap mp\">\n\n"
    "\t<Identity Name=\"" << this->GUID << "\" Publisher=\"CN=CMake\""
    " Version=\"1.0.0.0\" />\n"
    "\t<mp:PhoneIdentity PhoneProductId=\"" << this->GUID <<
    "\" PhonePublisherId=\"00000000-0000-0000-0000-000000000000\"/>\n"
    "\t<Properties>\n"
    "\t\t<DisplayName>" << targetNameXML << "</DisplayName>\n"
    "\t\t<PublisherDisplayName>CMake</PublisherDisplayName>\n"
    "\t\t<Logo>" << artifactDirXML << "\\StoreLogo.png</Logo>\n"
    "\t</Properties>\n"
    "\t<Dependencies>\n"
    "\t\t<TargetDeviceFamily Name=\"Windows.Universal\" "
    "MinVersion=\"10.0.0.0\" MaxVersionTested=\"10.0.0.0\" />\n"
    "\t</Dependencies>\n"

    "\t<Resources>\n"
    "\t\t<Resource Language=\"x-generate\" />\n"
    "\t</Resources>\n"
    "\t<Applications>\n"
    "\t\t<Application Id=\"App\""
    " Executable=\"" << targetNameXML << ".exe\""
    " EntryPoint=\"" << targetNameXML << ".App\">\n"
    "\t\t\t<uap:VisualElements\n"
    "\t\t\t\tDisplayName=\"" << targetNameXML << "\"\n"
    "\t\t\t\tDescription=\"" << targetNameXML << "\"\n"
    "\t\t\t\tBackgroundColor=\"#336699\"\n"
    "\t\t\t\tSquare150x150Logo=\"" << artifactDirXML << "\\Logo.png\"\n"
    "\t\t\t\tSquare44x44Logo=\"" << artifactDirXML <<
    "\\SmallLogo44x44.png\">\n"
    "\t\t\t\t<uap:SplashScreen"
    " Image=\"" << artifactDirXML << "\\SplashScreen.png\" />\n"
    "\t\t\t</uap:VisualElements>\n"
    "\t\t</Application>\n"
    "\t</Applications>\n"
    "</Package>\n";
  /* clang-format on */

  this->WriteCommonMissingFiles(e1, manifestFile);
}

void cmVisualStudio10TargetGenerator::WriteCommonMissingFiles(
  Elem& e1, const std::string& manifestFile)
{
  std::string templateFolder =
    cmStrCat(cmSystemTools::GetCMakeRoot(), "/Templates/Windows");

  std::string sourceFile = this->ConvertPath(manifestFile, false);
  ConvertToWindowsSlash(sourceFile);
  {
    Elem e2(e1, "AppxManifest");
    e2.Attribute("Include", sourceFile);
    e2.Element("SubType", "Designer");
  }
  this->AddedFiles.push_back(sourceFile);

  std::string smallLogo = cmStrCat(this->DefaultArtifactDir, "/SmallLogo.png");
  cmSystemTools::CopyAFile(cmStrCat(templateFolder, "/SmallLogo.png"),
                           smallLogo, false);
  ConvertToWindowsSlash(smallLogo);
  Elem(e1, "Image").Attribute("Include", smallLogo);
  this->AddedFiles.push_back(smallLogo);

  std::string smallLogo44 =
    cmStrCat(this->DefaultArtifactDir, "/SmallLogo44x44.png");
  cmSystemTools::CopyAFile(cmStrCat(templateFolder, "/SmallLogo44x44.png"),
                           smallLogo44, false);
  ConvertToWindowsSlash(smallLogo44);
  Elem(e1, "Image").Attribute("Include", smallLogo44);
  this->AddedFiles.push_back(smallLogo44);

  std::string logo = cmStrCat(this->DefaultArtifactDir, "/Logo.png");
  cmSystemTools::CopyAFile(cmStrCat(templateFolder, "/Logo.png"), logo, false);
  ConvertToWindowsSlash(logo);
  Elem(e1, "Image").Attribute("Include", logo);
  this->AddedFiles.push_back(logo);

  std::string storeLogo = cmStrCat(this->DefaultArtifactDir, "/StoreLogo.png");
  cmSystemTools::CopyAFile(cmStrCat(templateFolder, "/StoreLogo.png"),
                           storeLogo, false);
  ConvertToWindowsSlash(storeLogo);
  Elem(e1, "Image").Attribute("Include", storeLogo);
  this->AddedFiles.push_back(storeLogo);

  std::string splashScreen =
    cmStrCat(this->DefaultArtifactDir, "/SplashScreen.png");
  cmSystemTools::CopyAFile(cmStrCat(templateFolder, "/SplashScreen.png"),
                           splashScreen, false);
  ConvertToWindowsSlash(splashScreen);
  Elem(e1, "Image").Attribute("Include", splashScreen);
  this->AddedFiles.push_back(splashScreen);

  if (this->AddedDefaultCertificate) {
    // This file has already been added to the build so don't copy it
    std::string keyFile =
      cmStrCat(this->DefaultArtifactDir, "/Windows_TemporaryKey.pfx");
    ConvertToWindowsSlash(keyFile);
    Elem(e1, "None").Attribute("Include", keyFile);
  }
}

bool cmVisualStudio10TargetGenerator::ForceOld(const std::string& source) const
{
  HANDLE h =
    CreateFileW(cmSystemTools::ConvertToWindowsExtendedPath(source).c_str(),
                FILE_WRITE_ATTRIBUTES, FILE_SHARE_WRITE, 0, OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS, 0);
  if (!h) {
    return false;
  }

  FILETIME const ftime_20010101 = { 3365781504u, 29389701u };
  if (!SetFileTime(h, &ftime_20010101, &ftime_20010101, &ftime_20010101)) {
    CloseHandle(h);
    return false;
  }

  CloseHandle(h);
  return true;
}

void cmVisualStudio10TargetGenerator::GetCSharpSourceProperties(
  cmSourceFile const* sf, std::map<std::string, std::string>& tags)
{
  if (this->ProjectType == VsProjectType::csproj) {
    const cmPropertyMap& props = sf->GetProperties();
    for (const std::string& p : props.GetKeys()) {
      static const cm::string_view propNamePrefix = "VS_CSHARP_";
      if (cmHasPrefix(p, propNamePrefix)) {
        std::string tagName = p.substr(propNamePrefix.length());
        if (!tagName.empty()) {
          cmValue val = props.GetPropertyValue(p);
          if (cmNonempty(val)) {
            tags[tagName] = *val;
          } else {
            tags.erase(tagName);
          }
        }
      }
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteCSharpSourceProperties(
  Elem& e2, const std::map<std::string, std::string>& tags)
{
  for (const auto& i : tags) {
    e2.Element(i.first, i.second);
  }
}

std::string cmVisualStudio10TargetGenerator::GetCSharpSourceLink(
  cmSourceFile const* source)
{
  // For out of source files, we first check if a matching source group
  // for this file exists, otherwise we check if the path relative to current
  // source- or binary-dir is used within the link and return that.
  // In case of .cs files we can't do that automatically for files in the
  // binary directory, because this leads to compilation errors.
  std::string link;
  std::string sourceGroupedFile;
  std::string const& fullFileName = source->GetFullPath();
  std::string const& srcDir = this->Makefile->GetCurrentSourceDirectory();
  std::string const& binDir = this->Makefile->GetCurrentBinaryDirectory();
  // unfortunately we have to copy the source groups, because
  // FindSourceGroup uses a regex which is modifying the group
  std::vector<cmSourceGroup> sourceGroups = this->Makefile->GetSourceGroups();
  cmSourceGroup* sourceGroup =
    this->Makefile->FindSourceGroup(fullFileName, sourceGroups);
  if (sourceGroup && !sourceGroup->GetFullName().empty()) {
    sourceGroupedFile =
      cmStrCat(sourceGroup->GetFullName(), '/',
               cmsys::SystemTools::GetFilenameName(fullFileName));
    cmsys::SystemTools::ConvertToUnixSlashes(sourceGroupedFile);
  }

  if (!sourceGroupedFile.empty() &&
      cmHasSuffix(fullFileName, sourceGroupedFile)) {
    link = sourceGroupedFile;
  } else if (cmHasPrefix(fullFileName, srcDir)) {
    link = fullFileName.substr(srcDir.length() + 1);
  } else if (!cmHasSuffix(fullFileName, ".cs") &&
             cmHasPrefix(fullFileName, binDir)) {
    link = fullFileName.substr(binDir.length() + 1);
  } else if (cmValue l = source->GetProperty("VS_CSHARP_Link")) {
    link = *l;
  }

  ConvertToWindowsSlash(link);
  return link;
}

std::string cmVisualStudio10TargetGenerator::GetCMakeFilePath(
  const char* relativeFilePath) const
{
  // Always search in the standard modules location.
  std::string path =
    cmStrCat(cmSystemTools::GetCMakeRoot(), '/', relativeFilePath);
  ConvertToWindowsSlash(path);

  return path;
}

void cmVisualStudio10TargetGenerator::WriteStdOutEncodingUtf8(Elem& e1)
{
  if (this->GlobalGenerator->IsUtf8EncodingSupported()) {
    e1.Element("UseUtf8Encoding", "Always");
  } else if (this->GlobalGenerator->IsStdOutEncodingSupported()) {
    e1.Element("StdOutEncoding", "UTF-8");
  }
}

void cmVisualStudio10TargetGenerator::UpdateCache()
{
  std::vector<std::string> packageReferences;

  if (this->GeneratorTarget->HasPackageReferences()) {
    // Store a cache entry that later determines, if a package restore is
    // required.
    this->GeneratorTarget->Makefile->AddCacheDefinition(
      cmStrCat(this->GeneratorTarget->GetName(),
               "_REQUIRES_VS_PACKAGE_RESTORE"),
      "ON", "Value Computed by CMake", cmStateEnums::STATIC);
  } else {
    // If there are any dependencies that require package restore, inherit the
    // cache variable.
    cmGlobalGenerator::TargetDependSet const& unordered =
      this->GlobalGenerator->GetTargetDirectDepends(this->GeneratorTarget);
    using OrderedTargetDependSet =
      cmGlobalVisualStudioGenerator::OrderedTargetDependSet;
    OrderedTargetDependSet depends(unordered, CMAKE_CHECK_BUILD_SYSTEM_TARGET);

    for (cmGeneratorTarget const* dt : depends) {
      if (dt->HasPackageReferences()) {
        this->GeneratorTarget->Makefile->AddCacheDefinition(
          cmStrCat(this->GeneratorTarget->GetName(),
                   "_REQUIRES_VS_PACKAGE_RESTORE"),
          "ON", "Value Computed by CMake", cmStateEnums::STATIC);
      }
    }
  }
}
