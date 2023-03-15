/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include "cmLocalVisualStudio7Generator.h"

class cmGeneratorTarget;
class cmGlobalGenerator;
class cmMakefile;

/** \class cmLocalVisualStudio10Generator
 * \brief Write Visual Studio 10 project files.
 *
 * cmLocalVisualStudio10Generator produces a MSBuild project file for each
 * target in its directory.
 */
class cmLocalVisualStudio10Generator : public cmLocalVisualStudio7Generator
{
public:
  //! Set cache only and recurse to false by default.
  cmLocalVisualStudio10Generator(cmGlobalGenerator* gg, cmMakefile* mf);

  ~cmLocalVisualStudio10Generator() override;

  void ReadAndStoreExternalGUID(const std::string& name,
                                const char* path) override;

protected:
  const char* ReportErrorLabel() const override;
  bool CustomCommandUseLocal() const override { return true; }

private:
  void GenerateTarget(cmGeneratorTarget* target) override;
};
