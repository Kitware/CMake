/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmDebuggerVariablesHelper.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iomanip>
#include <map>
#include <sstream>

#include "cm_codecvt.hxx"

#include "cmDebuggerStackFrame.h"
#include "cmDebuggerVariables.h"
#include "cmFileSet.h"
#include "cmGlobalGenerator.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmPropertyMap.h"
#include "cmState.h"
#include "cmStateSnapshot.h"
#include "cmTarget.h"
#include "cmTest.h"
#include "cmValue.h"
#include "cmake.h"

namespace cmDebugger {

std::shared_ptr<cmDebuggerVariables> cmDebuggerVariablesHelper::Create(
  std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
  std::string const& name, bool supportsVariableType,
  cmPolicies::PolicyMap const& policyMap)
{
  static std::map<cmPolicies::PolicyStatus, std::string> policyStatusString = {
    { cmPolicies::PolicyStatus::OLD, "OLD" },
    { cmPolicies::PolicyStatus::WARN, "WARN" },
    { cmPolicies::PolicyStatus::NEW, "NEW" },
    { cmPolicies::PolicyStatus::REQUIRED_IF_USED, "REQUIRED_IF_USED" },
    { cmPolicies::PolicyStatus::REQUIRED_ALWAYS, "REQUIRED_ALWAYS" }
  };

  return std::make_shared<cmDebuggerVariables>(
    variablesManager, name, supportsVariableType, [=]() {
      std::vector<cmDebuggerVariableEntry> ret;
      ret.reserve(cmPolicies::CMPCOUNT);
      for (int i = 0; i < cmPolicies::CMPCOUNT; ++i) {
        if (policyMap.IsDefined(static_cast<cmPolicies::PolicyID>(i))) {
          auto status = policyMap.Get(static_cast<cmPolicies::PolicyID>(i));
          std::ostringstream ss;
          ss << "CMP" << std::setfill('0') << std::setw(4) << i;
          ret.emplace_back(ss.str(), policyStatusString[status]);
        }
      }
      return ret;
    });
}

std::shared_ptr<cmDebuggerVariables> cmDebuggerVariablesHelper::CreateIfAny(
  std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
  std::string const& name, bool supportsVariableType,
  std::vector<std::pair<std::string, std::string>> const& list)
{
  if (list.empty()) {
    return {};
  }

  auto listVariables = std::make_shared<cmDebuggerVariables>(
    variablesManager, name, supportsVariableType, [=]() {
      std::vector<cmDebuggerVariableEntry> ret;
      ret.reserve(list.size());
      for (auto const& kv : list) {
        ret.emplace_back(kv.first, kv.second);
      }
      return ret;
    });

  listVariables->SetValue(std::to_string(list.size()));
  return listVariables;
}

std::shared_ptr<cmDebuggerVariables> cmDebuggerVariablesHelper::CreateIfAny(
  std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
  std::string const& name, bool supportsVariableType,
  cmBTStringRange const& entries)
{
  if (entries.empty()) {
    return {};
  }

  auto sourceEntries = std::make_shared<cmDebuggerVariables>(
    variablesManager, name, supportsVariableType);

  for (auto const& entry : entries) {
    auto arrayVariables = std::make_shared<cmDebuggerVariables>(
      variablesManager, entry.Value, supportsVariableType, [=]() {
        cmList items{ entry.Value };
        std::vector<cmDebuggerVariableEntry> ret;
        ret.reserve(items.size());
        int i = 0;
        for (std::string const& item : items) {
          ret.emplace_back("[" + std::to_string(i++) + "]", item);
        }
        return ret;
      });
    arrayVariables->SetEnableSorting(false);
    sourceEntries->AddSubVariables(arrayVariables);
  }

  sourceEntries->SetValue(std::to_string(entries.size()));
  return sourceEntries;
}

std::shared_ptr<cmDebuggerVariables> cmDebuggerVariablesHelper::CreateIfAny(
  std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
  std::string const& name, bool supportsVariableType,
  std::set<std::string> const& values)
{
  if (values.empty()) {
    return {};
  }

  auto arrayVariables = std::make_shared<cmDebuggerVariables>(
    variablesManager, name, supportsVariableType, [=]() {
      std::vector<cmDebuggerVariableEntry> ret;
      ret.reserve(values.size());
      int i = 0;
      for (std::string const& value : values) {
        ret.emplace_back("[" + std::to_string(i++) + "]", value);
      }
      return ret;
    });
  arrayVariables->SetValue(std::to_string(values.size()));
  arrayVariables->SetEnableSorting(false);
  return arrayVariables;
}

std::shared_ptr<cmDebuggerVariables> cmDebuggerVariablesHelper::CreateIfAny(
  std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
  std::string const& name, bool supportsVariableType,
  std::vector<std::string> const& values)
{
  if (values.empty()) {
    return {};
  }

  auto arrayVariables = std::make_shared<cmDebuggerVariables>(
    variablesManager, name, supportsVariableType, [=]() {
      std::vector<cmDebuggerVariableEntry> ret;
      ret.reserve(values.size());
      int i = 0;
      for (std::string const& value : values) {
        ret.emplace_back("[" + std::to_string(i++) + "]", value);
      }
      return ret;
    });

  arrayVariables->SetValue(std::to_string(values.size()));
  arrayVariables->SetEnableSorting(false);
  return arrayVariables;
}

std::shared_ptr<cmDebuggerVariables> cmDebuggerVariablesHelper::CreateIfAny(
  std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
  std::string const& name, bool supportsVariableType,
  std::vector<BT<std::string>> const& list)
{
  if (list.empty()) {
    return {};
  }

  auto variables = std::make_shared<cmDebuggerVariables>(
    variablesManager, name, supportsVariableType, [=]() {
      std::vector<cmDebuggerVariableEntry> ret;
      ret.reserve(list.size());
      int i = 0;
      for (auto const& item : list) {
        ret.emplace_back("[" + std::to_string(i++) + "]", item.Value);
      }

      return ret;
    });

  variables->SetValue(std::to_string(list.size()));
  variables->SetEnableSorting(false);
  return variables;
}

std::shared_ptr<cmDebuggerVariables> cmDebuggerVariablesHelper::CreateIfAny(
  std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
  std::string const& name, bool supportsVariableType, cmFileSet* fileSet)
{
  if (fileSet == nullptr) {
    return {};
  }

  static auto visibilityString = [](cmFileSetVisibility visibility) {
    switch (visibility) {
      case cmFileSetVisibility::Private:
        return "Private";
      case cmFileSetVisibility::Public:
        return "Public";
      case cmFileSetVisibility::Interface:
        return "Interface";
      default:
        return "Unknown";
    }
  };

  auto variables = std::make_shared<cmDebuggerVariables>(
    variablesManager, name, supportsVariableType, [=]() {
      std::vector<cmDebuggerVariableEntry> ret{
        { "Name", fileSet->GetName() },
        { "Type", fileSet->GetType() },
        { "Visibility", visibilityString(fileSet->GetVisibility()) },
      };

      return ret;
    });

  variables->AddSubVariables(CreateIfAny(variablesManager, "Directories",
                                         supportsVariableType,
                                         fileSet->GetDirectoryEntries()));
  variables->AddSubVariables(CreateIfAny(variablesManager, "Files",
                                         supportsVariableType,
                                         fileSet->GetFileEntries()));
  return variables;
}

std::shared_ptr<cmDebuggerVariables> cmDebuggerVariablesHelper::CreateIfAny(
  std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
  std::string const& name, bool supportsVariableType,
  std::vector<cmFileSet*> const& fileSets)
{
  if (fileSets.empty()) {
    return {};
  }

  auto fileSetsVariables = std::make_shared<cmDebuggerVariables>(
    variablesManager, name, supportsVariableType);

  for (auto const& fileSet : fileSets) {
    fileSetsVariables->AddSubVariables(CreateIfAny(
      variablesManager, fileSet->GetName(), supportsVariableType, fileSet));
  }

  return fileSetsVariables;
}

std::shared_ptr<cmDebuggerVariables> cmDebuggerVariablesHelper::CreateIfAny(
  std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
  std::string const& name, bool supportsVariableType,
  std::vector<cmTarget*> const& targets)
{
  if (targets.empty()) {
    return {};
  }

  auto targetsVariables = std::make_shared<cmDebuggerVariables>(
    variablesManager, name, supportsVariableType);

  for (auto const& target : targets) {
    auto targetVariables = std::make_shared<cmDebuggerVariables>(
      variablesManager, target->GetName(), supportsVariableType, [=]() {
        std::vector<cmDebuggerVariableEntry> ret = {
          { "InstallPath", target->GetInstallPath() },
          { "IsAIX", target->IsAIX() },
          { "IsAndroidGuiExecutable", target->IsAndroidGuiExecutable() },
          { "IsAppBundleOnApple", target->IsAppBundleOnApple() },
          { "IsDLLPlatform", target->IsDLLPlatform() },
          { "IsExecutableWithExports", target->IsExecutableWithExports() },
          { "IsFrameworkOnApple", target->IsFrameworkOnApple() },
          { "IsImported", target->IsImported() },
          { "IsImportedGloballyVisible", target->IsImportedGloballyVisible() },
          { "IsPerConfig", target->IsPerConfig() },
          { "Name", target->GetName() },
          { "RuntimeInstallPath", target->GetRuntimeInstallPath() },
          { "Type", cmState::GetTargetTypeName(target->GetType()) },
        };

        return ret;
      });
    targetVariables->SetValue(cmState::GetTargetTypeName(target->GetType()));

    targetVariables->AddSubVariables(Create(variablesManager, "PolicyMap",
                                            supportsVariableType,
                                            target->GetPolicyMap()));
    targetVariables->AddSubVariables(
      CreateIfAny(variablesManager, "Properties", supportsVariableType,
                  target->GetProperties().GetList()));

    targetVariables->AddSubVariables(
      CreateIfAny(variablesManager, "IncludeDirectories", supportsVariableType,
                  target->GetIncludeDirectoriesEntries()));
    targetVariables->AddSubVariables(CreateIfAny(variablesManager, "Sources",
                                                 supportsVariableType,
                                                 target->GetSourceEntries()));
    targetVariables->AddSubVariables(
      CreateIfAny(variablesManager, "CompileDefinitions", supportsVariableType,
                  target->GetCompileDefinitionsEntries()));
    targetVariables->AddSubVariables(
      CreateIfAny(variablesManager, "CompileFeatures", supportsVariableType,
                  target->GetCompileFeaturesEntries()));
    targetVariables->AddSubVariables(
      CreateIfAny(variablesManager, "CompileOptions", supportsVariableType,
                  target->GetCompileOptionsEntries()));
    targetVariables->AddSubVariables(
      CreateIfAny(variablesManager, "CxxModuleSets", supportsVariableType,
                  target->GetCxxModuleSetsEntries()));
    targetVariables->AddSubVariables(
      CreateIfAny(variablesManager, "HeaderSets", supportsVariableType,
                  target->GetHeaderSetsEntries()));
    targetVariables->AddSubVariables(CreateIfAny(
      variablesManager, "InterfaceHeaderSets", supportsVariableType,
      target->GetInterfaceHeaderSetsEntries()));
    targetVariables->AddSubVariables(
      CreateIfAny(variablesManager, "LinkDirectories", supportsVariableType,
                  target->GetLinkDirectoriesEntries()));
    targetVariables->AddSubVariables(CreateIfAny(
      variablesManager, "LinkImplementations", supportsVariableType,
      target->GetLinkImplementationEntries()));
    targetVariables->AddSubVariables(CreateIfAny(
      variablesManager, "LinkInterfaceDirects", supportsVariableType,
      target->GetLinkInterfaceDirectEntries()));
    targetVariables->AddSubVariables(CreateIfAny(
      variablesManager, "LinkInterfaceDirectExcludes", supportsVariableType,
      target->GetLinkInterfaceDirectExcludeEntries()));
    targetVariables->AddSubVariables(
      CreateIfAny(variablesManager, "LinkInterfaces", supportsVariableType,
                  target->GetLinkInterfaceEntries()));
    targetVariables->AddSubVariables(
      CreateIfAny(variablesManager, "LinkOptions", supportsVariableType,
                  target->GetLinkOptionsEntries()));
    targetVariables->AddSubVariables(CreateIfAny(
      variablesManager, "SystemIncludeDirectories", supportsVariableType,
      target->GetSystemIncludeDirectories()));
    targetVariables->AddSubVariables(CreateIfAny(variablesManager, "Makefile",
                                                 supportsVariableType,
                                                 target->GetMakefile()));
    targetVariables->AddSubVariables(
      CreateIfAny(variablesManager, "GlobalGenerator", supportsVariableType,
                  target->GetGlobalGenerator()));

    std::vector<cmFileSet*> allFileSets;
    auto allFileSetNames = target->GetAllFileSetNames();
    allFileSets.reserve(allFileSetNames.size());
    for (auto const& fileSetName : allFileSetNames) {
      allFileSets.emplace_back(target->GetFileSet(fileSetName));
    }
    targetVariables->AddSubVariables(CreateIfAny(
      variablesManager, "AllFileSets", supportsVariableType, allFileSets));

    std::vector<cmFileSet*> allInterfaceFileSets;
    auto allInterfaceFileSetNames = target->GetAllInterfaceFileSets();
    allInterfaceFileSets.reserve(allInterfaceFileSetNames.size());
    for (auto const& interfaceFileSetName : allInterfaceFileSetNames) {
      allInterfaceFileSets.emplace_back(
        target->GetFileSet(interfaceFileSetName));
    }
    targetVariables->AddSubVariables(
      CreateIfAny(variablesManager, "AllInterfaceFileSets",
                  supportsVariableType, allInterfaceFileSets));

    targetVariables->SetIgnoreEmptyStringEntries(true);
    targetsVariables->AddSubVariables(targetVariables);
  }

  targetsVariables->SetValue(std::to_string(targets.size()));
  return targetsVariables;
}

std::shared_ptr<cmDebuggerVariables> cmDebuggerVariablesHelper::Create(
  std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
  std::string const& name, bool supportsVariableType,
  std::shared_ptr<cmDebuggerStackFrame> const& frame)
{
  auto variables = std::make_shared<cmDebuggerVariables>(
    variablesManager, name, supportsVariableType, [=]() {
      return std::vector<cmDebuggerVariableEntry>{ { "CurrentLine",
                                                     frame->GetLine() } };
    });

  auto closureKeys = frame->GetMakefile()->GetStateSnapshot().ClosureKeys();
  auto locals = std::make_shared<cmDebuggerVariables>(
    variablesManager, "Locals", supportsVariableType, [=]() {
      std::vector<cmDebuggerVariableEntry> ret;
      ret.reserve(closureKeys.size());
      for (auto const& key : closureKeys) {
        ret.emplace_back(
          key, frame->GetMakefile()->GetStateSnapshot().GetDefinition(key));
      }
      return ret;
    });
  locals->SetValue(std::to_string(closureKeys.size()));
  variables->AddSubVariables(locals);

  std::function<bool(std::string const&)> isDirectory =
    [](std::string const& key) {
      size_t pos1 = key.rfind("_DIR");
      size_t pos2 = key.rfind("_DIRECTORY");
      return !((pos1 == std::string::npos || pos1 != key.size() - 4) &&
               (pos2 == std::string::npos || pos2 != key.size() - 10));
    };
  auto directorySize =
    std::count_if(closureKeys.begin(), closureKeys.end(), isDirectory);
  auto directories = std::make_shared<cmDebuggerVariables>(
    variablesManager, "Directories", supportsVariableType, [=]() {
      std::vector<cmDebuggerVariableEntry> ret;
      ret.reserve(directorySize);
      for (auto const& key : closureKeys) {
        if (isDirectory(key)) {
          ret.emplace_back(
            key, frame->GetMakefile()->GetStateSnapshot().GetDefinition(key));
        }
      }
      return ret;
    });
  directories->SetValue(std::to_string(directorySize));
  variables->AddSubVariables(directories);

  auto cacheVariables = std::make_shared<cmDebuggerVariables>(
    variablesManager, "CacheVariables", supportsVariableType);
  auto* state = frame->GetMakefile()->GetCMakeInstance()->GetState();
  auto keys = state->GetCacheEntryKeys();
  for (auto const& key : keys) {
    auto entry = std::make_shared<cmDebuggerVariables>(
      variablesManager,
      key + ":" +
        cmState::CacheEntryTypeToString(state->GetCacheEntryType(key)),
      supportsVariableType, [=]() {
        std::vector<cmDebuggerVariableEntry> ret;
        auto properties = state->GetCacheEntryPropertyList(key);
        ret.reserve(properties.size() + 2);
        for (auto const& propertyName : properties) {
          ret.emplace_back(propertyName,
                           state->GetCacheEntryProperty(key, propertyName));
        }

        ret.emplace_back(
          "TYPE",
          cmState::CacheEntryTypeToString(state->GetCacheEntryType(key)));
        ret.emplace_back("VALUE", state->GetCacheEntryValue(key));
        return ret;
      });

    entry->SetValue(state->GetCacheEntryValue(key));
    cacheVariables->AddSubVariables(entry);
  }

  cacheVariables->SetValue(std::to_string(keys.size()));
  variables->AddSubVariables(cacheVariables);

  auto targetVariables =
    CreateIfAny(variablesManager, "Targets", supportsVariableType,
                frame->GetMakefile()->GetOrderedTargets());

  variables->AddSubVariables(targetVariables);
  std::vector<cmTest*> tests;
  frame->GetMakefile()->GetTests(
    frame->GetMakefile()->GetDefaultConfiguration(), tests);
  variables->AddSubVariables(
    CreateIfAny(variablesManager, "Tests", supportsVariableType, tests));

  return variables;
}

std::shared_ptr<cmDebuggerVariables> cmDebuggerVariablesHelper::CreateIfAny(
  std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
  std::string const& name, bool supportsVariableType, cmTest* test)
{
  if (test == nullptr) {
    return {};
  }

  auto variables = std::make_shared<cmDebuggerVariables>(
    variablesManager, name, supportsVariableType, [=]() {
      std::vector<cmDebuggerVariableEntry> ret{
        { "CommandExpandLists", test->GetCommandExpandLists() },
        { "Name", test->GetName() },
        { "OldStyle", test->GetOldStyle() },
      };

      return ret;
    });

  variables->AddSubVariables(CreateIfAny(
    variablesManager, "Command", supportsVariableType, test->GetCommand()));

  variables->AddSubVariables(CreateIfAny(variablesManager, "Properties",
                                         supportsVariableType,
                                         test->GetProperties().GetList()));
  return variables;
}

std::shared_ptr<cmDebuggerVariables> cmDebuggerVariablesHelper::CreateIfAny(
  std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
  std::string const& name, bool supportsVariableType,
  std::vector<cmTest*> const& tests)
{
  if (tests.empty()) {
    return {};
  }

  auto variables = std::make_shared<cmDebuggerVariables>(
    variablesManager, name, supportsVariableType);

  for (auto const& test : tests) {
    variables->AddSubVariables(CreateIfAny(variablesManager, test->GetName(),
                                           supportsVariableType, test));
  }
  variables->SetValue(std::to_string(tests.size()));
  return variables;
}

std::shared_ptr<cmDebuggerVariables> cmDebuggerVariablesHelper::CreateIfAny(
  std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
  std::string const& name, bool supportsVariableType, cmMakefile* mf)
{
  if (mf == nullptr) {
    return {};
  }

  auto AppleSDKTypeString = [&](cmMakefile::AppleSDK sdk) {
    switch (sdk) {
      case cmMakefile::AppleSDK::MacOS:
        return "MacOS";
      case cmMakefile::AppleSDK::IPhoneOS:
        return "IPhoneOS";
      case cmMakefile::AppleSDK::IPhoneSimulator:
        return "IPhoneSimulator";
      case cmMakefile::AppleSDK::AppleTVOS:
        return "AppleTVOS";
      case cmMakefile::AppleSDK::AppleTVSimulator:
        return "AppleTVSimulator";
      default:
        return "Unknown";
    }
  };

  auto variables = std::make_shared<cmDebuggerVariables>(
    variablesManager, name, supportsVariableType, [=]() {
      std::vector<cmDebuggerVariableEntry> ret = {
        { "DefineFlags", mf->GetDefineFlags() },
        { "DirectoryId", mf->GetDirectoryId().String },
        { "IsRootMakefile", mf->IsRootMakefile() },
        { "HomeDirectory", mf->GetHomeDirectory() },
        { "HomeOutputDirectory", mf->GetHomeOutputDirectory() },
        { "CurrentSourceDirectory", mf->GetCurrentSourceDirectory() },
        { "CurrentBinaryDirectory", mf->GetCurrentBinaryDirectory() },
        { "PlatformIs32Bit", mf->PlatformIs32Bit() },
        { "PlatformIs64Bit", mf->PlatformIs64Bit() },
        { "PlatformIsx32", mf->PlatformIsx32() },
        { "AppleSDKType", AppleSDKTypeString(mf->GetAppleSDKType()) },
        { "PlatformIsAppleEmbedded", mf->PlatformIsAppleEmbedded() }
      };

      return ret;
    });

  variables->AddSubVariables(CreateIfAny(
    variablesManager, "ListFiles", supportsVariableType, mf->GetListFiles()));
  variables->AddSubVariables(CreateIfAny(variablesManager, "OutputFiles",
                                         supportsVariableType,
                                         mf->GetOutputFiles()));

  variables->SetIgnoreEmptyStringEntries(true);
  variables->SetValue(mf->GetDirectoryId().String);
  return variables;
}

std::shared_ptr<cmDebuggerVariables> cmDebuggerVariablesHelper::CreateIfAny(
  std::shared_ptr<cmDebuggerVariablesManager> const& variablesManager,
  std::string const& name, bool supportsVariableType, cmGlobalGenerator* gen)
{
  if (gen == nullptr) {
    return {};
  }

  auto makeFileEncodingString = [](codecvt::Encoding encoding) {
    switch (encoding) {
      case codecvt::Encoding::None:
        return "None";
      case codecvt::Encoding::UTF8:
        return "UTF8";
      case codecvt::Encoding::UTF8_WITH_BOM:
        return "UTF8_WITH_BOM";
      case codecvt::Encoding::ANSI:
        return "ANSI";
      case codecvt::Encoding::ConsoleOutput:
        return "ConsoleOutput";
      default:
        return "Unknown";
    }
  };

  auto variables = std::make_shared<cmDebuggerVariables>(
    variablesManager, name, supportsVariableType, [=]() {
      std::vector<cmDebuggerVariableEntry> ret = {
        { "AllTargetName", gen->GetAllTargetName() },
        { "CleanTargetName", gen->GetCleanTargetName() },
        { "EditCacheCommand", gen->GetEditCacheCommand() },
        { "EditCacheTargetName", gen->GetEditCacheTargetName() },
        { "ExtraGeneratorName", gen->GetExtraGeneratorName() },
        { "ForceUnixPaths", gen->GetForceUnixPaths() },
        { "InstallLocalTargetName", gen->GetInstallLocalTargetName() },
        { "InstallStripTargetName", gen->GetInstallStripTargetName() },
        { "InstallTargetName", gen->GetInstallTargetName() },
        { "IsMultiConfig", gen->IsMultiConfig() },
        { "Name", gen->GetName() },
        { "MakefileEncoding",
          makeFileEncodingString(gen->GetMakefileEncoding()) },
        { "PackageSourceTargetName", gen->GetPackageSourceTargetName() },
        { "PackageTargetName", gen->GetPackageTargetName() },
        { "PreinstallTargetName", gen->GetPreinstallTargetName() },
        { "NeedSymbolicMark", gen->GetNeedSymbolicMark() },
        { "RebuildCacheTargetName", gen->GetRebuildCacheTargetName() },
        { "TestTargetName", gen->GetTestTargetName() },
        { "UseLinkScript", gen->GetUseLinkScript() },
      };

      return ret;
    });

  if (gen->GetInstallComponents() != nullptr) {
    variables->AddSubVariables(
      CreateIfAny(variablesManager, "InstallComponents", supportsVariableType,
                  *gen->GetInstallComponents()));
  }

  variables->SetIgnoreEmptyStringEntries(true);
  variables->SetValue(gen->GetName());

  return variables;
}

} // namespace cmDebugger
