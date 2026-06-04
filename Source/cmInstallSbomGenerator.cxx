/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmInstallSbomGenerator.h"

#include <ostream>
#include <utility>
#include <vector>

#include <cm/memory>

#include "cmCryptoHash.h"
#include "cmDiagnosticContext.h"
#include "cmGeneratedFileStream.h"
#include "cmInstallSbomBuilder.h"
#include "cmInstallType.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmSbomArguments.h"
#include "cmScriptGenerator.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cmInstallSbomGenerator::cmInstallSbomGenerator(
  std::vector<cmExportSet*> exportSets, std::string destination,
  std::string filePermissions, std::vector<std::string> const& configurations,
  std::string component, MessageLevel message, bool excludeFromAll,
  cmSbomArguments args, cmDiagnosticContext context)
  : cmInstallGenerator(std::move(destination), configurations,
                       std::move(component), message, excludeFromAll, false,
                       std::move(context))
  , FilePermissions(std::move(filePermissions))
  , SbomFileName(args.GetPackageName())
  , SbomFilePath(cmStrCat(this->Destination, '/', this->SbomFileName))
  , SbomFormat(args.GetFormat())
  , Builder(cm::make_unique<cmInstallSbomBuilder>(std::move(args),
                                                  std::move(exportSets)))
{
}

cmInstallSbomGenerator::~cmInstallSbomGenerator() = default;

bool cmInstallSbomGenerator::Compute(cmLocalGenerator* lg)
{
  this->LocalGenerator = lg;
  this->Builder->Compute(lg);
  return true;
}

bool cmInstallSbomGenerator::CoversTarget(
  cmGeneratorTarget const* target) const
{
  return this->Builder->CoversTarget(target);
}

std::string const& cmInstallSbomGenerator::GetPackageName() const
{
  return this->Builder->GetPackageName();
}

bool cmInstallSbomGenerator::CoversExportSet(cmExportSet const* set) const
{
  return this->Builder->CoversExportSet(set);
}

void cmInstallSbomGenerator::GenerateScript(std::ostream& os)
{
  // Choose a temporary directory in the build tree to hold the generated SBOM.
  cmCryptoHash hasher(cmCryptoHash::AlgoMD5);
  std::string const tempDir =
    cmStrCat(this->LocalGenerator->GetCurrentBinaryDirectory(),
             "/CMakeFiles/sbom/", hasher.HashString(this->Destination));
  cmSystemTools::MakeDirectory(tempDir);

  for (std::string const& c :
       this->LocalGenerator->GetMakefile()->GetGeneratorConfigs(
         cmMakefile::IncludeEmptyConfig)) {
    std::string configName =
      cmStrCat(tempDir, '/', this->SbomFileName, "-", c, ".spdx.json");
    cmGeneratedFileStream sbomStream(configName);
    this->TempSbomFiles.emplace(c, configName);
    if (!this->Builder->Generate(sbomStream, c)) {
      break;
    }
  }

  // Emit the cmake_install.cmake script to copy the file at install time.
  this->cmInstallGenerator::GenerateScript(os);
}

void cmInstallSbomGenerator::GenerateScriptActions(std::ostream& os,
                                                   Indent indent)
{
  for (auto const& i : this->TempSbomFiles) {
    std::string configTest = this->CreateConfigTest(i.first);
    os << indent << "if(" << configTest << ")\n";
    this->AddInstallRule(os, this->Destination, cmInstallType_FILES,
                         { i.second }, false, this->FilePermissions.c_str(),
                         nullptr, nullptr, nullptr, indent.Next());
    os << indent << "endif()\n";
  }
}
