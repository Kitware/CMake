/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <memory>
#include <string>

#include "cmGlobalVisualStudioGenerator.h"
#include "cmLocalGenerator.h"
#include "cmVsProjectType.h"

class cmCustomCommand;
class cmCustomCommandGenerator;
class cmGeneratorTarget;
class cmGlobalGenerator;
class cmMakefile;
class cmSourceFile;

/** \class cmLocalVisualStudioGenerator
 * \brief Base class for Visual Studio generators.
 *
 * cmLocalVisualStudioGenerator provides functionality common to all
 * Visual Studio generators.
 */
class cmLocalVisualStudioGenerator : public cmLocalGenerator
{
public:
  cmLocalVisualStudioGenerator(cmGlobalGenerator* gg, cmMakefile* mf);
  ~cmLocalVisualStudioGenerator() override;

  virtual bool IsVFProj() const = 0;

  std::string ConstructScript(cmCustomCommandGenerator const& ccg,
                              std::string const& newline = "\n");
  std::string FinishConstructScript(VsProjectType projectType,
                                    std::string const& newline = "\n");

  /** Label to which to jump in a batch file after a failed step in a
      sequence of custom commands. */
  char const* GetReportErrorLabel() const;

  cmGlobalVisualStudioGenerator::VSVersion GetVersion() const;

  virtual std::string ComputeLongestObjectDirectory(
    cmGeneratorTarget const*) const = 0;

  void ComputeObjectFilenames(
    std::map<cmSourceFile const*, std::string>& mapping,
    cmGeneratorTarget const* = nullptr) override;

protected:
  virtual char const* ReportErrorLabel() const;
  virtual bool CustomCommandUseLocal() const { return false; }

  /** Construct a custom command to make exe import lib dir.  */
  std::unique_ptr<cmCustomCommand> MaybeCreateImplibDir(
    cmGeneratorTarget* target, std::string const& config, bool isFortran);
};
