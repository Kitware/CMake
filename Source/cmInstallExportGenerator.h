/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmInstallExportGenerator_h
#define cmInstallExportGenerator_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>

#include "cmInstallGenerator.h"
#include "cmScriptGenerator.h"

class cmExportInstallFileGenerator;
class cmExportSet;
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
                           std::string name_space, bool exportOld,
                           bool android);
  ~cmInstallExportGenerator() override;

  cmExportSet* GetExportSet() { return this->ExportSet; }

  bool Compute(cmLocalGenerator* lg) override;

  cmLocalGenerator* GetLocalGenerator() const { return this->LocalGenerator; }

  const std::string& GetNamespace() const { return this->Namespace; }

  std::string const& GetDestination() const { return this->Destination; }
  std::string GetDestinationFile() const;

protected:
  void GenerateScript(std::ostream& os) override;
  void GenerateScriptConfigs(std::ostream& os, Indent indent) override;
  void GenerateScriptActions(std::ostream& os, Indent indent) override;
  void GenerateImportFile(cmExportSet const* exportSet);
  void GenerateImportFile(const char* config, cmExportSet const* exportSet);
  void ComputeTempDir();
  size_t GetMaxConfigLength() const;

  cmExportSet* const ExportSet;
  std::string const FilePermissions;
  std::string const FileName;
  std::string const Namespace;
  bool const ExportOld;
  cmLocalGenerator* LocalGenerator;

  std::string TempDir;
  std::string MainImportFile;
  cmExportInstallFileGenerator* EFGen;
};

#endif
