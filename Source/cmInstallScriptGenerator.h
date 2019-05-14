/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmInstallScriptGenerator_h
#define cmInstallScriptGenerator_h

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmInstallGenerator.h"
#include "cmScriptGenerator.h"

#include <iosfwd>
#include <string>

class cmLocalGenerator;

/** \class cmInstallScriptGenerator
 * \brief Generate target installation rules.
 */
class cmInstallScriptGenerator : public cmInstallGenerator
{
public:
  cmInstallScriptGenerator(const char* script, bool code,
                           const char* component, bool exclude_from_all);
  ~cmInstallScriptGenerator() override;

  bool Compute(cmLocalGenerator* lg) override;

protected:
  void GenerateScriptActions(std::ostream& os, Indent indent) override;
  void GenerateScriptForConfig(std::ostream& os, const std::string& config,
                               Indent indent) override;
  void AddScriptInstallRule(std::ostream& os, Indent indent,
                            std::string const& script);

  std::string Script;
  bool Code;
  cmLocalGenerator* LocalGenerator;
  bool AllowGenex;
};

#endif
