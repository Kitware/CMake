/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>
#include <vector>

#include "cmInstallGenerator.h"
#include "cmScriptGenerator.h"

class cmGeneratorTarget;
class cmListFileBacktrace;
class cmLocalGenerator;

/** \class cmInstallCxxModuleBmiGenerator
 * \brief Generate C++ module BMI installation rules.
 */
class cmInstallCxxModuleBmiGenerator : public cmInstallGenerator
{
public:
  cmInstallCxxModuleBmiGenerator(
    std::string target, std::string const& dest, std::string file_permissions,
    std::vector<std::string> const& configurations,
    std::string const& component, MessageLevel message, bool exclude_from_all,
    bool optional, cmListFileBacktrace backtrace);
  ~cmInstallCxxModuleBmiGenerator() override;

  bool Compute(cmLocalGenerator* lg) override;

  std::string const& GetFilePermissions() const
  {
    return this->FilePermissions;
  }
  std::string GetDestination(std::string const& config) const;
  std::string GetScriptLocation(std::string const& config) const;
  cmGeneratorTarget const* GetTarget() const { return this->Target; }
  bool GetOptional() const { return this->Optional; }
  MessageLevel GetMessageLevel() const { return this->Message; }

protected:
  void GenerateScriptForConfig(std::ostream& os, const std::string& config,
                               Indent indent) override;

  std::string const TargetName;
  cmGeneratorTarget const* Target = nullptr;
  cmLocalGenerator* LocalGenerator = nullptr;
  std::string const FilePermissions;
  bool const Optional;
};
