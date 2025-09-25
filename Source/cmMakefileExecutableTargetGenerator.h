/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmMakefileTargetGenerator.h"

class cmGeneratorTarget;

class cmMakefileExecutableTargetGenerator : public cmMakefileTargetGenerator
{
public:
  cmMakefileExecutableTargetGenerator(cmGeneratorTarget* target);
  ~cmMakefileExecutableTargetGenerator() override;

  /* the main entry point for this class. Writes the Makefiles associated
     with this target */
  void WriteRuleFiles() override;

protected:
  virtual void WriteExecutableRule(bool relink);
  virtual void WriteDeviceExecutableRule(bool relink);
  virtual void WriteNvidiaDeviceExecutableRule(
    bool relink, std::vector<std::string>& commands,
    std::string const& targetOutput);

private:
  std::string DeviceLinkObject;
};
