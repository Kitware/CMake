/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "cmInstallGenerator.h"

class cmExportInstallFileGenerator;
class cmExportSet;
class cmListFileBacktrace;
class cmLocalGenerator;

/** \class cmInstallExportGenerator
 * \brief Support class for generating rules for creating export files.
 */
class cmInstallExportGenerator : public cmInstallGenerator
{
public:
  cmInstallExportGenerator(cmExportSet* exportSet, std::string destination,
                           std::string filePermissions,
                           std::vector<std::string> const& configurations,
                           std::string component, MessageLevel message,
                           bool excludeFromAll, std::string filename,
                           std::string targetNamespace,
                           std::string cxxModulesDirectory,
                           cmListFileBacktrace backtrace);
  cmInstallExportGenerator(const cmInstallExportGenerator&) = delete;
  ~cmInstallExportGenerator() override;

  cmInstallExportGenerator& operator=(const cmInstallExportGenerator&) =
    delete;

  virtual char const* InstallSubcommand() const = 0;

  cmExportSet* GetExportSet() { return this->ExportSet; }

  bool Compute(cmLocalGenerator* lg) override;

  cmLocalGenerator* GetLocalGenerator() const { return this->LocalGenerator; }

  const std::string& GetNamespace() const { return this->Namespace; }

  std::string const& GetMainImportFile() const { return this->MainImportFile; }

  std::string const& GetDestination() const { return this->Destination; }
  std::string GetDestinationFile() const;
  std::string GetFileName() const { return this->FileName; }
  std::string GetTempDir() const;
  std::string GetCxxModuleDirectory() const
  {
    return this->CxxModulesDirectory;
  }

protected:
  void GenerateScript(std::ostream& os) override;
  void GenerateScriptConfigs(std::ostream& os, Indent indent) override;
  void GenerateScriptActions(std::ostream& os, Indent indent) override;
  std::string TempDirCalculate() const;
  void ComputeTempDir();

  cmExportSet* const ExportSet;
  std::string const FilePermissions;
  std::string const FileName;
  std::string const Namespace;
  std::string const CxxModulesDirectory;
  cmLocalGenerator* LocalGenerator = nullptr;

  std::string TempDir;
  std::string MainImportFile;
  std::unique_ptr<cmExportInstallFileGenerator> EFGen;
};
