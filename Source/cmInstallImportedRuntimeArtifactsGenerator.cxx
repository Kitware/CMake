/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmInstallImportedRuntimeArtifactsGenerator.h"

#include <cassert>
#include <string>
#include <utility>
#include <vector>

#include "cmsys/RegularExpression.hxx"

#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmInstallType.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"

namespace {
const cmsys::RegularExpression FrameworkRegularExpression(
  "^(.*/)?([^/]*)\\.framework/(.*)$");

const cmsys::RegularExpression BundleRegularExpression(
  "^(.*/)?([^/]*)\\.app/(.*)$");

const cmsys::RegularExpression CFBundleRegularExpression(
  "^(.*/)?([^/]*)\\.bundle/(.*)$");
}

cmInstallImportedRuntimeArtifactsGenerator::
  cmInstallImportedRuntimeArtifactsGenerator(
    std::string targetName, std::string const& dest,
    std::string file_permissions,
    std::vector<std::string> const& configurations,
    std::string const& component, MessageLevel message, bool exclude_from_all,
    bool optional, cmListFileBacktrace backtrace)
  : cmInstallGenerator(dest, configurations, component, message,
                       exclude_from_all, false, std::move(backtrace))
  , TargetName(std::move(targetName))
  , FilePermissions(std::move(file_permissions))
  , Optional(optional)
{
  this->ActionsPerConfig = true;
}

bool cmInstallImportedRuntimeArtifactsGenerator::Compute(cmLocalGenerator* lg)
{
  // Lookup this target in the current directory.
  this->Target = lg->FindGeneratorTargetToUse(this->TargetName);
  if (!this->Target || !this->Target->IsImported()) {
    // If no local target has been found, find it in the global scope.
    this->Target =
      lg->GetGlobalGenerator()->FindGeneratorTarget(this->TargetName);
  }

  return true;
}

std::string cmInstallImportedRuntimeArtifactsGenerator::GetDestination(
  std::string const& config) const
{
  return cmGeneratorExpression::Evaluate(
    this->Destination, this->Target->GetLocalGenerator(), config);
}

void cmInstallImportedRuntimeArtifactsGenerator::GenerateScriptForConfig(
  std::ostream& os, const std::string& config, Indent indent)
{
  auto location = this->Target->GetFullPath(config);

  switch (this->Target->GetType()) {
    case cmStateEnums::EXECUTABLE:
      if (this->Target->IsBundleOnApple()) {
        cmsys::RegularExpressionMatch match;
        if (BundleRegularExpression.find(location.c_str(), match)) {
          auto bundleDir = match.match(1);
          auto bundleName = match.match(2);
          auto bundlePath = cmStrCat(bundleDir, bundleName, ".app");
          this->AddInstallRule(os, this->GetDestination(config),
                               cmInstallType_DIRECTORY, { bundlePath },
                               this->Optional, nullptr,
                               this->FilePermissions.c_str(), nullptr,
                               " USE_SOURCE_PERMISSIONS", indent);
        }
      } else {
        this->AddInstallRule(os, this->GetDestination(config),
                             cmInstallType_EXECUTABLE, { location },
                             this->Optional, this->FilePermissions.c_str(),
                             nullptr, nullptr, nullptr, indent);
      }
      break;
    case cmStateEnums::SHARED_LIBRARY:
      if (this->Target->IsFrameworkOnApple()) {
        cmsys::RegularExpressionMatch match;
        if (FrameworkRegularExpression.find(location.c_str(), match)) {
          auto frameworkDir = match.match(1);
          auto frameworkName = match.match(2);
          auto frameworkPath =
            cmStrCat(frameworkDir, frameworkName, ".framework");
          this->AddInstallRule(os, this->GetDestination(config),
                               cmInstallType_DIRECTORY, { frameworkPath },
                               this->Optional, nullptr,
                               this->FilePermissions.c_str(), nullptr,
                               " USE_SOURCE_PERMISSIONS", indent);
        }
      } else {
        std::vector<std::string> files{ location };
        auto soName = this->Target->GetSOName(config);
        auto soNameFile =
          cmStrCat(this->Target->GetDirectory(config), '/', soName);
        if (!soName.empty() && soNameFile != location) {
          files.push_back(soNameFile);
        }
        this->AddInstallRule(os, this->GetDestination(config),
                             cmInstallType_SHARED_LIBRARY, files,
                             this->Optional, this->FilePermissions.c_str(),
                             nullptr, nullptr, nullptr, indent);
      }
      break;
    case cmStateEnums::MODULE_LIBRARY:
      if (this->Target->IsCFBundleOnApple()) {
        cmsys::RegularExpressionMatch match;
        if (CFBundleRegularExpression.find(location.c_str(), match)) {
          auto bundleDir = match.match(1);
          auto bundleName = match.match(2);
          auto bundlePath = cmStrCat(bundleDir, bundleName, ".bundle");
          this->AddInstallRule(os, this->GetDestination(config),
                               cmInstallType_DIRECTORY, { bundlePath },
                               this->Optional, nullptr,
                               this->FilePermissions.c_str(), nullptr,
                               " USE_SOURCE_PERMISSIONS", indent);
        }
      } else {
        this->AddInstallRule(os, this->GetDestination(config),
                             cmInstallType_MODULE_LIBRARY, { location },
                             this->Optional, this->FilePermissions.c_str(),
                             nullptr, nullptr, nullptr, indent);
      }
      break;
    default:
      assert(false && "This should never happen");
      break;
  }
}
