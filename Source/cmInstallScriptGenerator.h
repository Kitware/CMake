/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>

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
    bool exclude_from_all, bool all_components,
    cmListFileBacktrace backtrace = cmListFileBacktrace());
  ~cmInstallScriptGenerator() override;

  bool Compute(cmLocalGenerator* lg) override;

  bool IsCode() const { return this->Code; }

  std::string GetScript(std::string const& config) const;

protected:
  void GenerateScriptActions(std::ostream& os, Indent indent) override;
  void GenerateScriptForConfig(std::ostream& os, const std::string& config,
                               Indent indent) override;
  void AddScriptInstallRule(std::ostream& os, Indent indent,
                            std::string const& script) const;

  std::string const Script;
  bool const Code;
  cmLocalGenerator* LocalGenerator;
  bool AllowGenex = false;
};
