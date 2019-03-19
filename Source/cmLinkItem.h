/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmLinkItem_h
#define cmLinkItem_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <algorithm>
#include <map>
#include <ostream>
#include <string>
#include <vector>

#include "cmListFileCache.h"
#include "cmSystemTools.h"
#include "cmTargetLinkLibraryType.h"

class cmGeneratorTarget;

// Basic information about each link item.
class cmLinkItem
{
  std::string String;

public:
  cmLinkItem();
  cmLinkItem(std::string s, cmListFileBacktrace bt);
  cmLinkItem(cmGeneratorTarget const* t, cmListFileBacktrace bt);
  std::string const& AsStr() const;
  cmGeneratorTarget const* Target = nullptr;
  cmListFileBacktrace Backtrace;
  friend bool operator<(cmLinkItem const& l, cmLinkItem const& r);
  friend bool operator==(cmLinkItem const& l, cmLinkItem const& r);
  friend std::ostream& operator<<(std::ostream& os, cmLinkItem const& item);
};

class cmLinkImplItem : public cmLinkItem
{
public:
  cmLinkImplItem();
  cmLinkImplItem(cmLinkItem item, bool fromGenex);
  bool FromGenex = false;
};

/** The link implementation specifies the direct library
    dependencies needed by the object files of the target.  */
struct cmLinkImplementationLibraries
{
  // Libraries linked directly in this configuration.
  std::vector<cmLinkImplItem> Libraries;

  // Libraries linked directly in other configurations.
  // Needed only for OLD behavior of CMP0003.
  std::vector<cmLinkItem> WrongConfigLibraries;
};

struct cmLinkInterfaceLibraries
{
  // Libraries listed in the interface.
  std::vector<cmLinkItem> Libraries;
};

struct cmLinkInterface : public cmLinkInterfaceLibraries
{
  // Languages whose runtime libraries must be linked.
  std::vector<std::string> Languages;

  // Shared library dependencies needed for linking on some platforms.
  std::vector<cmLinkItem> SharedDeps;

  // Number of repetitions of a strongly connected component of two
  // or more static libraries.
  unsigned int Multiplicity = 0;

  // Libraries listed for other configurations.
  // Needed only for OLD behavior of CMP0003.
  std::vector<cmLinkItem> WrongConfigLibraries;

  bool ImplementationIsInterface = false;
};

struct cmOptionalLinkInterface : public cmLinkInterface
{
  bool LibrariesDone = false;
  bool AllDone = false;
  bool Exists = false;
  bool HadHeadSensitiveCondition = false;
  const char* ExplicitLibraries = nullptr;
};

struct cmHeadToLinkInterfaceMap
  : public std::map<cmGeneratorTarget const*, cmOptionalLinkInterface>
{
};

struct cmLinkImplementation : public cmLinkImplementationLibraries
{
  // Languages whose runtime libraries must be linked.
  std::vector<std::string> Languages;
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
  if (std::find(debugConfigs.begin(), debugConfigs.end(), configUpper) !=
      debugConfigs.end()) {
    return DEBUG_LibraryType;
  }
  // The current configuration is not a debug configuration.
  return OPTIMIZED_LibraryType;
}

#endif
