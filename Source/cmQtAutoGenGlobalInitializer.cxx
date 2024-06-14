/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoGenGlobalInitializer.h"

#include <set>
#include <utility>

#include <cm/memory>

#include "cmCustomCommand.h"
#include "cmDuration.h"
#include "cmGeneratorTarget.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmProcessOutput.h"
#include "cmQtAutoGen.h"
#include "cmQtAutoGenInitializer.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmValue.h"

cmQtAutoGenGlobalInitializer::Keywords::Keywords()
  : AUTOMOC("AUTOMOC")
  , AUTOUIC("AUTOUIC")
  , AUTORCC("AUTORCC")
  , AUTOMOC_EXECUTABLE("AUTOMOC_EXECUTABLE")
  , AUTOUIC_EXECUTABLE("AUTOUIC_EXECUTABLE")
  , AUTORCC_EXECUTABLE("AUTORCC_EXECUTABLE")
  , SKIP_AUTOGEN("SKIP_AUTOGEN")
  , SKIP_AUTOMOC("SKIP_AUTOMOC")
  , SKIP_AUTOUIC("SKIP_AUTOUIC")
  , SKIP_AUTORCC("SKIP_AUTORCC")
  , AUTOUIC_OPTIONS("AUTOUIC_OPTIONS")
  , AUTORCC_OPTIONS("AUTORCC_OPTIONS")
  , qrc("qrc")
  , ui("ui")
{
}

cmQtAutoGenGlobalInitializer::cmQtAutoGenGlobalInitializer(
  std::vector<std::unique_ptr<cmLocalGenerator>> const& localGenerators)
{
  for (const auto& localGen : localGenerators) {
    // Detect global autogen and autorcc target names
    bool globalAutoGenTarget = false;
    bool globalAutoRccTarget = false;
    {
      cmMakefile const* makefile = localGen->GetMakefile();
      // Detect global autogen target name
      if (makefile->IsOn("CMAKE_GLOBAL_AUTOGEN_TARGET")) {
        std::string targetName =
          makefile->GetSafeDefinition("CMAKE_GLOBAL_AUTOGEN_TARGET_NAME");
        if (targetName.empty()) {
          targetName = "autogen";
        }
        this->GlobalAutoGenTargets_.emplace(localGen.get(),
                                            std::move(targetName));
        globalAutoGenTarget = true;
      }

      // Detect global autorcc target name
      if (makefile->IsOn("CMAKE_GLOBAL_AUTORCC_TARGET")) {
        std::string targetName =
          makefile->GetSafeDefinition("CMAKE_GLOBAL_AUTORCC_TARGET_NAME");
        if (targetName.empty()) {
          targetName = "autorcc";
        }
        this->GlobalAutoRccTargets_.emplace(localGen.get(),
                                            std::move(targetName));
        globalAutoRccTarget = true;
      }
    }

    // Find targets that require AUTOMOC/UIC/RCC processing
    for (const auto& target : localGen->GetGeneratorTargets()) {
      // Process only certain target types
      switch (target->GetType()) {
        case cmStateEnums::EXECUTABLE:
        case cmStateEnums::STATIC_LIBRARY:
        case cmStateEnums::SHARED_LIBRARY:
        case cmStateEnums::MODULE_LIBRARY:
        case cmStateEnums::OBJECT_LIBRARY:
          // Process target
          break;
        default:
          // Don't process target
          continue;
      }
      if (target->IsImported()) {
        // Don't process target
        continue;
      }
      std::set<std::string> const& languages =
        target->GetAllConfigCompileLanguages();
      // cmGeneratorTarget::GetAllConfigCompileLanguages caches the target's
      // sources. Clear it so that OBJECT library targets that are AUTOGEN
      // initialized after this target get their added mocs_compilation.cpp
      // source acknowledged by this target.
      target->ClearSourcesCache();
      if (languages.count("CSharp")) {
        // Don't process target if it's a CSharp target
        continue;
      }

      bool const moc = target->GetPropertyAsBool(this->kw().AUTOMOC);
      bool const uic = target->GetPropertyAsBool(this->kw().AUTOUIC);
      bool const rcc = target->GetPropertyAsBool(this->kw().AUTORCC);
      if (moc || uic || rcc) {
        std::string const& mocExec =
          target->GetSafeProperty(this->kw().AUTOMOC_EXECUTABLE);
        std::string const& uicExec =
          target->GetSafeProperty(this->kw().AUTOUIC_EXECUTABLE);
        std::string const& rccExec =
          target->GetSafeProperty(this->kw().AUTORCC_EXECUTABLE);

        // We support Qt4, Qt5 and Qt6
        auto const qtVersion =
          cmQtAutoGenInitializer::GetQtVersion(target.get(), mocExec);
        bool const validQt = (qtVersion.first.Major == 4) ||
          (qtVersion.first.Major == 5) || (qtVersion.first.Major == 6);

        bool const mocAvailable = (validQt || !mocExec.empty());
        bool const uicAvailable = (validQt || !uicExec.empty());
        bool const rccAvailable = (validQt || !rccExec.empty());
        bool const mocIsValid = (moc && mocAvailable);
        bool const uicIsValid = (uic && uicAvailable);
        bool const rccIsValid = (rcc && rccAvailable);
        // Disabled AUTOMOC/UIC/RCC warning
        bool const mocDisabled = (moc && !mocAvailable);
        bool const uicDisabled = (uic && !uicAvailable);
        bool const rccDisabled = (rcc && !rccAvailable);
        if (mocDisabled || uicDisabled || rccDisabled) {
          cmAlphaNum version = (qtVersion.second == 0)
            ? cmAlphaNum("<QTVERSION>")
            : cmAlphaNum(qtVersion.second);
          cmAlphaNum component = uicDisabled ? "Widgets" : "Core";

          std::string const msg = cmStrCat(
            "AUTOGEN: No valid Qt version found for target ",
            target->GetName(), ".  ",
            cmQtAutoGen::Tools(mocDisabled, uicDisabled, rccDisabled),
            " disabled.  Consider adding:\n", "  find_package(Qt", version,
            " COMPONENTS ", component, ")\n", "to your CMakeLists.txt file.");
          target->Makefile->IssueMessage(MessageType::AUTHOR_WARNING, msg);
        }
        if (mocIsValid || uicIsValid || rccIsValid) {
          // Create autogen target initializer
          this->Initializers_.emplace_back(
            cm::make_unique<cmQtAutoGenInitializer>(
              this, target.get(), qtVersion.first, mocIsValid, uicIsValid,
              rccIsValid, globalAutoGenTarget, globalAutoRccTarget));
        }
      }
    }
  }
}

cmQtAutoGenGlobalInitializer::~cmQtAutoGenGlobalInitializer() = default;

void cmQtAutoGenGlobalInitializer::GetOrCreateGlobalTarget(
  cmLocalGenerator* localGen, std::string const& name,
  std::string const& comment)
{
  // Test if the target already exists
  if (localGen->FindGeneratorTargetToUse(name) == nullptr) {
    cmMakefile const* makefile = localGen->GetMakefile();

    // Create utility target
    auto cc = cm::make_unique<cmCustomCommand>();
    cc->SetWorkingDirectory(makefile->GetHomeOutputDirectory().c_str());
    cc->SetEscapeOldStyle(false);
    cc->SetComment(comment.c_str());
    cmTarget* target = localGen->AddUtilityCommand(name, true, std::move(cc));
    localGen->AddGeneratorTarget(
      cm::make_unique<cmGeneratorTarget>(target, localGen));

    // Set FOLDER property in the target
    {
      cmValue folder =
        makefile->GetState()->GetGlobalProperty("AUTOGEN_TARGETS_FOLDER");
      if (folder) {
        target->SetProperty("FOLDER", folder);
      }
    }
  }
}

void cmQtAutoGenGlobalInitializer::AddToGlobalAutoGen(
  cmLocalGenerator* localGen, std::string const& targetName)
{
  auto const it = this->GlobalAutoGenTargets_.find(localGen);
  if (it != this->GlobalAutoGenTargets_.end()) {
    cmGeneratorTarget const* target =
      localGen->FindGeneratorTargetToUse(it->second);
    if (target != nullptr) {
      target->Target->AddUtility(targetName, false, localGen->GetMakefile());
    }
  }
}

void cmQtAutoGenGlobalInitializer::AddToGlobalAutoRcc(
  cmLocalGenerator* localGen, std::string const& targetName)
{
  auto const it = this->GlobalAutoRccTargets_.find(localGen);
  if (it != this->GlobalAutoRccTargets_.end()) {
    cmGeneratorTarget const* target =
      localGen->FindGeneratorTargetToUse(it->second);
    if (target != nullptr) {
      target->Target->AddUtility(targetName, false, localGen->GetMakefile());
    }
  }
}

cmQtAutoGen::ConfigStrings<cmQtAutoGen::CompilerFeaturesHandle>
cmQtAutoGenGlobalInitializer::GetCompilerFeatures(
  std::string const& generator, cmQtAutoGen::ConfigString const& executable,
  std::string& error, bool const isMultiConfig, bool const UseBetterGraph)
{
  cmQtAutoGen::ConfigStrings<cmQtAutoGen::CompilerFeaturesHandle> res;
  if (isMultiConfig && UseBetterGraph) {
    for (auto const& config : executable.Config) {
      auto const exe = config.second;
      // Check if we have cached features
      {
        auto it = this->CompilerFeatures_.Config[config.first].find(exe);
        if (it != this->CompilerFeatures_.Config[config.first].end()) {
          res.Config[config.first] = it->second;
          continue;
        }
      }

      // Check if the executable exists
      if (!cmSystemTools::FileExists(exe, true)) {
        error = cmStrCat("The \"", generator, "\" executable ",
                         cmQtAutoGen::Quoted(exe), " does not exist.");
        res.Config[config.first] = {};
        continue;
      }

      // Test the executable
      std::string stdOut;
      {
        std::string stdErr;
        std::vector<std::string> command;
        command.emplace_back(exe);
        command.emplace_back("-h");
        int retVal = 0;
        const bool runResult = cmSystemTools::RunSingleCommand(
          command, &stdOut, &stdErr, &retVal, nullptr,
          cmSystemTools::OUTPUT_NONE, cmDuration::zero(),
          cmProcessOutput::Auto);
        if (!runResult) {
          error = cmStrCat("Test run of \"", generator, "\" executable ",
                           cmQtAutoGen::Quoted(exe), " failed.\n",
                           cmQtAutoGen::QuotedCommand(command), '\n', stdOut,
                           '\n', stdErr);
          res.Config[config.first] = {};
          continue;
        }
      }

      // Create valid handle
      res.Config[config.first] =
        std::make_shared<cmQtAutoGen::CompilerFeatures>();
      res.Config[config.first]->HelpOutput = std::move(stdOut);

      // Register compiler features
      this->CompilerFeatures_.Config[config.first].emplace(
        exe, res.Config[config.first]);
    }
    return res;
  }

  // Check if we have cached features
  {
    auto const it = this->CompilerFeatures_.Default.find(executable.Default);
    if (it != this->CompilerFeatures_.Default.end()) {
      res.Default = it->second;
      return res;
    }
  }

  // Check if the executable exists
  if (!cmSystemTools::FileExists(executable.Default, true)) {
    error =
      cmStrCat("The \"", generator, "\" executable ",
               cmQtAutoGen::Quoted(executable.Default), " does not exist.");
    return cmQtAutoGen::ConfigStrings<cmQtAutoGen::CompilerFeaturesHandle>();
  }

  // Test the executable
  std::string stdOut;
  {
    std::string stdErr;
    std::vector<std::string> command;
    command.emplace_back(executable.Default);
    command.emplace_back("-h");
    int retVal = 0;
    const bool runResult = cmSystemTools::RunSingleCommand(
      command, &stdOut, &stdErr, &retVal, nullptr, cmSystemTools::OUTPUT_NONE,
      cmDuration::zero(), cmProcessOutput::Auto);
    if (!runResult) {
      error = cmStrCat("Test run of \"", generator, "\" executable ",
                       cmQtAutoGen::Quoted(executable.Default), " failed.\n",
                       cmQtAutoGen::QuotedCommand(command), '\n', stdOut, '\n',
                       stdErr);
      return cmQtAutoGen::ConfigStrings<cmQtAutoGen::CompilerFeaturesHandle>();
    }
  }

  res.Default = std::make_shared<cmQtAutoGen::CompilerFeatures>();
  res.Default->HelpOutput = std::move(stdOut);

  // Register compiler features
  this->CompilerFeatures_.Default.emplace(executable.Default, res.Default);

  return res;
}

bool cmQtAutoGenGlobalInitializer::InitializeCustomTargets()
{
  // Initialize global autogen targets
  {
    std::string const comment = "Global AUTOGEN target";
    for (auto const& pair : this->GlobalAutoGenTargets_) {
      this->GetOrCreateGlobalTarget(pair.first, pair.second, comment);
    }
  }
  // Initialize global autorcc targets
  {
    std::string const comment = "Global AUTORCC target";
    for (auto const& pair : this->GlobalAutoRccTargets_) {
      this->GetOrCreateGlobalTarget(pair.first, pair.second, comment);
    }
  }
  // Initialize per target autogen targets
  for (auto& initializer : this->Initializers_) {
    if (!initializer->InitCustomTargets()) {
      return false;
    }
  }
  return true;
}

bool cmQtAutoGenGlobalInitializer::SetupCustomTargets()
{
  for (auto& initializer : this->Initializers_) {
    if (!initializer->SetupCustomTargets()) {
      return false;
    }
  }
  return true;
}
