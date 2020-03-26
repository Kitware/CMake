/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCommonTargetGenerator.h"

#include <set>
#include <sstream>
#include <utility>

#include "cmComputeLinkInformation.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalCommonGenerator.h"
#include "cmLinkLineComputer.h"
#include "cmLocalCommonGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmOutputConverter.h"
#include "cmSourceFile.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"

cmCommonTargetGenerator::cmCommonTargetGenerator(cmGeneratorTarget* gt)
  : GeneratorTarget(gt)
  , Makefile(gt->Makefile)
  , LocalCommonGenerator(
      static_cast<cmLocalCommonGenerator*>(gt->LocalGenerator))
  , GlobalCommonGenerator(static_cast<cmGlobalCommonGenerator*>(
      gt->LocalGenerator->GetGlobalGenerator()))
  , ConfigNames(LocalCommonGenerator->GetConfigNames())
{
}

cmCommonTargetGenerator::~cmCommonTargetGenerator() = default;

std::vector<std::string> const& cmCommonTargetGenerator::GetConfigNames() const
{
  return this->ConfigNames;
}

const char* cmCommonTargetGenerator::GetFeature(const std::string& feature,
                                                const std::string& config)
{
  return this->GeneratorTarget->GetFeature(feature, config);
}

void cmCommonTargetGenerator::AddModuleDefinitionFlag(
  cmLinkLineComputer* linkLineComputer, std::string& flags,
  const std::string& config)
{
  cmGeneratorTarget::ModuleDefinitionInfo const* mdi =
    this->GeneratorTarget->GetModuleDefinitionInfo(config);
  if (!mdi || mdi->DefFile.empty()) {
    return;
  }

  // TODO: Create a per-language flag variable.
  const char* defFileFlag =
    this->Makefile->GetDefinition("CMAKE_LINK_DEF_FILE_FLAG");
  if (!defFileFlag) {
    return;
  }

  // Append the flag and value.  Use ConvertToLinkReference to help
  // vs6's "cl -link" pass it to the linker.
  std::string flag =
    cmStrCat(defFileFlag,
             this->LocalCommonGenerator->ConvertToOutputFormat(
               linkLineComputer->ConvertToLinkReference(mdi->DefFile),
               cmOutputConverter::SHELL));
  this->LocalCommonGenerator->AppendFlags(flags, flag);
}

void cmCommonTargetGenerator::AppendFortranFormatFlags(
  std::string& flags, cmSourceFile const& source)
{
  const char* srcfmt = source.GetProperty("Fortran_FORMAT");
  cmOutputConverter::FortranFormat format =
    cmOutputConverter::GetFortranFormat(srcfmt);
  if (format == cmOutputConverter::FortranFormatNone) {
    const char* tgtfmt = this->GeneratorTarget->GetProperty("Fortran_FORMAT");
    format = cmOutputConverter::GetFortranFormat(tgtfmt);
  }
  const char* var = nullptr;
  switch (format) {
    case cmOutputConverter::FortranFormatFixed:
      var = "CMAKE_Fortran_FORMAT_FIXED_FLAG";
      break;
    case cmOutputConverter::FortranFormatFree:
      var = "CMAKE_Fortran_FORMAT_FREE_FLAG";
      break;
    default:
      break;
  }
  if (var) {
    this->LocalCommonGenerator->AppendFlags(
      flags, this->Makefile->GetSafeDefinition(var));
  }
}

std::string cmCommonTargetGenerator::GetFlags(const std::string& l,
                                              const std::string& config)
{
  auto i = this->Configs[config].FlagsByLanguage.find(l);
  if (i == this->Configs[config].FlagsByLanguage.end()) {
    std::string flags;

    this->LocalCommonGenerator->GetTargetCompileFlags(this->GeneratorTarget,
                                                      config, l, flags);

    ByLanguageMap::value_type entry(l, flags);
    i = this->Configs[config].FlagsByLanguage.insert(entry).first;
  }
  return i->second;
}

std::string cmCommonTargetGenerator::GetDefines(const std::string& l,
                                                const std::string& config)
{
  auto i = this->Configs[config].DefinesByLanguage.find(l);
  if (i == this->Configs[config].DefinesByLanguage.end()) {
    std::set<std::string> defines;
    this->LocalCommonGenerator->GetTargetDefines(this->GeneratorTarget, config,
                                                 l, defines);

    std::string definesString;
    this->LocalCommonGenerator->JoinDefines(defines, definesString, l);

    ByLanguageMap::value_type entry(l, definesString);
    i = this->Configs[config].DefinesByLanguage.insert(entry).first;
  }
  return i->second;
}

std::string cmCommonTargetGenerator::GetIncludes(std::string const& l,
                                                 const std::string& config)
{
  auto i = this->Configs[config].IncludesByLanguage.find(l);
  if (i == this->Configs[config].IncludesByLanguage.end()) {
    std::string includes;
    this->AddIncludeFlags(includes, l, config);
    ByLanguageMap::value_type entry(l, includes);
    i = this->Configs[config].IncludesByLanguage.insert(entry).first;
  }
  return i->second;
}

std::vector<std::string> cmCommonTargetGenerator::GetLinkedTargetDirectories(
  const std::string& config) const
{
  std::vector<std::string> dirs;
  std::set<cmGeneratorTarget const*> emitted;
  if (cmComputeLinkInformation* cli =
        this->GeneratorTarget->GetLinkInformation(config)) {
    cmComputeLinkInformation::ItemVector const& items = cli->GetItems();
    for (auto const& item : items) {
      cmGeneratorTarget const* linkee = item.Target;
      if (linkee &&
          !linkee->IsImported()
          // We can ignore the INTERFACE_LIBRARY items because
          // Target->GetLinkInformation already processed their
          // link interface and they don't have any output themselves.
          && linkee->GetType() != cmStateEnums::INTERFACE_LIBRARY &&
          emitted.insert(linkee).second) {
        cmLocalGenerator* lg = linkee->GetLocalGenerator();
        std::string di = cmStrCat(lg->GetCurrentBinaryDirectory(), '/',
                                  lg->GetTargetDirectory(linkee));
        dirs.push_back(std::move(di));
      }
    }
  }
  return dirs;
}

std::string cmCommonTargetGenerator::ComputeTargetCompilePDB(
  const std::string& config) const
{
  std::string compilePdbPath;
  if (this->GeneratorTarget->GetType() > cmStateEnums::OBJECT_LIBRARY) {
    return compilePdbPath;
  }

  compilePdbPath = this->GeneratorTarget->GetCompilePDBPath(config);
  if (compilePdbPath.empty()) {
    // Match VS default: `$(IntDir)vc$(PlatformToolsetVersion).pdb`.
    // A trailing slash tells the toolchain to add its default file name.
    compilePdbPath = this->GeneratorTarget->GetSupportDirectory();
    if (this->GlobalCommonGenerator->IsMultiConfig()) {
      compilePdbPath += "/";
      compilePdbPath += config;
    }
    compilePdbPath += "/";
    if (this->GeneratorTarget->GetType() == cmStateEnums::STATIC_LIBRARY) {
      // Match VS default for static libs: `$(IntDir)$(ProjectName).pdb`.
      compilePdbPath += this->GeneratorTarget->GetName();
      compilePdbPath += ".pdb";
    }
  }

  return compilePdbPath;
}

std::string cmCommonTargetGenerator::GetManifests(const std::string& config)
{
  std::vector<cmSourceFile const*> manifest_srcs;
  this->GeneratorTarget->GetManifests(manifest_srcs, config);

  std::vector<std::string> manifests;
  manifests.reserve(manifest_srcs.size());
  for (cmSourceFile const* manifest_src : manifest_srcs) {
    manifests.push_back(this->LocalCommonGenerator->ConvertToOutputFormat(
      this->LocalCommonGenerator->MaybeConvertToRelativePath(
        this->LocalCommonGenerator->GetWorkingDirectory(),
        manifest_src->GetFullPath()),
      cmOutputConverter::SHELL));
  }

  return cmJoin(manifests, " ");
}

std::string cmCommonTargetGenerator::GetAIXExports(std::string const&)
{
  std::string aixExports;
  if (this->GeneratorTarget->Target->IsAIX()) {
    if (const char* exportAll =
          this->GeneratorTarget->GetProperty("AIX_EXPORT_ALL_SYMBOLS")) {
      if (cmIsOff(exportAll)) {
        aixExports = "-n";
      }
    }
  }
  return aixExports;
}

void cmCommonTargetGenerator::AppendOSXVerFlag(std::string& flags,
                                               const std::string& lang,
                                               const char* name, bool so)
{
  // Lookup the flag to specify the version.
  std::string fvar = cmStrCat("CMAKE_", lang, "_OSX_", name, "_VERSION_FLAG");
  const char* flag = this->Makefile->GetDefinition(fvar);

  // Skip if no such flag.
  if (!flag) {
    return;
  }

  // Lookup the target version information.
  int major;
  int minor;
  int patch;
  std::string prop = cmStrCat("MACHO_", name, "_VERSION");
  std::string fallback_prop = so ? "SOVERSION" : "VERSION";
  this->GeneratorTarget->GetTargetVersionFallback(prop, fallback_prop, major,
                                                  minor, patch);
  if (major > 0 || minor > 0 || patch > 0) {
    // Append the flag since a non-zero version is specified.
    std::ostringstream vflag;
    vflag << flag << major << "." << minor << "." << patch;
    this->LocalCommonGenerator->AppendFlags(flags, vflag.str());
  }
}
