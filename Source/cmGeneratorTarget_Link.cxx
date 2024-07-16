/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
/* clang-format off */
#include "cmGeneratorTarget.h"
/* clang-format on */

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cm/optional>
#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmAlgorithms.h"
#include "cmComputeLinkInformation.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include "cmGlobalGenerator.h"
#include "cmLinkItem.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmRange.h"
#include "cmSourceFile.h"
#include "cmSourceFileLocationKind.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetLinkLibraryType.h"
#include "cmValue.h"
#include "cmake.h"

namespace {
using UseTo = cmGeneratorTarget::UseTo;

const std::string kINTERFACE_LINK_LIBRARIES = "INTERFACE_LINK_LIBRARIES";
const std::string kINTERFACE_LINK_LIBRARIES_DIRECT =
  "INTERFACE_LINK_LIBRARIES_DIRECT";
const std::string kINTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE =
  "INTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE";

unsigned int CheckLinkLibrariesSuppressionRAIICount;
void MaybeEnableCheckLinkLibraries(cmOptionalLinkImplementation& impl)
{
  if (CheckLinkLibrariesSuppressionRAIICount == 0) {
    impl.CheckLinkLibraries = true;
  }
}
void MaybeEnableCheckLinkLibraries(cmOptionalLinkInterface& iface)
{
  if (CheckLinkLibrariesSuppressionRAIICount == 0) {
    iface.CheckLinkLibraries = true;
  }
}
}

class cmTargetCollectLinkLanguages
{
public:
  cmTargetCollectLinkLanguages(cmGeneratorTarget const* target,
                               std::string config,
                               std::unordered_set<std::string>& languages,
                               cmGeneratorTarget const* head, bool secondPass)
    : Config(std::move(config))
    , Languages(languages)
    , HeadTarget(head)
    , SecondPass(secondPass)
  {
    this->Visited.insert(target);
  }

  void Visit(cmLinkItem const& item)
  {
    if (!item.Target) {
      return;
    }
    if (!this->Visited.insert(item.Target).second) {
      return;
    }
    cmLinkInterface const* iface = item.Target->GetLinkInterface(
      this->Config, this->HeadTarget, this->SecondPass);
    if (!iface) {
      return;
    }
    if (iface->HadLinkLanguageSensitiveCondition) {
      this->HadLinkLanguageSensitiveCondition = true;
    }

    for (std::string const& language : iface->Languages) {
      this->Languages.insert(language);
    }

    for (cmLinkItem const& lib : iface->Libraries) {
      this->Visit(lib);
    }
  }

  bool GetHadLinkLanguageSensitiveCondition() const
  {
    return this->HadLinkLanguageSensitiveCondition;
  }

private:
  std::string Config;
  std::unordered_set<std::string>& Languages;
  cmGeneratorTarget const* HeadTarget;
  std::set<cmGeneratorTarget const*> Visited;
  bool SecondPass;
  bool HadLinkLanguageSensitiveCondition = false;
};

cmGeneratorTarget::LinkClosure const* cmGeneratorTarget::GetLinkClosure(
  const std::string& config) const
{
  // There is no link implementation for targets that cannot compile sources.
  if (!this->CanCompileSources()) {
    static LinkClosure const empty = { {}, {} };
    return &empty;
  }

  std::string key(cmSystemTools::UpperCase(config));
  auto i = this->LinkClosureMap.find(key);
  if (i == this->LinkClosureMap.end()) {
    LinkClosure lc;
    this->ComputeLinkClosure(config, lc);
    LinkClosureMapType::value_type entry(key, lc);
    i = this->LinkClosureMap.insert(entry).first;
  }
  return &i->second;
}

class cmTargetSelectLinker
{
  int Preference = 0;
  cmGeneratorTarget const* Target;
  cmGlobalGenerator* GG;
  std::set<std::string> Preferred;

public:
  cmTargetSelectLinker(cmGeneratorTarget const* target)
    : Target(target)
  {
    this->GG = this->Target->GetLocalGenerator()->GetGlobalGenerator();
  }
  void Consider(const std::string& lang)
  {
    int preference = this->GG->GetLinkerPreference(lang);
    if (preference > this->Preference) {
      this->Preference = preference;
      this->Preferred.clear();
    }
    if (preference == this->Preference) {
      this->Preferred.insert(lang);
    }
  }
  std::string Choose()
  {
    if (this->Preferred.empty()) {
      return "";
    }
    if (this->Preferred.size() > 1) {
      std::ostringstream e;
      e << "Target " << this->Target->GetName()
        << " contains multiple languages with the highest linker preference"
        << " (" << this->Preference << "):\n";
      for (std::string const& li : this->Preferred) {
        e << "  " << li << "\n";
      }
      e << "Set the LINKER_LANGUAGE property for this target.";
      cmake* cm = this->Target->GetLocalGenerator()->GetCMakeInstance();
      cm->IssueMessage(MessageType::FATAL_ERROR, e.str(),
                       this->Target->GetBacktrace());
    }
    return *this->Preferred.begin();
  }
};

bool cmGeneratorTarget::ComputeLinkClosure(const std::string& config,
                                           LinkClosure& lc,
                                           bool secondPass) const
{
  // Get languages built in this target.
  std::unordered_set<std::string> languages;
  cmLinkImplementation const* impl =
    this->GetLinkImplementation(config, UseTo::Link, secondPass);
  assert(impl);
  languages.insert(impl->Languages.cbegin(), impl->Languages.cend());

  // Add interface languages from linked targets.
  // cmTargetCollectLinkLanguages cll(this, config, languages, this,
  // secondPass);
  cmTargetCollectLinkLanguages cll(this, config, languages, this, secondPass);
  for (cmLinkImplItem const& lib : impl->Libraries) {
    cll.Visit(lib);
  }

  // Store the transitive closure of languages.
  cm::append(lc.Languages, languages);

  // Choose the language whose linker should be used.
  if (secondPass || lc.LinkerLanguage.empty()) {
    // Find the language with the highest preference value.
    cmTargetSelectLinker tsl(this);

    // First select from the languages compiled directly in this target.
    for (std::string const& l : impl->Languages) {
      tsl.Consider(l);
    }

    // Now consider languages that propagate from linked targets.
    for (std::string const& lang : languages) {
      std::string propagates =
        "CMAKE_" + lang + "_LINKER_PREFERENCE_PROPAGATES";
      if (this->Makefile->IsOn(propagates)) {
        tsl.Consider(lang);
      }
    }

    lc.LinkerLanguage = tsl.Choose();
  }

  return impl->HadLinkLanguageSensitiveCondition ||
    cll.GetHadLinkLanguageSensitiveCondition();
}

void cmGeneratorTarget::ComputeLinkClosure(const std::string& config,
                                           LinkClosure& lc) const
{
  bool secondPass = false;

  {
    LinkClosure linkClosure;
    linkClosure.LinkerLanguage = this->LinkerLanguage;

    bool hasHardCodedLinkerLanguage = this->Target->GetProperty("HAS_CXX") ||
      !this->Target->GetSafeProperty("LINKER_LANGUAGE").empty();

    // Get languages built in this target.
    secondPass = this->ComputeLinkClosure(config, linkClosure, false) &&
      !hasHardCodedLinkerLanguage;
    this->LinkerLanguage = linkClosure.LinkerLanguage;
    if (!secondPass) {
      lc = std::move(linkClosure);
    }
  }

  if (secondPass) {
    LinkClosure linkClosure;

    this->ComputeLinkClosure(config, linkClosure, secondPass);
    lc = std::move(linkClosure);

    // linker language must not be changed between the two passes
    if (this->LinkerLanguage != lc.LinkerLanguage) {
      std::ostringstream e;
      e << "Evaluation of $<LINK_LANGUAGE:...> or $<LINK_LAND_AND_ID:...> "
           "changes\nthe linker language for target \""
        << this->GetName() << "\" (from '" << this->LinkerLanguage << "' to '"
        << lc.LinkerLanguage << "') which is invalid.";
      cmSystemTools::Error(e.str());
    }
  }
}

static void processILibs(const std::string& config,
                         cmGeneratorTarget const* headTarget,
                         cmLinkItem const& item, cmGlobalGenerator* gg,
                         std::vector<cmGeneratorTarget const*>& tgts,
                         std::set<cmGeneratorTarget const*>& emitted,
                         UseTo usage)
{
  if (item.Target && emitted.insert(item.Target).second) {
    tgts.push_back(item.Target);
    if (cmLinkInterfaceLibraries const* iface =
          item.Target->GetLinkInterfaceLibraries(config, headTarget, usage)) {
      for (cmLinkItem const& lib : iface->Libraries) {
        processILibs(config, headTarget, lib, gg, tgts, emitted, usage);
      }
    }
  }
}

std::vector<cmGeneratorTarget const*>
cmGeneratorTarget::GetLinkInterfaceClosure(std::string const& config,
                                           cmGeneratorTarget const* headTarget,
                                           UseTo usage) const
{
  cmGlobalGenerator* gg = this->GetLocalGenerator()->GetGlobalGenerator();
  std::vector<cmGeneratorTarget const*> tgts;
  std::set<cmGeneratorTarget const*> emitted;
  if (cmLinkInterfaceLibraries const* iface =
        this->GetLinkInterfaceLibraries(config, headTarget, usage)) {
    for (cmLinkItem const& lib : iface->Libraries) {
      processILibs(config, headTarget, lib, gg, tgts, emitted, usage);
    }
  }
  return tgts;
}

const std::vector<const cmGeneratorTarget*>&
cmGeneratorTarget::GetLinkImplementationClosure(const std::string& config,
                                                UseTo usage) const
{
  // There is no link implementation for targets that cannot compile sources.
  if (!this->CanCompileSources()) {
    static std::vector<const cmGeneratorTarget*> const empty;
    return empty;
  }

  LinkImplClosure& tgts =
    (usage == UseTo::Compile ? this->LinkImplClosureForUsageMap[config]
                             : this->LinkImplClosureForLinkMap[config]);
  if (!tgts.Done) {
    tgts.Done = true;
    std::set<cmGeneratorTarget const*> emitted;

    cmLinkImplementationLibraries const* impl =
      this->GetLinkImplementationLibraries(config, usage);
    assert(impl);

    for (cmLinkImplItem const& lib : impl->Libraries) {
      processILibs(config, this, lib,
                   this->LocalGenerator->GetGlobalGenerator(), tgts, emitted,
                   usage);
    }
  }
  return tgts;
}

cmComputeLinkInformation* cmGeneratorTarget::GetLinkInformation(
  const std::string& config) const
{
  // Lookup any existing information for this configuration.
  std::string key(cmSystemTools::UpperCase(config));
  auto i = this->LinkInformation.find(key);
  if (i == this->LinkInformation.end()) {
    // Compute information for this configuration.
    auto info = cm::make_unique<cmComputeLinkInformation>(this, config);
    if (info && !info->Compute()) {
      info.reset();
    }

    // Store the information for this configuration.
    i = this->LinkInformation.emplace(key, std::move(info)).first;

    if (i->second) {
      this->CheckPropertyCompatibility(*i->second, config);
    }
  }
  return i->second.get();
}

void cmGeneratorTarget::CheckLinkLibraries() const
{
  bool linkLibrariesOnlyTargets =
    this->GetPropertyAsBool("LINK_LIBRARIES_ONLY_TARGETS");

  // Evaluate the link interface of this target if needed for extra checks.
  if (linkLibrariesOnlyTargets) {
    std::vector<std::string> const& configs =
      this->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);
    for (std::string const& config : configs) {
      this->GetLinkInterfaceLibraries(config, this, UseTo::Link);
    }
  }

  // Check link the implementation for each generated configuration.
  for (auto const& hmp : this->LinkImplMap) {
    HeadToLinkImplementationMap const& hm = hmp.second;
    // There could be several entries used when computing the pre-CMP0022
    // default link interface.  Check only the entry for our own link impl.
    auto const hmi = hm.find(this);
    if (hmi == hm.end() || !hmi->second.LibrariesDone ||
        !hmi->second.CheckLinkLibraries) {
      continue;
    }
    for (cmLinkImplItem const& item : hmi->second.Libraries) {
      if (!this->VerifyLinkItemColons(LinkItemRole::Implementation, item)) {
        return;
      }
      if (linkLibrariesOnlyTargets &&
          !this->VerifyLinkItemIsTarget(LinkItemRole::Implementation, item)) {
        return;
      }
    }
  }

  // Check link the interface for each generated combination of
  // configuration and consuming head target.  We should not need to
  // consider LinkInterfaceUsageRequirementsOnlyMap because its entries
  // should be a subset of LinkInterfaceMap (with LINK_ONLY left out).
  for (auto const& hmp : this->LinkInterfaceMap) {
    for (auto const& hmi : hmp.second) {
      if (!hmi.second.LibrariesDone || !hmi.second.CheckLinkLibraries) {
        continue;
      }
      for (cmLinkItem const& item : hmi.second.Libraries) {
        if (!this->VerifyLinkItemColons(LinkItemRole::Interface, item)) {
          return;
        }
        if (linkLibrariesOnlyTargets &&
            !this->VerifyLinkItemIsTarget(LinkItemRole::Interface, item)) {
          return;
        }
      }
    }
  }
}

cmGeneratorTarget::CheckLinkLibrariesSuppressionRAII::
  CheckLinkLibrariesSuppressionRAII()
{
  ++CheckLinkLibrariesSuppressionRAIICount;
}

cmGeneratorTarget::CheckLinkLibrariesSuppressionRAII::
  ~CheckLinkLibrariesSuppressionRAII()
{
  --CheckLinkLibrariesSuppressionRAIICount;
}

namespace {
cm::string_view missingTargetPossibleReasons =
  "Possible reasons include:\n"
  "  * There is a typo in the target name.\n"
  "  * A find_package call is missing for an IMPORTED target.\n"
  "  * An ALIAS target is missing.\n"_s;
}

bool cmGeneratorTarget::VerifyLinkItemColons(LinkItemRole role,
                                             cmLinkItem const& item) const
{
  if (item.Target || cmHasPrefix(item.AsStr(), "<LINK_GROUP:"_s) ||
      item.AsStr().find("::") == std::string::npos) {
    return true;
  }
  MessageType messageType = MessageType::FATAL_ERROR;
  std::string e;
  switch (this->GetLocalGenerator()->GetPolicyStatus(cmPolicies::CMP0028)) {
    case cmPolicies::WARN: {
      e = cmStrCat(cmPolicies::GetPolicyWarning(cmPolicies::CMP0028), "\n");
      messageType = MessageType::AUTHOR_WARNING;
    } break;
    case cmPolicies::OLD:
      return true;
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
    case cmPolicies::NEW:
      // Issue the fatal message.
      break;
  }

  if (role == LinkItemRole::Implementation) {
    e = cmStrCat(e, "Target \"", this->GetName(), "\" links to");
  } else {
    e = cmStrCat(e, "The link interface of target \"", this->GetName(),
                 "\" contains");
  }
  e =
    cmStrCat(e, ":\n  ", item.AsStr(), "\n", "but the target was not found.  ",
             missingTargetPossibleReasons);
  cmListFileBacktrace backtrace = item.Backtrace;
  if (backtrace.Empty()) {
    backtrace = this->GetBacktrace();
  }
  this->GetLocalGenerator()->GetCMakeInstance()->IssueMessage(messageType, e,
                                                              backtrace);
  return false;
}

bool cmGeneratorTarget::VerifyLinkItemIsTarget(LinkItemRole role,
                                               cmLinkItem const& item) const
{
  if (item.Target) {
    return true;
  }
  std::string const& str = item.AsStr();
  if (!str.empty() &&
      (str[0] == '-' || str[0] == '$' || str[0] == '`' ||
       str.find_first_of("/\\") != std::string::npos ||
       cmHasPrefix(str, "<LINK_LIBRARY:"_s) ||
       cmHasPrefix(str, "<LINK_GROUP:"_s))) {
    return true;
  }

  std::string e = cmStrCat("Target \"", this->GetName(),
                           "\" has LINK_LIBRARIES_ONLY_TARGETS enabled, but ",
                           role == LinkItemRole::Implementation
                             ? "it links to"
                             : "its link interface contains",
                           ":\n  ", item.AsStr(), "\nwhich is not a target.  ",
                           missingTargetPossibleReasons);
  cmListFileBacktrace backtrace = item.Backtrace;
  if (backtrace.Empty()) {
    backtrace = this->GetBacktrace();
  }
  this->LocalGenerator->GetCMakeInstance()->IssueMessage(
    MessageType::FATAL_ERROR, e, backtrace);
  return false;
}

bool cmGeneratorTarget::IsLinkLookupScope(std::string const& n,
                                          cmLocalGenerator const*& lg) const
{
  if (cmHasLiteralPrefix(n, CMAKE_DIRECTORY_ID_SEP)) {
    cmDirectoryId const dirId = n.substr(cmStrLen(CMAKE_DIRECTORY_ID_SEP));
    if (dirId.String.empty()) {
      lg = this->LocalGenerator;
      return true;
    }
    if (cmLocalGenerator const* otherLG =
          this->GlobalGenerator->FindLocalGenerator(dirId)) {
      lg = otherLG;
      return true;
    }
  }
  return false;
}

cm::optional<cmLinkItem> cmGeneratorTarget::LookupLinkItem(
  std::string const& n, cmListFileBacktrace const& bt,
  std::string const& linkFeature, LookupLinkItemScope* scope,
  LookupSelf lookupSelf) const
{
  cm::optional<cmLinkItem> maybeItem;
  if (this->IsLinkLookupScope(n, scope->LG)) {
    return maybeItem;
  }

  std::string name = this->CheckCMP0004(n);
  if (name.empty() ||
      (lookupSelf == LookupSelf::No && name == this->GetName())) {
    return maybeItem;
  }
  maybeItem =
    this->ResolveLinkItem(BT<std::string>(name, bt), scope->LG, linkFeature);
  return maybeItem;
}

void cmGeneratorTarget::ExpandLinkItems(std::string const& prop,
                                        cmBTStringRange entries,
                                        std::string const& config,
                                        cmGeneratorTarget const* headTarget,
                                        UseTo usage, LinkInterfaceField field,
                                        cmLinkInterface& iface) const
{
  if (entries.empty()) {
    return;
  }
  // Keep this logic in sync with ComputeLinkImplementationLibraries.
  cmGeneratorExpressionDAGChecker dagChecker(this, prop, nullptr, nullptr,
                                             this->LocalGenerator, config);
  // The $<LINK_ONLY> expression may be in a link interface to specify
  // private link dependencies that are otherwise excluded from usage
  // requirements.
  if (usage == UseTo::Compile) {
    dagChecker.SetTransitivePropertiesOnly();
    dagChecker.SetTransitivePropertiesOnlyCMP0131();
  }
  cmMakefile const* mf = this->LocalGenerator->GetMakefile();
  LookupLinkItemScope scope{ this->LocalGenerator };
  for (BT<std::string> const& entry : entries) {
    cmGeneratorExpression ge(*this->LocalGenerator->GetCMakeInstance(),
                             entry.Backtrace);
    std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(entry.Value);
    cge->SetEvaluateForBuildsystem(true);
    cmList libs{ cge->Evaluate(this->LocalGenerator, config, headTarget,
                               &dagChecker, this,
                               headTarget->LinkerLanguage) };

    auto linkFeature = cmLinkItem::DEFAULT;
    for (auto const& lib : libs) {
      if (auto maybeLinkFeature = ParseLinkFeature(lib)) {
        linkFeature = std::move(*maybeLinkFeature);
        continue;
      }

      if (cm::optional<cmLinkItem> maybeItem = this->LookupLinkItem(
            lib, cge->GetBacktrace(), linkFeature, &scope,
            field == LinkInterfaceField::Libraries ? LookupSelf::No
                                                   : LookupSelf::Yes)) {
        cmLinkItem item = std::move(*maybeItem);

        if (field == LinkInterfaceField::HeadInclude) {
          iface.HeadInclude.emplace_back(std::move(item));
          continue;
        }
        if (field == LinkInterfaceField::HeadExclude) {
          iface.HeadExclude.emplace_back(std::move(item));
          continue;
        }
        if (!item.Target) {
          // Report explicitly linked object files separately.
          std::string const& maybeObj = item.AsStr();
          if (cmSystemTools::FileIsFullPath(maybeObj)) {
            cmSourceFile const* sf =
              mf->GetSource(maybeObj, cmSourceFileLocationKind::Known);
            if (sf && sf->GetPropertyAsBool("EXTERNAL_OBJECT")) {
              item.ObjectSource = sf;
              iface.Objects.emplace_back(std::move(item));
              continue;
            }
          }
        }

        iface.Libraries.emplace_back(std::move(item));
      }
    }
    if (cge->GetHadHeadSensitiveCondition()) {
      iface.HadHeadSensitiveCondition = true;
    }
    if (cge->GetHadContextSensitiveCondition()) {
      iface.HadContextSensitiveCondition = true;
    }
    if (cge->GetHadLinkLanguageSensitiveCondition()) {
      iface.HadLinkLanguageSensitiveCondition = true;
    }
  }
}

cmLinkInterface const* cmGeneratorTarget::GetLinkInterface(
  const std::string& config, cmGeneratorTarget const* head) const
{
  return this->GetLinkInterface(config, head, false);
}

cmLinkInterface const* cmGeneratorTarget::GetLinkInterface(
  const std::string& config, cmGeneratorTarget const* head,
  bool secondPass) const
{
  // Imported targets have their own link interface.
  if (this->IsImported()) {
    return this->GetImportLinkInterface(config, head, UseTo::Link, secondPass);
  }

  // Link interfaces are not supported for executables that do not
  // export symbols.
  if (this->GetType() == cmStateEnums::EXECUTABLE &&
      !this->IsExecutableWithExports()) {
    return nullptr;
  }

  // Lookup any existing link interface for this configuration.
  cmHeadToLinkInterfaceMap& hm = this->GetHeadToLinkInterfaceMap(config);

  // If the link interface does not depend on the head target
  // then reuse the one from the head we computed first.
  if (!hm.empty() && !hm.begin()->second.HadHeadSensitiveCondition) {
    head = hm.begin()->first;
  }

  cmOptionalLinkInterface& iface = hm[head];
  if (secondPass) {
    iface = cmOptionalLinkInterface();
  }
  MaybeEnableCheckLinkLibraries(iface);
  if (!iface.LibrariesDone) {
    iface.LibrariesDone = true;
    this->ComputeLinkInterfaceLibraries(config, iface, head, UseTo::Link);
  }
  if (!iface.AllDone) {
    iface.AllDone = true;
    if (iface.Exists) {
      this->ComputeLinkInterface(config, iface, head, secondPass);
      this->ComputeLinkInterfaceRuntimeLibraries(config, iface);
    }
  }

  return iface.Exists ? &iface : nullptr;
}

void cmGeneratorTarget::ComputeLinkInterface(
  const std::string& config, cmOptionalLinkInterface& iface,
  cmGeneratorTarget const* headTarget) const
{
  this->ComputeLinkInterface(config, iface, headTarget, false);
}

void cmGeneratorTarget::ComputeLinkInterface(
  const std::string& config, cmOptionalLinkInterface& iface,
  cmGeneratorTarget const* headTarget, bool secondPass) const
{
  if (iface.Explicit) {
    if (this->GetType() == cmStateEnums::SHARED_LIBRARY ||
        this->GetType() == cmStateEnums::STATIC_LIBRARY ||
        this->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
      // Shared libraries may have runtime implementation dependencies
      // on other shared libraries that are not in the interface.
      std::set<cmLinkItem> emitted;
      for (cmLinkItem const& lib : iface.Libraries) {
        emitted.insert(lib);
      }
      if (this->GetType() != cmStateEnums::INTERFACE_LIBRARY) {
        cmLinkImplementation const* impl =
          this->GetLinkImplementation(config, UseTo::Link, secondPass);
        for (cmLinkImplItem const& lib : impl->Libraries) {
          if (emitted.insert(lib).second) {
            if (lib.Target) {
              // This is a runtime dependency on another shared library.
              if (lib.Target->GetType() == cmStateEnums::SHARED_LIBRARY) {
                iface.SharedDeps.push_back(lib);
              }
            } else {
              // TODO: Recognize shared library file names.  Perhaps this
              // should be moved to cmComputeLinkInformation, but that
              // creates a chicken-and-egg problem since this list is needed
              // for its construction.
            }
          }
        }
      }
    }
  } else if (this->GetPolicyStatusCMP0022() == cmPolicies::WARN ||
             this->GetPolicyStatusCMP0022() == cmPolicies::OLD) {
    // The link implementation is the default link interface.
    cmLinkImplementationLibraries const* impl =
      this->GetLinkImplementationLibrariesInternal(config, headTarget,
                                                   UseTo::Link);
    iface.ImplementationIsInterface = true;
    iface.WrongConfigLibraries = impl->WrongConfigLibraries;
  }

  if (this->LinkLanguagePropagatesToDependents()) {
    // Targets using this archive need its language runtime libraries.
    if (cmLinkImplementation const* impl =
          this->GetLinkImplementation(config, UseTo::Link, secondPass)) {
      iface.Languages = impl->Languages;
    }
  }

  if (this->GetType() == cmStateEnums::STATIC_LIBRARY) {
    // Construct the property name suffix for this configuration.
    std::string suffix = "_";
    if (!config.empty()) {
      suffix += cmSystemTools::UpperCase(config);
    } else {
      suffix += "NOCONFIG";
    }

    // How many repetitions are needed if this library has cyclic
    // dependencies?
    std::string propName = cmStrCat("LINK_INTERFACE_MULTIPLICITY", suffix);
    if (cmValue config_reps = this->GetProperty(propName)) {
      sscanf(config_reps->c_str(), "%u", &iface.Multiplicity);
    } else if (cmValue reps =
                 this->GetProperty("LINK_INTERFACE_MULTIPLICITY")) {
      sscanf(reps->c_str(), "%u", &iface.Multiplicity);
    }
  }
}

const cmLinkInterfaceLibraries* cmGeneratorTarget::GetLinkInterfaceLibraries(
  const std::string& config, cmGeneratorTarget const* head, UseTo usage) const
{
  // Imported targets have their own link interface.
  if (this->IsImported()) {
    return this->GetImportLinkInterface(config, head, usage);
  }

  // Link interfaces are not supported for executables that do not
  // export symbols.
  if (this->GetType() == cmStateEnums::EXECUTABLE &&
      !this->IsExecutableWithExports()) {
    return nullptr;
  }

  // Lookup any existing link interface for this configuration.
  cmHeadToLinkInterfaceMap& hm =
    (usage == UseTo::Compile
       ? this->GetHeadToLinkInterfaceUsageRequirementsMap(config)
       : this->GetHeadToLinkInterfaceMap(config));

  // If the link interface does not depend on the head target
  // then reuse the one from the head we computed first.
  if (!hm.empty() && !hm.begin()->second.HadHeadSensitiveCondition) {
    head = hm.begin()->first;
  }

  cmOptionalLinkInterface& iface = hm[head];
  MaybeEnableCheckLinkLibraries(iface);
  if (!iface.LibrariesDone) {
    iface.LibrariesDone = true;
    this->ComputeLinkInterfaceLibraries(config, iface, head, usage);
  }

  return iface.Exists ? &iface : nullptr;
}

void cmGeneratorTarget::ComputeLinkInterfaceLibraries(
  const std::string& config, cmOptionalLinkInterface& iface,
  cmGeneratorTarget const* headTarget, UseTo usage) const
{
  // Construct the property name suffix for this configuration.
  std::string suffix = "_";
  if (!config.empty()) {
    suffix += cmSystemTools::UpperCase(config);
  } else {
    suffix += "NOCONFIG";
  }

  // An explicit list of interface libraries may be set for shared
  // libraries and executables that export symbols.
  bool haveExplicitLibraries = false;
  cmValue explicitLibrariesCMP0022OLD;
  std::string linkIfacePropCMP0022OLD;
  bool const cmp0022NEW = (this->GetPolicyStatusCMP0022() != cmPolicies::OLD &&
                           this->GetPolicyStatusCMP0022() != cmPolicies::WARN);
  if (cmp0022NEW) {
    // CMP0022 NEW behavior is to use INTERFACE_LINK_LIBRARIES.
    haveExplicitLibraries = !this->Target->GetLinkInterfaceEntries().empty() ||
      !this->Target->GetLinkInterfaceDirectEntries().empty() ||
      !this->Target->GetLinkInterfaceDirectExcludeEntries().empty();
  } else {
    // CMP0022 OLD behavior is to use LINK_INTERFACE_LIBRARIES if set on a
    // shared lib or executable.
    if (this->GetType() == cmStateEnums::SHARED_LIBRARY ||
        this->IsExecutableWithExports()) {
      // Lookup the per-configuration property.
      linkIfacePropCMP0022OLD = cmStrCat("LINK_INTERFACE_LIBRARIES", suffix);
      explicitLibrariesCMP0022OLD = this->GetProperty(linkIfacePropCMP0022OLD);

      // If not set, try the generic property.
      if (!explicitLibrariesCMP0022OLD) {
        linkIfacePropCMP0022OLD = "LINK_INTERFACE_LIBRARIES";
        explicitLibrariesCMP0022OLD =
          this->GetProperty(linkIfacePropCMP0022OLD);
      }
    }

    if (explicitLibrariesCMP0022OLD &&
        this->GetPolicyStatusCMP0022() == cmPolicies::WARN &&
        !this->PolicyWarnedCMP0022) {
      // Compare the explicitly set old link interface properties to the
      // preferred new link interface property one and warn if different.
      cmValue newExplicitLibraries =
        this->GetProperty("INTERFACE_LINK_LIBRARIES");
      if (newExplicitLibraries &&
          (*newExplicitLibraries != *explicitLibrariesCMP0022OLD)) {
        std::ostringstream w;
        /* clang-format off */
        w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0022) << "\n"
          "Target \"" << this->GetName() << "\" has an "
          "INTERFACE_LINK_LIBRARIES property which differs from its " <<
          linkIfacePropCMP0022OLD << " properties."
          "\n"
          "INTERFACE_LINK_LIBRARIES:\n"
          "  " << *newExplicitLibraries << "\n" <<
          linkIfacePropCMP0022OLD << ":\n"
          "  " << *explicitLibrariesCMP0022OLD << "\n";
        /* clang-format on */
        this->LocalGenerator->IssueMessage(MessageType::AUTHOR_WARNING,
                                           w.str());
        this->PolicyWarnedCMP0022 = true;
      }
    }

    haveExplicitLibraries = static_cast<bool>(explicitLibrariesCMP0022OLD);
  }

  // There is no implicit link interface for executables or modules
  // so if none was explicitly set then there is no link interface.
  if (!haveExplicitLibraries &&
      (this->GetType() == cmStateEnums::EXECUTABLE ||
       (this->GetType() == cmStateEnums::MODULE_LIBRARY))) {
    return;
  }
  iface.Exists = true;

  // If CMP0022 is NEW then the plain tll signature sets the
  // INTERFACE_LINK_LIBRARIES property.  Even if the project
  // clears it, the link interface is still explicit.
  iface.Explicit = cmp0022NEW || explicitLibrariesCMP0022OLD;

  if (cmp0022NEW) {
    // The interface libraries are specified by INTERFACE_LINK_LIBRARIES.
    // Use its special representation directly to get backtraces.
    this->ExpandLinkItems(
      kINTERFACE_LINK_LIBRARIES, this->Target->GetLinkInterfaceEntries(),
      config, headTarget, usage, LinkInterfaceField::Libraries, iface);
    this->ExpandLinkItems(kINTERFACE_LINK_LIBRARIES_DIRECT,
                          this->Target->GetLinkInterfaceDirectEntries(),
                          config, headTarget, usage,
                          LinkInterfaceField::HeadInclude, iface);
    this->ExpandLinkItems(kINTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE,
                          this->Target->GetLinkInterfaceDirectExcludeEntries(),
                          config, headTarget, usage,
                          LinkInterfaceField::HeadExclude, iface);
  } else if (explicitLibrariesCMP0022OLD) {
    // The interface libraries have been explicitly set in pre-CMP0022 style.
    std::vector<BT<std::string>> entries;
    entries.emplace_back(*explicitLibrariesCMP0022OLD);
    this->ExpandLinkItems(linkIfacePropCMP0022OLD, cmMakeRange(entries),
                          config, headTarget, usage,
                          LinkInterfaceField::Libraries, iface);
  }

  // If the link interface is explicit, do not fall back to the link impl.
  if (iface.Explicit) {
    return;
  }

  // The link implementation is the default link interface.
  if (cmLinkImplementationLibraries const* impl =
        this->GetLinkImplementationLibrariesInternal(config, headTarget,
                                                     usage)) {
    iface.Libraries.insert(iface.Libraries.end(), impl->Libraries.begin(),
                           impl->Libraries.end());
    if (this->GetPolicyStatusCMP0022() == cmPolicies::WARN &&
        !this->PolicyWarnedCMP0022 && usage == UseTo::Link) {
      // Compare the link implementation fallback link interface to the
      // preferred new link interface property and warn if different.
      cmLinkInterface ifaceNew;
      this->ExpandLinkItems(
        kINTERFACE_LINK_LIBRARIES, this->Target->GetLinkInterfaceEntries(),
        config, headTarget, usage, LinkInterfaceField::Libraries, ifaceNew);
      if (ifaceNew.Libraries != iface.Libraries) {
        std::string oldLibraries = cmJoin(impl->Libraries, ";");
        std::string newLibraries = cmJoin(ifaceNew.Libraries, ";");
        if (oldLibraries.empty()) {
          oldLibraries = "(empty)";
        }
        if (newLibraries.empty()) {
          newLibraries = "(empty)";
        }

        std::ostringstream w;
        /* clang-format off */
        w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0022) << "\n"
          "Target \"" << this->GetName() << "\" has an "
          "INTERFACE_LINK_LIBRARIES property.  "
          "This should be preferred as the source of the link interface "
          "for this library but because CMP0022 is not set CMake is "
          "ignoring the property and using the link implementation "
          "as the link interface instead."
          "\n"
          "INTERFACE_LINK_LIBRARIES:\n"
          "  " << newLibraries << "\n"
          "Link implementation:\n"
          "  " << oldLibraries << "\n";
        /* clang-format on */
        this->LocalGenerator->IssueMessage(MessageType::AUTHOR_WARNING,
                                           w.str());
        this->PolicyWarnedCMP0022 = true;
      }
    }
  }
}

namespace {

template <typename ReturnType>
ReturnType constructItem(cmGeneratorTarget* target,
                         cmListFileBacktrace const& bt);

template <>
inline cmLinkImplItem constructItem(cmGeneratorTarget* target,
                                    cmListFileBacktrace const& bt)
{
  return cmLinkImplItem(cmLinkItem(target, false, bt), false);
}

template <>
inline cmLinkItem constructItem(cmGeneratorTarget* target,
                                cmListFileBacktrace const& bt)
{
  return cmLinkItem(target, false, bt);
}

template <typename ValueType>
std::vector<ValueType> computeImplicitLanguageTargets(
  std::string const& lang, std::string const& config,
  cmGeneratorTarget const* currentTarget)
{
  cmListFileBacktrace bt;
  std::vector<ValueType> result;
  cmLocalGenerator* lg = currentTarget->GetLocalGenerator();

  std::string const& runtimeLibrary =
    currentTarget->GetRuntimeLinkLibrary(lang, config);
  if (cmValue runtimeLinkOptions = currentTarget->Makefile->GetDefinition(
        "CMAKE_" + lang + "_RUNTIME_LIBRARIES_" + runtimeLibrary)) {
    cmList libsList{ *runtimeLinkOptions };
    result.reserve(libsList.size());

    for (auto const& i : libsList) {
      cmGeneratorTarget::TargetOrString resolved =
        currentTarget->ResolveTargetReference(i, lg);
      if (resolved.Target) {
        result.emplace_back(constructItem<ValueType>(resolved.Target, bt));
      }
    }
  }

  return result;
}
}

void cmGeneratorTarget::ComputeLinkInterfaceRuntimeLibraries(
  const std::string& config, cmOptionalLinkInterface& iface) const
{
  for (std::string const& lang : iface.Languages) {
    if ((lang == "CUDA" || lang == "HIP") &&
        iface.LanguageRuntimeLibraries.find(lang) ==
          iface.LanguageRuntimeLibraries.end()) {
      auto implicitTargets =
        computeImplicitLanguageTargets<cmLinkItem>(lang, config, this);
      iface.LanguageRuntimeLibraries[lang] = std::move(implicitTargets);
    }
  }
}

void cmGeneratorTarget::ComputeLinkImplementationRuntimeLibraries(
  const std::string& config, cmOptionalLinkImplementation& impl) const
{
  for (std::string const& lang : impl.Languages) {
    if ((lang == "CUDA" || lang == "HIP") &&
        impl.LanguageRuntimeLibraries.find(lang) ==
          impl.LanguageRuntimeLibraries.end()) {
      auto implicitTargets =
        computeImplicitLanguageTargets<cmLinkImplItem>(lang, config, this);
      impl.LanguageRuntimeLibraries[lang] = std::move(implicitTargets);
    }
  }
}

const cmLinkInterface* cmGeneratorTarget::GetImportLinkInterface(
  const std::string& config, cmGeneratorTarget const* headTarget, UseTo usage,
  bool secondPass) const
{
  cmGeneratorTarget::ImportInfo const* info = this->GetImportInfo(config);
  if (!info) {
    return nullptr;
  }

  cmHeadToLinkInterfaceMap& hm =
    (usage == UseTo::Compile
       ? this->GetHeadToLinkInterfaceUsageRequirementsMap(config)
       : this->GetHeadToLinkInterfaceMap(config));

  // If the link interface does not depend on the head target
  // then reuse the one from the head we computed first.
  if (!hm.empty() && !hm.begin()->second.HadHeadSensitiveCondition) {
    headTarget = hm.begin()->first;
  }

  cmOptionalLinkInterface& iface = hm[headTarget];
  if (secondPass) {
    iface = cmOptionalLinkInterface();
  }
  MaybeEnableCheckLinkLibraries(iface);
  if (!iface.AllDone) {
    iface.AllDone = true;
    iface.LibrariesDone = true;
    iface.Multiplicity = info->Multiplicity;
    cmExpandList(info->Languages, iface.Languages);
    this->ExpandLinkItems(kINTERFACE_LINK_LIBRARIES_DIRECT,
                          cmMakeRange(info->LibrariesHeadInclude), config,
                          headTarget, usage, LinkInterfaceField::HeadInclude,
                          iface);
    this->ExpandLinkItems(kINTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE,
                          cmMakeRange(info->LibrariesHeadExclude), config,
                          headTarget, usage, LinkInterfaceField::HeadExclude,
                          iface);
    this->ExpandLinkItems(info->LibrariesProp, cmMakeRange(info->Libraries),
                          config, headTarget, usage,
                          LinkInterfaceField::Libraries, iface);
    cmList deps{ info->SharedDeps };
    LookupLinkItemScope scope{ this->LocalGenerator };

    auto linkFeature = cmLinkItem::DEFAULT;
    for (auto const& dep : deps) {
      if (auto maybeLinkFeature = ParseLinkFeature(dep)) {
        linkFeature = std::move(*maybeLinkFeature);
        continue;
      }

      if (cm::optional<cmLinkItem> maybeItem = this->LookupLinkItem(
            dep, cmListFileBacktrace(), linkFeature, &scope, LookupSelf::No)) {
        iface.SharedDeps.emplace_back(std::move(*maybeItem));
      }
    }
  }

  return &iface;
}

cmHeadToLinkInterfaceMap& cmGeneratorTarget::GetHeadToLinkInterfaceMap(
  const std::string& config) const
{
  return this->LinkInterfaceMap[cmSystemTools::UpperCase(config)];
}

cmHeadToLinkInterfaceMap&
cmGeneratorTarget::GetHeadToLinkInterfaceUsageRequirementsMap(
  const std::string& config) const
{
  return this
    ->LinkInterfaceUsageRequirementsOnlyMap[cmSystemTools::UpperCase(config)];
}

const cmLinkImplementation* cmGeneratorTarget::GetLinkImplementation(
  const std::string& config, UseTo usage) const
{
  return this->GetLinkImplementation(config, usage, false);
}

const cmLinkImplementation* cmGeneratorTarget::GetLinkImplementation(
  const std::string& config, UseTo usage, bool secondPass) const
{
  // There is no link implementation for targets that cannot compile sources.
  if (!this->CanCompileSources()) {
    return nullptr;
  }

  HeadToLinkImplementationMap& hm =
    (usage == UseTo::Compile
       ? this->GetHeadToLinkImplementationUsageRequirementsMap(config)
       : this->GetHeadToLinkImplementationMap(config));
  cmOptionalLinkImplementation& impl = hm[this];
  if (secondPass) {
    impl = cmOptionalLinkImplementation();
  }
  MaybeEnableCheckLinkLibraries(impl);
  if (!impl.LibrariesDone) {
    impl.LibrariesDone = true;
    this->ComputeLinkImplementationLibraries(config, impl, this, usage);
  }
  if (!impl.LanguagesDone) {
    impl.LanguagesDone = true;
    this->ComputeLinkImplementationLanguages(config, impl);
    this->ComputeLinkImplementationRuntimeLibraries(config, impl);
  }
  return &impl;
}

cmGeneratorTarget::HeadToLinkImplementationMap&
cmGeneratorTarget::GetHeadToLinkImplementationMap(
  std::string const& config) const
{
  return this->LinkImplMap[cmSystemTools::UpperCase(config)];
}

cmGeneratorTarget::HeadToLinkImplementationMap&
cmGeneratorTarget::GetHeadToLinkImplementationUsageRequirementsMap(
  std::string const& config) const
{
  return this
    ->LinkImplUsageRequirementsOnlyMap[cmSystemTools::UpperCase(config)];
}

cmLinkImplementationLibraries const*
cmGeneratorTarget::GetLinkImplementationLibraries(const std::string& config,
                                                  UseTo usage) const
{
  return this->GetLinkImplementationLibrariesInternal(config, this, usage);
}

cmLinkImplementationLibraries const*
cmGeneratorTarget::GetLinkImplementationLibrariesInternal(
  const std::string& config, cmGeneratorTarget const* head, UseTo usage) const
{
  // There is no link implementation for targets that cannot compile sources.
  if (!this->CanCompileSources()) {
    return nullptr;
  }

  // Populate the link implementation libraries for this configuration.
  HeadToLinkImplementationMap& hm =
    (usage == UseTo::Compile
       ? this->GetHeadToLinkImplementationUsageRequirementsMap(config)
       : this->GetHeadToLinkImplementationMap(config));

  // If the link implementation does not depend on the head target
  // then reuse the one from the head we computed first.
  if (!hm.empty() && !hm.begin()->second.HadHeadSensitiveCondition) {
    head = hm.begin()->first;
  }

  cmOptionalLinkImplementation& impl = hm[head];
  MaybeEnableCheckLinkLibraries(impl);
  if (!impl.LibrariesDone) {
    impl.LibrariesDone = true;
    this->ComputeLinkImplementationLibraries(config, impl, head, usage);
  }
  return &impl;
}

namespace {
class TransitiveLinkImpl
{
  cmGeneratorTarget const* Self;
  std::string const& Config;
  UseTo ImplFor;
  cmLinkImplementation& Impl;

  std::set<cmLinkItem> Emitted;
  std::set<cmLinkItem> Excluded;
  std::unordered_set<cmGeneratorTarget const*> Followed;

  void Follow(cmGeneratorTarget const* target);

public:
  TransitiveLinkImpl(cmGeneratorTarget const* self, std::string const& config,
                     UseTo usage, cmLinkImplementation& impl)
    : Self(self)
    , Config(config)
    , ImplFor(usage)
    , Impl(impl)
  {
  }

  void Compute();
};

void TransitiveLinkImpl::Follow(cmGeneratorTarget const* target)
{
  if (!target || !this->Followed.insert(target).second ||
      target->GetPolicyStatusCMP0022() == cmPolicies::OLD ||
      target->GetPolicyStatusCMP0022() == cmPolicies::WARN) {
    return;
  }

  // Get this target's usage requirements.
  cmLinkInterfaceLibraries const* iface =
    target->GetLinkInterfaceLibraries(this->Config, this->Self, this->ImplFor);
  if (!iface) {
    return;
  }
  if (iface->HadContextSensitiveCondition) {
    this->Impl.HadContextSensitiveCondition = true;
  }

  // Process 'INTERFACE_LINK_LIBRARIES_DIRECT' usage requirements.
  for (cmLinkItem const& item : iface->HeadInclude) {
    // Inject direct dependencies from the item's usage requirements
    // before the item itself.
    this->Follow(item.Target);

    // Add the item itself, but at most once.
    if (this->Emitted.insert(item).second) {
      this->Impl.Libraries.emplace_back(item, /* checkCMP0027= */ false);
    }
  }

  // Follow transitive dependencies.
  for (cmLinkItem const& item : iface->Libraries) {
    this->Follow(item.Target);
  }

  // Record exclusions from 'INTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE'
  // usage requirements.
  for (cmLinkItem const& item : iface->HeadExclude) {
    this->Excluded.insert(item);
  }
}

void TransitiveLinkImpl::Compute()
{
  // Save the original items and start with an empty list.
  std::vector<cmLinkImplItem> original = std::move(this->Impl.Libraries);

  // Avoid injecting any original items as usage requirements.
  // This gives LINK_LIBRARIES final control over the order
  // if it explicitly lists everything.
  this->Emitted.insert(original.cbegin(), original.cend());

  // Process each original item.
  for (cmLinkImplItem& item : original) {
    // Inject direct dependencies listed in 'INTERFACE_LINK_LIBRARIES_DIRECT'
    // usage requirements before the item itself.
    this->Follow(item.Target);

    // Add the item itself.
    this->Impl.Libraries.emplace_back(std::move(item));
  }

  // Remove items listed in 'INTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE'
  // usage requirements found through any dependency above.
  this->Impl.Libraries.erase(
    std::remove_if(this->Impl.Libraries.begin(), this->Impl.Libraries.end(),
                   [this](cmLinkImplItem const& item) {
                     return this->Excluded.find(item) != this->Excluded.end();
                   }),
    this->Impl.Libraries.end());
}

void ComputeLinkImplTransitive(cmGeneratorTarget const* self,
                               std::string const& config, UseTo usage,
                               cmLinkImplementation& impl)
{
  TransitiveLinkImpl transitiveLinkImpl(self, config, usage, impl);
  transitiveLinkImpl.Compute();
}
}

void cmGeneratorTarget::ComputeLinkImplementationLibraries(
  const std::string& config, cmOptionalLinkImplementation& impl,
  cmGeneratorTarget const* head, UseTo usage) const
{
  cmLocalGenerator const* lg = this->LocalGenerator;
  cmMakefile const* mf = lg->GetMakefile();
  cmBTStringRange entryRange = this->Target->GetLinkImplementationEntries();
  auto const& synthTargetsForConfig = this->Configs[config].SyntheticDeps;
  // Collect libraries directly linked in this configuration.
  for (auto const& entry : entryRange) {
    // Keep this logic in sync with ExpandLinkItems.
    cmGeneratorExpressionDAGChecker dagChecker(
      this, "LINK_LIBRARIES", nullptr, nullptr, this->LocalGenerator, config);
    // The $<LINK_ONLY> expression may be used to specify link dependencies
    // that are otherwise excluded from usage requirements.
    if (usage == UseTo::Compile) {
      dagChecker.SetTransitivePropertiesOnly();
      switch (this->GetPolicyStatusCMP0131()) {
        case cmPolicies::WARN:
        case cmPolicies::OLD:
          break;
        case cmPolicies::REQUIRED_IF_USED:
        case cmPolicies::REQUIRED_ALWAYS:
        case cmPolicies::NEW:
          dagChecker.SetTransitivePropertiesOnlyCMP0131();
          break;
      }
    }
    cmGeneratorExpression ge(*this->LocalGenerator->GetCMakeInstance(),
                             entry.Backtrace);
    std::unique_ptr<cmCompiledGeneratorExpression> const cge =
      ge.Parse(entry.Value);
    cge->SetEvaluateForBuildsystem(true);
    std::string const& evaluated =
      cge->Evaluate(this->LocalGenerator, config, head, &dagChecker, nullptr,
                    this->LinkerLanguage);
    bool const checkCMP0027 = evaluated != entry.Value;
    cmList llibs(evaluated);
    if (cge->GetHadHeadSensitiveCondition()) {
      impl.HadHeadSensitiveCondition = true;
    }
    if (cge->GetHadContextSensitiveCondition()) {
      impl.HadContextSensitiveCondition = true;
    }
    if (cge->GetHadLinkLanguageSensitiveCondition()) {
      impl.HadLinkLanguageSensitiveCondition = true;
    }

    auto linkFeature = cmLinkItem::DEFAULT;
    for (auto const& lib : llibs) {
      if (auto maybeLinkFeature = ParseLinkFeature(lib)) {
        linkFeature = std::move(*maybeLinkFeature);
        continue;
      }

      if (this->IsLinkLookupScope(lib, lg)) {
        continue;
      }

      // Skip entries that resolve to the target itself or are empty.
      std::string name = this->CheckCMP0004(lib);
      if (this->GetPolicyStatusCMP0108() == cmPolicies::NEW) {
        // resolve alias name
        auto* target = this->Makefile->FindTargetToUse(name);
        if (target) {
          name = target->GetName();
        }
      }
      if (name == this->GetName() || name.empty()) {
        if (name == this->GetName()) {
          bool noMessage = false;
          MessageType messageType = MessageType::FATAL_ERROR;
          std::ostringstream e;
          switch (this->GetPolicyStatusCMP0038()) {
            case cmPolicies::WARN: {
              e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0038) << "\n";
              messageType = MessageType::AUTHOR_WARNING;
            } break;
            case cmPolicies::OLD:
              noMessage = true;
              break;
            case cmPolicies::REQUIRED_IF_USED:
            case cmPolicies::REQUIRED_ALWAYS:
            case cmPolicies::NEW:
              // Issue the fatal message.
              break;
          }

          if (!noMessage) {
            e << "Target \"" << this->GetName() << "\" links to itself.";
            this->LocalGenerator->GetCMakeInstance()->IssueMessage(
              messageType, e.str(), this->GetBacktrace());
            if (messageType == MessageType::FATAL_ERROR) {
              return;
            }
          }
        }
        continue;
      }

      // The entry is meant for this configuration.
      cmLinkItem item = this->ResolveLinkItem(
        BT<std::string>(name, entry.Backtrace), lg, linkFeature);
      if (item.Target) {
        auto depsForTarget = synthTargetsForConfig.find(item.Target);
        if (depsForTarget != synthTargetsForConfig.end()) {
          for (auto const* depForTarget : depsForTarget->second) {
            cmLinkItem synthItem(depForTarget, item.Cross, item.Backtrace);
            impl.Libraries.emplace_back(std::move(synthItem), false);
          }
        }
      } else {
        // Report explicitly linked object files separately.
        std::string const& maybeObj = item.AsStr();
        if (cmSystemTools::FileIsFullPath(maybeObj)) {
          cmSourceFile const* sf =
            mf->GetSource(maybeObj, cmSourceFileLocationKind::Known);
          if (sf && sf->GetPropertyAsBool("EXTERNAL_OBJECT")) {
            item.ObjectSource = sf;
            impl.Objects.emplace_back(std::move(item));
            continue;
          }
        }
      }

      impl.Libraries.emplace_back(std::move(item), checkCMP0027);
    }

    std::set<std::string> const& seenProps = cge->GetSeenTargetProperties();
    for (std::string const& sp : seenProps) {
      if (!this->GetProperty(sp)) {
        this->LinkImplicitNullProperties.insert(sp);
      }
    }
    cge->GetMaxLanguageStandard(this, this->MaxLanguageStandards);
  }

  // Update the list of direct link dependencies from usage requirements.
  if (head == this) {
    ComputeLinkImplTransitive(this, config, usage, impl);
  }

  // Get the list of configurations considered to be DEBUG.
  std::vector<std::string> debugConfigs =
    this->Makefile->GetCMakeInstance()->GetDebugConfigs();

  cmTargetLinkLibraryType linkType =
    CMP0003_ComputeLinkType(config, debugConfigs);
  cmTarget::LinkLibraryVectorType const& oldllibs =
    this->Target->GetOriginalLinkLibraries();

  auto linkFeature = cmLinkItem::DEFAULT;
  for (cmTarget::LibraryID const& oldllib : oldllibs) {
    if (auto maybeLinkFeature = ParseLinkFeature(oldllib.first)) {
      linkFeature = std::move(*maybeLinkFeature);
      continue;
    }

    if (oldllib.second != GENERAL_LibraryType && oldllib.second != linkType) {
      std::string name = this->CheckCMP0004(oldllib.first);
      if (name == this->GetName() || name.empty()) {
        continue;
      }
      // Support OLD behavior for CMP0003.
      impl.WrongConfigLibraries.push_back(
        this->ResolveLinkItem(BT<std::string>(name), linkFeature));
    }
  }
}

cmGeneratorTarget::TargetOrString cmGeneratorTarget::ResolveTargetReference(
  std::string const& name) const
{
  return this->ResolveTargetReference(name, this->LocalGenerator);
}

cmGeneratorTarget::TargetOrString cmGeneratorTarget::ResolveTargetReference(
  std::string const& name, cmLocalGenerator const* lg) const
{
  TargetOrString resolved;

  if (cmGeneratorTarget* tgt = lg->FindGeneratorTargetToUse(name)) {
    resolved.Target = tgt;
  } else {
    resolved.String = name;
  }

  return resolved;
}

cmLinkItem cmGeneratorTarget::ResolveLinkItem(
  BT<std::string> const& name, std::string const& linkFeature) const
{
  return this->ResolveLinkItem(name, this->LocalGenerator, linkFeature);
}

cmLinkItem cmGeneratorTarget::ResolveLinkItem(
  BT<std::string> const& name, cmLocalGenerator const* lg,
  std::string const& linkFeature) const
{
  auto bt = name.Backtrace;
  TargetOrString resolved = this->ResolveTargetReference(name.Value, lg);

  if (!resolved.Target) {
    return cmLinkItem(resolved.String, false, bt, linkFeature);
  }

  // Check deprecation, issue message with `bt` backtrace.
  if (resolved.Target->IsDeprecated()) {
    std::ostringstream w;
    /* clang-format off */
    w <<
      "The library that is being linked to, "  << resolved.Target->GetName() <<
      ", is marked as being deprecated by the owner.  The message provided by "
      "the developer is: \n" << resolved.Target->GetDeprecation() << "\n";
    /* clang-format on */
    this->LocalGenerator->GetCMakeInstance()->IssueMessage(
      MessageType::AUTHOR_WARNING, w.str(), bt);
  }

  // Skip targets that will not really be linked.  This is probably a
  // name conflict between an external library and an executable
  // within the project.
  if (resolved.Target->GetType() == cmStateEnums::EXECUTABLE &&
      !resolved.Target->IsExecutableWithExports()) {
    return cmLinkItem(resolved.Target->GetName(), false, bt, linkFeature);
  }

  return cmLinkItem(resolved.Target, false, bt, linkFeature);
}
