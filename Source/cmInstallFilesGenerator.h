/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmInstallFilesGenerator_h
#define cmInstallFilesGenerator_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>
#include <vector>

#include "cmInstallGenerator.h"
#include "cmScriptGenerator.h"

class cmLocalGenerator;

/** \class cmInstallFilesGenerator
 * \brief Generate file installation rules.
 */
class cmInstallFilesGenerator : public cmInstallGenerator
{
public:
  cmInstallFilesGenerator(std::vector<std::string> const& files,
                          std::string const& dest, bool programs,
                          std::string file_permissions,
                          std::vector<std::string> const& configurations,
                          std::string const& component, MessageLevel message,
                          bool exclude_from_all, std::string rename,
                          bool optional = false);
  ~cmInstallFilesGenerator() override;

  bool Compute(cmLocalGenerator* lg) override;

  std::string GetDestination(std::string const& config) const;

protected:
  void GenerateScriptActions(std::ostream& os, Indent indent) override;
  void GenerateScriptForConfig(std::ostream& os, const std::string& config,
                               Indent indent) override;
  void AddFilesInstallRule(std::ostream& os, std::string const& config,
                           Indent indent,
                           std::vector<std::string> const& files);

  cmLocalGenerator* LocalGenerator;
  std::vector<std::string> const Files;
  std::string const FilePermissions;
  std::string const Rename;
  bool const Programs;
  bool const Optional;
};

#endif
