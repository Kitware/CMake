/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmInstallSbomGenerator.h"

#include <utility>
#include <vector>

#include <cm/memory>

#include "cmCryptoHash.h"
#include "cmDiagnosticContext.h"
#include "cmGeneratedFileStream.h"
#include "cmInstallSbomBuilder.h"
#include "cmInstallType.h"
#include "cmLocalGenerator.h"
#include "cmSbomArguments.h"
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
  , SbomFileName(args.GetPackageFileName())
  , SbomFilePath(cmStrCat(this->Destination, '/', this->SbomFileName))
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
             "/CMakeFiles/Sbom/", hasher.HashString(this->Destination));

  cmSystemTools::MakeDirectory(tempDir);

  this->TempSbomFilePath = cmStrCat(tempDir, '/', this->SbomFileName);

  // Generate the SBOM file now, at cmake generate time.
  cmGeneratedFileStream sbomStream(this->TempSbomFilePath);
  this->Builder->Generate(sbomStream);

  // Emit the cmake_install.cmake script to copy the file at install time.
  this->cmInstallGenerator::GenerateScript(os);
}

void cmInstallSbomGenerator::GenerateScriptActions(std::ostream& os,
                                                   Indent indent)
{
  std::vector<std::string> files{ this->TempSbomFilePath };
  this->AddInstallRule(os, this->Destination, cmInstallType_FILES, files,
                       false, this->FilePermissions.c_str(), nullptr, nullptr,
                       nullptr, indent);
}
