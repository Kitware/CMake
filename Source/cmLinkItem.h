/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <cmext/algorithm>

#include "cmListFileCache.h"
#include "cmSystemTools.h"
#include "cmTargetLinkLibraryType.h"

class cmGeneratorTarget;
class cmSourceFile;

// Basic information about each link item.
class cmLinkItem
{
  std::string String;

public:
  cmLinkItem();
  cmLinkItem(std::string s, bool c, cmListFileBacktrace bt);
  cmLinkItem(cmGeneratorTarget const* t, bool c, cmListFileBacktrace bt);
  std::string const& AsStr() const;
  cmGeneratorTarget const* Target = nullptr;
  // The source file representing the external object (used when linking
  // `$<TARGET_OBJECTS>`)
  cmSourceFile const* ObjectSource = nullptr;
  bool Cross = false;
  cmListFileBacktrace Backtrace;
  friend bool operator<(cmLinkItem const& l, cmLinkItem const& r);
  friend bool operator==(cmLinkItem const& l, cmLinkItem const& r);
  friend std::ostream& operator<<(std::ostream& os, cmLinkItem const& item);
};

class cmLinkImplItem : public cmLinkItem
{
public:
  cmLinkImplItem();
  cmLinkImplItem(cmLinkItem item, bool checkCMP0027);
  bool CheckCMP0027 = false;
};

/** The link implementation specifies the direct library
    dependencies needed by the object files of the target.  */
struct cmLinkImplementationLibraries
{
  // Libraries linked directly in this configuration.
  std::vector<cmLinkImplItem> Libraries;

  // Object files linked directly in this configuration.
  std::vector<cmLinkItem> Objects;

  // Libraries linked directly in other configurations.
  // Needed only for OLD behavior of CMP0003.
  std::vector<cmLinkItem> WrongConfigLibraries;

  // Whether the list depends on a genex referencing the configuration.
  bool HadContextSensitiveCondition = false;
};

struct cmLinkInterfaceLibraries
{
  // Libraries listed in the interface.
  std::vector<cmLinkItem> Libraries;

  // Object files listed in the interface.
  std::vector<cmLinkItem> Objects;

  // Items to be included as if directly linked by the head target.
  std::vector<cmLinkItem> HeadInclude;

  // Items to be excluded from direct linking by the head target.
  std::vector<cmLinkItem> HeadExclude;

  // Whether the list depends on a genex referencing the head target.
  bool HadHeadSensitiveCondition = false;

  // Whether the list depends on a genex referencing the configuration.
  bool HadContextSensitiveCondition = false;
};

struct cmLinkInterface : public cmLinkInterfaceLibraries
{
  // Languages whose runtime libraries must be linked.
  std::vector<std::string> Languages;
  std::unordered_map<std::string, std::vector<cmLinkItem>>
    LanguageRuntimeLibraries;

  // Shared library dependencies needed for linking on some platforms.
  std::vector<cmLinkItem> SharedDeps;

  // Number of repetitions of a strongly connected component of two
  // or more static libraries.
  unsigned int Multiplicity = 0;

  // Libraries listed for other configurations.
  // Needed only for OLD behavior of CMP0003.
  std::vector<cmLinkItem> WrongConfigLibraries;

  bool ImplementationIsInterface = false;

  // Whether the list depends on a link language genex.
  bool HadLinkLanguageSensitiveCondition = false;
};

struct cmOptionalLinkInterface : public cmLinkInterface
{
  bool LibrariesDone = false;
  bool AllDone = false;
  bool Exists = false;
  bool Explicit = false;
};

struct cmHeadToLinkInterfaceMap
  : public std::map<cmGeneratorTarget const*, cmOptionalLinkInterface>
{
};

struct cmLinkImplementation : public cmLinkImplementationLibraries
{
  // Languages whose runtime libraries must be linked.
  std::vector<std::string> Languages;
  std::unordered_map<std::string, std::vector<cmLinkImplItem>>
    LanguageRuntimeLibraries;

  // Whether the list depends on a link language genex.
  bool HadLinkLanguageSensitiveCondition = false;
};

// Cache link implementation computation from each configuration.
struct cmOptionalLinkImplementation : public cmLinkImplementation
{
  bool LibrariesDone = false;
  bool LanguagesDone = false;
  bool HadHeadSensitiveCondition = false;
};

/** Compute the link type to use for the given configuration.  */
inline cmTargetLinkLibraryType CMP0003_ComputeLinkType(
  const std::string& config, std::vector<std::string> const& debugConfigs)
{
  // No configuration is always optimized.
  if (config.empty()) {
    return OPTIMIZED_LibraryType;
  }

  // Check if any entry in the list matches this configuration.
  std::string configUpper = cmSystemTools::UpperCase(config);
  if (cm::contains(debugConfigs, configUpper)) {
    return DEBUG_LibraryType;
  }
  // The current configuration is not a debug configuration.
  return OPTIMIZED_LibraryType;
}
