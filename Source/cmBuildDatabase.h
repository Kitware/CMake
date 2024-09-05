/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <cm/optional>

class cmGeneratorTarget;

class cmBuildDatabase
{
public:
  struct TranslationUnit
  {
    std::string WorkDirectory;
    std::string Source;
    cm::optional<std::string> Object;
    std::vector<std::string> Requires;
    std::map<std::string, std::string> Provides;
    std::vector<std::string> BaselineArguments;
    std::vector<std::string> LocalArguments;
    std::vector<std::string> Arguments;
    bool Private = false;
  };

  struct Set
  {
    std::string Name;
    std::string FamilyName;
    std::vector<std::string> VisibleSets;
    std::vector<TranslationUnit> TranslationUnits;
  };

  cmBuildDatabase();
  cmBuildDatabase(cmBuildDatabase const&);
  ~cmBuildDatabase();

  using LookupTable = std::map<std::string, TranslationUnit*>;
  // Generate a lookup table for the database.
  //
  // Only use when loading a single target's database in order to populate it.
  LookupTable GenerateLookupTable();

  bool HasPlaceholderNames() const;

  void Write(std::string const& path) const;

  static std::unique_ptr<cmBuildDatabase> Load(std::string const& path);
  static cmBuildDatabase Merge(std::vector<cmBuildDatabase> const& components);
  static cmBuildDatabase ForTarget(cmGeneratorTarget* gt,
                                   std::string const& config);

private:
  std::vector<Set> Sets;
};
