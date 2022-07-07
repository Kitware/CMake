/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "cmInstallGenerator.h"
#include "cmScriptGenerator.h"

class cmExportInstallFileGenerator;
class cmExportSet;
class cmListFileBacktrace;
class cmLocalGenerator;

/** \class cmInstallExportGenerator
 * \brief Generate rules for creating an export files.
 */
class cmInstallExportGenerator : public cmInstallGenerator
{
public:
  cmInstallExportGenerator(cmExportSet* exportSet, std::string const& dest,
                           std::string file_permissions,
                           const std::vector<std::string>& configurations,
                           std::string const& component, MessageLevel message,
                           bool exclude_from_all, std::string filename,
                           std::string name_space,
                           std::string cxx_modules_directory, bool exportOld,
                           bool android, cmListFileBacktrace backtrace);
  cmInstallExportGenerator(const cmInstallExportGenerator&) = delete;
  ~cmInstallExportGenerator() override;

  cmInstallExportGenerator& operator=(const cmInstallExportGenerator&) =
    delete;

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
  void GenerateImportFile(cmExportSet const* exportSet);
  void GenerateImportFile(const char* config, cmExportSet const* exportSet);
  std::string TempDirCalculate() const;
  void ComputeTempDir();

  cmExportSet* const ExportSet;
  std::string const FilePermissions;
  std::string const FileName;
  std::string const Namespace;
  std::string const CxxModulesDirectory;
  bool const ExportOld;
  cmLocalGenerator* LocalGenerator = nullptr;

  std::string TempDir;
  std::string MainImportFile;
  std::unique_ptr<cmExportInstallFileGenerator> EFGen;
};
