/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>
#include <vector>

#include "cmInstallGenerator.h"
#include "cmListFileCache.h"

class cmLocalGenerator;

/** \class cmInstallScriptGenerator
 * \brief Generate target installation rules.
 */
class cmInstallScriptGenerator : public cmInstallGenerator
{
public:
  cmInstallScriptGenerator(
    std::string script, bool code, std::string const& component,
    bool excludeFromAll, bool allComponents,
    std::vector<std::string> buildDepends = {},
    cmListFileBacktrace backtrace = cmListFileBacktrace());
  ~cmInstallScriptGenerator() override;

  bool Compute(cmLocalGenerator* lg) override;

  void GetInstallBuildDependencies(
    std::string const& config,
    std::vector<BuildDependency>& dependencies) const override;

  bool IsCode() const { return this->Code; }

  std::string GetScript(std::string const& config) const;

protected:
  void GenerateScriptActions(std::ostream& os, Indent indent) override;
  void GenerateScriptForConfig(std::ostream& os, std::string const& config,
                               Indent indent) override;
  void AddScriptInstallRule(std::ostream& os, Indent indent,
                            std::string const& script) const;

  std::string const Script;
  bool const Code;
  std::vector<std::string> const BuildDepends;
  BuildDependsEntries ResolvedBuildDepends;
  cmLocalGenerator* LocalGenerator;
  bool AllowGenex = false;
};
