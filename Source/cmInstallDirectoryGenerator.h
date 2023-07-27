/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>
#include <vector>

#include "cmInstallGenerator.h"

class cmListFileBacktrace;
class cmLocalGenerator;

/** \class cmInstallDirectoryGenerator
 * \brief Generate directory installation rules.
 */
class cmInstallDirectoryGenerator : public cmInstallGenerator
{
public:
  cmInstallDirectoryGenerator(
    std::vector<std::string> const& dirs, std::string const& dest,
    std::string file_permissions, std::string dir_permissions,
    std::vector<std::string> const& configurations,
    std::string const& component, MessageLevel message, bool exclude_from_all,
    std::string literal_args, bool optional, cmListFileBacktrace backtrace);
  ~cmInstallDirectoryGenerator() override;

  bool Compute(cmLocalGenerator* lg) override;

  std::string GetDestination(std::string const& config) const;
  std::vector<std::string> GetDirectories(std::string const& config) const;

  bool GetOptional() const { return this->Optional; }

protected:
  void GenerateScriptActions(std::ostream& os, Indent indent) override;
  void GenerateScriptForConfig(std::ostream& os, const std::string& config,
                               Indent indent) override;
  void AddDirectoryInstallRule(std::ostream& os, const std::string& config,
                               Indent indent,
                               std::vector<std::string> const& dirs);
  cmLocalGenerator* LocalGenerator = nullptr;
  std::vector<std::string> const Directories;
  std::string const FilePermissions;
  std::string const DirPermissions;
  std::string const LiteralArguments;
  bool const Optional;
};
