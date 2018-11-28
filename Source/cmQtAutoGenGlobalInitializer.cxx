/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoGenGlobalInitializer.h"
#include "cmQtAutoGen.h"
#include "cmQtAutoGenInitializer.h"

#include "cmAlgorithms.h"
#include "cmCustomCommandLines.h"
#include "cmGeneratorTarget.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmSystemTools.h"
#include "cmTarget.h"

#include <memory>
#include <utility>

cmQtAutoGenGlobalInitializer::cmQtAutoGenGlobalInitializer(
  std::vector<cmLocalGenerator*> const& localGenerators)
{
  for (cmLocalGenerator* localGen : localGenerators) {
    // Detect global autogen and autorcc target names
    bool globalAutoGenTarget = false;
    bool globalAutoRccTarget = false;
    {
      cmMakefile* makefile = localGen->GetMakefile();
      // Detect global autogen target name
      if (cmSystemTools::IsOn(
            makefile->GetSafeDefinition("CMAKE_GLOBAL_AUTOGEN_TARGET"))) {
        std::string targetName =
          makefile->GetSafeDefinition("CMAKE_GLOBAL_AUTOGEN_TARGET_NAME");
        if (targetName.empty()) {
          targetName = "autogen";
        }
        GlobalAutoGenTargets_.emplace(localGen, std::move(targetName));
        globalAutoGenTarget = true;
      }

      // Detect global autorcc target name
      if (cmSystemTools::IsOn(
            makefile->GetSafeDefinition("CMAKE_GLOBAL_AUTORCC_TARGET"))) {
        std::string targetName =
          makefile->GetSafeDefinition("CMAKE_GLOBAL_AUTORCC_TARGET_NAME");
        if (targetName.empty()) {
          targetName = "autorcc";
        }
        GlobalAutoRccTargets_.emplace(localGen, std::move(targetName));
        globalAutoRccTarget = true;
      }
    }

    // Find targets that require AUTOMOC/UIC/RCC processing
    for (cmGeneratorTarget* target : localGen->GetGeneratorTargets()) {
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

      bool const moc = target->GetPropertyAsBool("AUTOMOC");
      bool const uic = target->GetPropertyAsBool("AUTOUIC");
      bool const rcc = target->GetPropertyAsBool("AUTORCC");
      if (moc || uic || rcc) {
        std::string const mocExec =
          target->GetSafeProperty("AUTOMOC_EXECUTABLE");
        std::string const uicExec =
          target->GetSafeProperty("AUTOUIC_EXECUTABLE");
        std::string const rccExec =
          target->GetSafeProperty("AUTORCC_EXECUTABLE");

        // We support Qt4, Qt5 and Qt6
        auto qtVersion = cmQtAutoGenInitializer::GetQtVersion(target);
        bool const validQt = (qtVersion.Major == 4) ||
          (qtVersion.Major == 5) || (qtVersion.Major == 6);
        bool const mocIsValid = moc && (validQt || !mocExec.empty());
        bool const uicIsValid = uic && (validQt || !uicExec.empty());
        bool const rccIsValid = rcc && (validQt || !rccExec.empty());

        if (mocIsValid || uicIsValid || rccIsValid) {
          // Create autogen target initializer
          Initializers_.emplace_back(cm::make_unique<cmQtAutoGenInitializer>(
            this, target, qtVersion, mocIsValid, uicIsValid, rccIsValid,
            globalAutoGenTarget, globalAutoRccTarget));
        }
      }
    }
  }
}

cmQtAutoGenGlobalInitializer::~cmQtAutoGenGlobalInitializer()
{
}

void cmQtAutoGenGlobalInitializer::GetOrCreateGlobalTarget(
  cmLocalGenerator* localGen, std::string const& name,
  std::string const& comment)
{
  // Test if the target already exists
  if (localGen->FindGeneratorTargetToUse(name) == nullptr) {
    cmMakefile* makefile = localGen->GetMakefile();

    // Create utility target
    cmTarget* target = makefile->AddUtilityCommand(
      name, cmMakefile::TargetOrigin::Generator, true,
      makefile->GetHomeOutputDirectory().c_str() /*work dir*/,
      std::vector<std::string>() /*output*/,
      std::vector<std::string>() /*depends*/, cmCustomCommandLines(), false,
      comment.c_str());
    localGen->AddGeneratorTarget(new cmGeneratorTarget(target, localGen));

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
      target->Target->AddUtility(targetName, localGen->GetMakefile());
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
      target->Target->AddUtility(targetName, localGen->GetMakefile());
    }
  }
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
