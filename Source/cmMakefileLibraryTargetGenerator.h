/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmMakefileTargetGenerator.h"

class cmGeneratorTarget;

class cmMakefileLibraryTargetGenerator : public cmMakefileTargetGenerator
{
public:
  cmMakefileLibraryTargetGenerator(cmGeneratorTarget* target);
  ~cmMakefileLibraryTargetGenerator() override;

  /* the main entry point for this class. Writes the Makefiles associated
     with this target */
  void WriteRuleFiles() override;

protected:
  void WriteObjectLibraryRules();
  void WriteStaticLibraryRules();
  void WriteSharedLibraryRules(bool relink);
  void WriteModuleLibraryRules(bool relink);

  void WriteDeviceLibraryRules(const std::string& linkRule, bool relink);
  void WriteNvidiaDeviceLibraryRules(const std::string& linkRuleVar,
                                     bool relink,
                                     std::vector<std::string>& commands,
                                     const std::string& targetOutput);
  void WriteLibraryRules(const std::string& linkRule,
                         const std::string& extraFlags, bool relink);
  // MacOSX Framework support methods
  void WriteFrameworkRules(bool relink);

  // Store the computd framework version for OS X Frameworks.
  std::string FrameworkVersion;

private:
  std::string DeviceLinkObject;
};
