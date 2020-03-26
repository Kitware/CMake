/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmInstallScriptGenerator_h
#define cmInstallScriptGenerator_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>

#include "cmInstallGenerator.h"
#include "cmScriptGenerator.h"

class cmLocalGenerator;

/** \class cmInstallScriptGenerator
 * \brief Generate target installation rules.
 */
class cmInstallScriptGenerator : public cmInstallGenerator
{
public:
  cmInstallScriptGenerator(std::string script, bool code,
                           std::string const& component,
                           bool exclude_from_all);
  ~cmInstallScriptGenerator() override;

  bool Compute(cmLocalGenerator* lg) override;

protected:
  void GenerateScriptActions(std::ostream& os, Indent indent) override;
  void GenerateScriptForConfig(std::ostream& os, const std::string& config,
                               Indent indent) override;
  void AddScriptInstallRule(std::ostream& os, Indent indent,
                            std::string const& script);

  std::string const Script;
  bool const Code;
  cmLocalGenerator* LocalGenerator;
  bool AllowGenex;
};

#endif
