/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmMakefileUtilityTargetGenerator.h"

#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include <cm/memory>

#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmOSXBundleGenerator.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cmMakefileUtilityTargetGenerator::cmMakefileUtilityTargetGenerator(
  cmGeneratorTarget* target)
  : cmMakefileTargetGenerator(target)
{
  this->CustomCommandDriver = OnUtility;
  this->OSXBundleGenerator = cm::make_unique<cmOSXBundleGenerator>(target);
  this->OSXBundleGenerator->SetMacContentFolders(&this->MacContentFolders);
}

cmMakefileUtilityTargetGenerator::~cmMakefileUtilityTargetGenerator() =
  default;

void cmMakefileUtilityTargetGenerator::WriteRuleFiles()
{
  this->CreateRuleFile();

  *this->BuildFileStream << "# Utility rule file for "
                         << this->GeneratorTarget->GetName() << ".\n\n";

  const char* root = (this->Makefile->IsOn("CMAKE_MAKE_INCLUDE_FROM_ROOT")
                        ? "$(CMAKE_BINARY_DIR)/"
                        : "");

  // Include the dependencies for the target.
  std::string dependFile =
    cmStrCat(this->TargetBuildDirectoryFull, "/compiler_depend.make");
  *this->BuildFileStream
    << "# Include any custom commands dependencies for this target.\n"
    << this->GlobalGenerator->IncludeDirective << " " << root
    << cmSystemTools::ConvertToOutputPath(
         this->LocalGenerator->MaybeRelativeToTopBinDir(dependFile))
    << "\n\n";
  if (!cmSystemTools::FileExists(dependFile)) {
    // Write an empty dependency file.
    cmGeneratedFileStream depFileStream(
      dependFile, false, this->GlobalGenerator->GetMakefileEncoding());
    depFileStream << "# Empty custom commands generated dependencies file for "
                  << this->GeneratorTarget->GetName() << ".\n"
                  << "# This may be replaced when dependencies are built.\n";
  }

  std::string dependTimestamp =
    cmStrCat(this->TargetBuildDirectoryFull, "/compiler_depend.ts");
  if (!cmSystemTools::FileExists(dependTimestamp)) {
    // Write a dependency timestamp file.
    cmGeneratedFileStream depFileStream(
      dependTimestamp, false, this->GlobalGenerator->GetMakefileEncoding());
    depFileStream << "# CMAKE generated file: DO NOT EDIT!\n"
                  << "# Timestamp file for custom commands dependencies "
                     "management for "
                  << this->GeneratorTarget->GetName() << ".\n";
  }

  if (!this->NoRuleMessages) {
    // Include the progress variables for the target.
    *this->BuildFileStream
      << "# Include the progress variables for this target.\n"
      << this->GlobalGenerator->IncludeDirective << " " << root
      << cmSystemTools::ConvertToOutputPath(
           this->LocalGenerator->MaybeRelativeToTopBinDir(
             this->ProgressFileNameFull))
      << "\n\n";
  }

  // write the custom commands for this target
  this->WriteTargetBuildRules();

  // Collect the commands and dependencies.
  std::vector<std::string> commands;
  std::vector<std::string> depends;

  // Utility targets store their rules in pre- and post-build commands.
  this->LocalGenerator->AppendCustomDepends(
    depends, this->GeneratorTarget->GetPreBuildCommands());

  this->LocalGenerator->AppendCustomDepends(
    depends, this->GeneratorTarget->GetPostBuildCommands());

  this->LocalGenerator->AppendCustomCommands(
    commands, this->GeneratorTarget->GetPreBuildCommands(),
    this->GeneratorTarget, this->LocalGenerator->GetBinaryDirectory());

  // Depend on all custom command outputs for sources
  this->DriveCustomCommands(depends);

  this->LocalGenerator->AppendCustomCommands(
    commands, this->GeneratorTarget->GetPostBuildCommands(),
    this->GeneratorTarget, this->LocalGenerator->GetBinaryDirectory());

  // Add dependencies on targets that must be built first.
  this->AppendTargetDepends(depends);

  // Add a dependency on the rule file itself.
  this->LocalGenerator->AppendRuleDepend(depends,
                                         this->BuildFileNameFull.c_str());

  // If the rule is empty add the special empty rule dependency needed
  // by some make tools.
  if (depends.empty() && commands.empty()) {
    std::string hack = this->GlobalGenerator->GetEmptyRuleHackDepends();
    if (!hack.empty()) {
      depends.push_back(std::move(hack));
    }
  }

  // Write the rule.
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, nullptr,
                                      this->GeneratorTarget->GetName(),
                                      depends, commands, true);

  // Write the main driver rule to build everything in this target.
  this->WriteTargetDriverRule(this->GeneratorTarget->GetName(), false);

  // Write clean target
  this->WriteTargetCleanRules();

  // Write the dependency generation rule.  This must be done last so
  // that multiple output pair information is available.
  this->WriteTargetDependRules();

  // close the streams
  this->CloseFileStreams();
}
