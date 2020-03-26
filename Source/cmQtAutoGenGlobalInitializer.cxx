/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoGenGlobalInitializer.h"

#include <utility>

#include <cm/memory>

#include "cmCustomCommandLines.h"
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
      cmMakefile* makefile = localGen->GetMakefile();
      // Detect global autogen target name
      if (cmIsOn(makefile->GetSafeDefinition("CMAKE_GLOBAL_AUTOGEN_TARGET"))) {
        std::string targetName =
          makefile->GetSafeDefinition("CMAKE_GLOBAL_AUTOGEN_TARGET_NAME");
        if (targetName.empty()) {
          targetName = "autogen";
        }
        GlobalAutoGenTargets_.emplace(localGen.get(), std::move(targetName));
        globalAutoGenTarget = true;
      }

      // Detect global autorcc target name
      if (cmIsOn(makefile->GetSafeDefinition("CMAKE_GLOBAL_AUTORCC_TARGET"))) {
        std::string targetName =
          makefile->GetSafeDefinition("CMAKE_GLOBAL_AUTORCC_TARGET_NAME");
        if (targetName.empty()) {
          targetName = "autorcc";
        }
        GlobalAutoRccTargets_.emplace(localGen.get(), std::move(targetName));
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

      bool const moc = target->GetPropertyAsBool(kw().AUTOMOC);
      bool const uic = target->GetPropertyAsBool(kw().AUTOUIC);
      bool const rcc = target->GetPropertyAsBool(kw().AUTORCC);
      if (moc || uic || rcc) {
        std::string const mocExec =
          target->GetSafeProperty(kw().AUTOMOC_EXECUTABLE);
        std::string const uicExec =
          target->GetSafeProperty(kw().AUTOUIC_EXECUTABLE);
        std::string const rccExec =
          target->GetSafeProperty(kw().AUTORCC_EXECUTABLE);

        // We support Qt4, Qt5 and Qt6
        auto qtVersion = cmQtAutoGenInitializer::GetQtVersion(target.get());
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
          Initializers_.emplace_back(cm::make_unique<cmQtAutoGenInitializer>(
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
    cmMakefile* makefile = localGen->GetMakefile();

    // Create utility target
    std::vector<std::string> no_byproducts;
    std::vector<std::string> no_depends;
    cmCustomCommandLines no_commands;
    cmTarget* target = localGen->AddUtilityCommand(
      name, true, makefile->GetHomeOutputDirectory().c_str(), no_byproducts,
      no_depends, no_commands, false, comment.c_str());
    localGen->AddGeneratorTarget(
      cm::make_unique<cmGeneratorTarget>(target, localGen));

    // Set FOLDER property in the target
    {
      char const* folder =
        makefile->GetState()->GetGlobalProperty("AUTOGEN_TARGETS_FOLDER");
      if (folder != nullptr) {
        target->SetProperty("FOLDER", folder);
      }
    }
  }
}

void cmQtAutoGenGlobalInitializer::AddToGlobalAutoGen(
  cmLocalGenerator* localGen, std::string const& targetName)
{
  auto it = GlobalAutoGenTargets_.find(localGen);
  if (it != GlobalAutoGenTargets_.end()) {
    cmGeneratorTarget* target = localGen->FindGeneratorTargetToUse(it->second);
    if (target != nullptr) {
      target->Target->AddUtility(targetName, false, localGen->GetMakefile());
    }
  }
}

void cmQtAutoGenGlobalInitializer::AddToGlobalAutoRcc(
  cmLocalGenerator* localGen, std::string const& targetName)
{
  auto it = GlobalAutoRccTargets_.find(localGen);
  if (it != GlobalAutoRccTargets_.end()) {
    cmGeneratorTarget* target = localGen->FindGeneratorTargetToUse(it->second);
    if (target != nullptr) {
      target->Target->AddUtility(targetName, false, localGen->GetMakefile());
    }
  }
}

cmQtAutoGen::CompilerFeaturesHandle
cmQtAutoGenGlobalInitializer::GetCompilerFeatures(
  std::string const& generator, std::string const& executable,
  std::string& error)
{
  // Check if we have cached features
  {
    auto it = this->CompilerFeatures_.find(executable);
    if (it != this->CompilerFeatures_.end()) {
      return it->second;
    }
  }

  // Check if the executable exists
  if (!cmSystemTools::FileExists(executable, true)) {
    error = cmStrCat("The \"", generator, "\" executable ",
                     cmQtAutoGen::Quoted(executable), " does not exist.");
    return cmQtAutoGen::CompilerFeaturesHandle();
  }

  // Test the executable
  std::string stdOut;
  {
    std::string stdErr;
    std::vector<std::string> command;
    command.emplace_back(executable);
    command.emplace_back("-h");
    int retVal = 0;
    const bool runResult = cmSystemTools::RunSingleCommand(
      command, &stdOut, &stdErr, &retVal, nullptr, cmSystemTools::OUTPUT_NONE,
      cmDuration::zero(), cmProcessOutput::Auto);
    if (!runResult) {
      error = cmStrCat("Test run of \"", generator, "\" executable ",
                       cmQtAutoGen::Quoted(executable), " failed.\n",
                       cmQtAutoGen::QuotedCommand(command), '\n', stdOut, '\n',
                       stdErr);
      return cmQtAutoGen::CompilerFeaturesHandle();
    }
  }

  // Create valid handle
  cmQtAutoGen::CompilerFeaturesHandle res =
    std::make_shared<cmQtAutoGen::CompilerFeatures>();
  res->HelpOutput = std::move(stdOut);

  // Register compiler features
  this->CompilerFeatures_.emplace(executable, res);

  return res;
}

bool cmQtAutoGenGlobalInitializer::generate()
{
  return (InitializeCustomTargets() && SetupCustomTargets());
}

bool cmQtAutoGenGlobalInitializer::InitializeCustomTargets()
{
  // Initialize global autogen targets
  {
    std::string const comment = "Global AUTOGEN target";
    for (auto const& pair : GlobalAutoGenTargets_) {
      GetOrCreateGlobalTarget(pair.first, pair.second, comment);
    }
  }
  // Initialize global autorcc targets
  {
    std::string const comment = "Global AUTORCC target";
    for (auto const& pair : GlobalAutoRccTargets_) {
      GetOrCreateGlobalTarget(pair.first, pair.second, comment);
    }
  }
  // Initialize per target autogen targets
  for (auto& initializer : Initializers_) {
    if (!initializer->InitCustomTargets()) {
      return false;
    }
  }
  return true;
}

bool cmQtAutoGenGlobalInitializer::SetupCustomTargets()
{
  for (auto& initializer : Initializers_) {
    if (!initializer->SetupCustomTargets()) {
      return false;
    }
  }
  return true;
}
