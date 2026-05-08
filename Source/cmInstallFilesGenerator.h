/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>
#include <vector>

#include "cmInstallGenerator.h"

class cmDiagnosticContext;
class cmLocalGenerator;

/** \class cmInstallFilesGenerator
 * \brief Generate file installation rules.
 */
class cmInstallFilesGenerator : public cmInstallGenerator
{
public:
  cmInstallFilesGenerator(std::vector<std::string> const& files,
                          std::string const& dest, bool programs,
                          std::string filePermissions,
                          std::vector<std::string> const& configurations,
                          std::string const& component, MessageLevel message,
                          bool excludeFromAll, std::string rename,
                          bool optional, cmDiagnosticContext context);
  ~cmInstallFilesGenerator() override;

  bool Compute(cmLocalGenerator* lg) override;

  std::string GetDestination(std::string const& config) const;
  std::string GetRename(std::string const& config) const;
  std::vector<std::string> GetFiles(std::string const& config) const;
  bool GetOptional() const { return this->Optional; }

protected:
  void GenerateScriptActions(std::ostream& os, Indent indent) override;
  void GenerateScriptForConfig(std::ostream& os, std::string const& config,
                               Indent indent) override;
  void AddFilesInstallRule(std::ostream& os, std::string const& config,
                           Indent indent,
                           std::vector<std::string> const& files);

  cmLocalGenerator* LocalGenerator = nullptr;
  std::vector<std::string> const Files;
  std::string const FilePermissions;
  std::string const Rename;
  bool const Programs;
  bool const Optional;
};
