/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoGenGlobalInitializer.h"
#include "cmAlgorithms.h"
#include "cmGeneratorTarget.h"
#include "cmLocalGenerator.h"
#include "cmQtAutoGenInitializer.h"

cmQtAutoGenGlobalInitializer::cmQtAutoGenGlobalInitializer(
  std::vector<cmLocalGenerator*> const& localGenerators)
{
  for (cmLocalGenerator* localGen : localGenerators) {
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
        // We support Qt4 and Qt5
        auto qtVersion = cmQtAutoGenInitializer::GetQtVersion(target);
        if ((qtVersion.Major == 4) || (qtVersion.Major == 5)) {
          // Create autogen target initializer
          Initializers_.emplace_back(cm::make_unique<cmQtAutoGenInitializer>(
            target, moc, uic, rcc, qtVersion));
        }
      }
    }
  }
}

cmQtAutoGenGlobalInitializer::~cmQtAutoGenGlobalInitializer()
{
}

bool cmQtAutoGenGlobalInitializer::generate()
{
  return (InitializeCustomTargets() && SetupCustomTargets());
}

bool cmQtAutoGenGlobalInitializer::InitializeCustomTargets()
{
  for (auto& autoGen : Initializers_) {
    if (!autoGen->InitCustomTargets()) {
      return false;
    }
  }
  return true;
}

bool cmQtAutoGenGlobalInitializer::SetupCustomTargets()
{
  for (auto& autoGen : Initializers_) {
    if (!autoGen->SetupCustomTargets()) {
      return false;
    }
  }
  return true;
}
