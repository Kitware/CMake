/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmLocalFastbuildGenerator.h"

#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "cmFastbuildTargetGenerator.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalFastbuildGenerator.h"
#include "cmList.h"
#include "cmLocalCommonGenerator.h"
#include "cmMakefile.h"
#include "cmObjectLocation.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmake.h"

class cmGlobalGenerator;

cmLocalFastbuildGenerator::cmLocalFastbuildGenerator(cmGlobalGenerator* gg,
                                                     cmMakefile* makefile)
  : cmLocalCommonGenerator(gg, makefile)
{
}

void cmLocalFastbuildGenerator::Generate()
{
  auto const& targets = this->GetGeneratorTargets();
  for (auto const& target : targets) {
    if (!target->IsInBuildSystem()) {
      continue;
    }
    for (std::string config : this->GetConfigNames()) {
      cmFastbuildTargetGenerator* tg =
        cmFastbuildTargetGenerator::New(target.get(), std::move(config));
      if (tg) {
        tg->Generate();
        delete tg;
      }
      // Directory level.
      this->AdditionalCleanFiles(config);
    }
  }
}

cmGlobalFastbuildGenerator const*
cmLocalFastbuildGenerator::GetGlobalFastbuildGenerator() const
{
  return static_cast<cmGlobalFastbuildGenerator const*>(
    this->GetGlobalGenerator());
}

cmGlobalFastbuildGenerator*
cmLocalFastbuildGenerator::GetGlobalFastbuildGenerator()
{
  return static_cast<cmGlobalFastbuildGenerator*>(this->GetGlobalGenerator());
}

void cmLocalFastbuildGenerator::ComputeObjectFilenames(
  std::map<cmSourceFile const*, cmObjectLocations>& mapping,
  std::string const& config, cmGeneratorTarget const* gt)
{
  char const* customExt = gt->GetCustomObjectExtension();
  for (auto& si : mapping) {
    cmSourceFile const* sf = si.first;
    si.second.LongLoc = this->GetObjectFileNameWithoutTarget(
      *sf, gt->ObjectDirectory, nullptr, nullptr);
    this->FillCustomInstallObjectLocations(*sf, config, nullptr,
                                           si.second.InstallLongLoc);
    // FASTBuild always appends output extension to the source file name.
    // So if custom ext is ".ptx", then
    // "kernelA.cu" will be outputted as "kernelA.cu.ptx",
    // that's why we can't just replace ".cu" with ".ptx".
    // This is needed to resolve $<TARGET_OBJECTS> genex correctly.
    // Tested in "CudaOnly.ExportPTX" test.
    if (customExt) {
      si.second.LongLoc.Update(
        cmStrCat(si.second.LongLoc.GetPath(), customExt));
    }
  }
}

void cmLocalFastbuildGenerator::AppendFlagEscape(
  std::string& flags, std::string const& rawFlag) const
{
  std::string escapedFlag = this->EscapeForShell(rawFlag);
  // Other make systems will remove the double $ but
  // fastbuild uses ^$ to escape it. So switch to that.
  // cmSystemTools::ReplaceString(escapedFlag, "$$", "^$");
  this->AppendFlags(flags, escapedFlag);
}

void cmLocalFastbuildGenerator::AdditionalCleanFiles(std::string const& config)
{
  if (cmValue prop_value =
        this->Makefile->GetProperty("ADDITIONAL_CLEAN_FILES")) {
    cmList cleanFiles{ cmGeneratorExpression::Evaluate(*prop_value, this,
                                                       config) };
    std::string const& binaryDir = this->GetCurrentBinaryDirectory();
    auto* gg = this->GetGlobalFastbuildGenerator();
    for (auto const& cleanFile : cleanFiles) {
      // Support relative paths
      gg->AddFileToClean(gg->ConvertToFastbuildPath(
        cmSystemTools::CollapseFullPath(cleanFile, binaryDir)));
    }
  }
}

std::string cmLocalFastbuildGenerator::ConvertToIncludeReference(
  std::string const& path, cmOutputConverter::OutputFormat format)
{
  std::string converted = this->ConvertToOutputForExisting(path, format);
  cmGlobalFastbuildGenerator const* GG = this->GetGlobalFastbuildGenerator();
  if (GG->UsingRelativePaths && cmSystemTools::FileIsFullPath(path)) {
    std::string const binDir = this->ConvertToOutputFormat(
      GG->GetCMakeInstance()->GetHomeOutputDirectory(), OutputFormat::SHELL);
    if (binDir == converted) {
      return ".";
    }
    return cmSystemTools::RelativePath(binDir, converted);
  }
  return converted;
}
