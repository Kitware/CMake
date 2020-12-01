/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "cmsys/RegularExpression.hxx"

#include "cmDepends.h"

class cmLocalUnixMakefileGenerator3;

/** \class cmDependsC
 * \brief Dependency scanner for C and C++ object files.
 */
class cmDependsC : public cmDepends
{
public:
  /** Checking instances need to know the build directory name and the
      relative path from the build directory to the target file.  */
  cmDependsC();
  cmDependsC(cmLocalUnixMakefileGenerator3* lg, const std::string& targetDir,
             const std::string& lang, const DependencyMap* validDeps);

  /** Virtual destructor to cleanup subclasses properly.  */
  ~cmDependsC() override;

  cmDependsC(cmDependsC const&) = delete;
  cmDependsC& operator=(cmDependsC const&) = delete;

protected:
  // Implement writing/checking methods required by superclass.
  bool WriteDependencies(const std::set<std::string>& sources,
                         const std::string& obj, std::ostream& makeDepends,
                         std::ostream& internalDepends) override;

  // Method to scan a single file.
  void Scan(std::istream& is, const std::string& directory,
            const std::string& fullName);

  // Regular expression to identify C preprocessor include directives.
  cmsys::RegularExpression IncludeRegexLine;

  // Regular expressions to choose which include files to scan
  // recursively and which to complain about not finding.
  cmsys::RegularExpression IncludeRegexScan;
  cmsys::RegularExpression IncludeRegexComplain;
  std::string IncludeRegexLineString;
  std::string IncludeRegexScanString;
  std::string IncludeRegexComplainString;

  // Regex to transform #include lines.
  std::string IncludeRegexTransformString;
  cmsys::RegularExpression IncludeRegexTransform;
  using TransformRulesType = std::map<std::string, std::string>;
  TransformRulesType TransformRules;
  void SetupTransforms();
  void ParseTransform(std::string const& xform);
  void TransformLine(std::string& line);

public:
  // Data structures for dependency graph walk.
  struct UnscannedEntry
  {
    std::string FileName;
    std::string QuotedLocation;
  };

  struct cmIncludeLines
  {
    std::vector<UnscannedEntry> UnscannedEntries;
    bool Used = false;
  };

protected:
  const DependencyMap* ValidDeps = nullptr;
  std::set<std::string> Encountered;
  std::queue<UnscannedEntry> Unscanned;

  std::map<std::string, cmIncludeLines> FileCache;
  std::map<std::string, std::string> HeaderLocationCache;

  std::string CacheFileName;

  void WriteCacheFile() const;
  void ReadCacheFile();
};
