/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFileTimeComparison.h"

#include <string>
#include <unordered_map>
#include <utility>

class cmFileTimeComparisonInternal
{
public:
  inline bool Load(std::string const& fname, cmFileTime& ftm);
  inline bool FileTimeCompare(std::string const& f1, std::string const& f2,
                              int* result);

  bool FileTimesDiffer(std::string const& f1, std::string const& f2);

private:
  typedef std::unordered_map<std::string, cmFileTime> FileStatsMap;
  FileStatsMap Files;
};

bool cmFileTimeComparisonInternal::Load(std::string const& fname,
                                        cmFileTime& ftm)
{
  // Use the stored time if available.
  {
    auto fit = this->Files.find(fname);
    if (fit != this->Files.end()) {
      ftm = fit->second;
      return true;
    }
  }
  // Read file time from OS
  if (!ftm.Load(fname)) {
    return false;
  }
  // Store file time in cache
  this->Files[fname] = ftm;
  return true;
}

cmFileTimeComparison::cmFileTimeComparison()
{
  this->Internals = new cmFileTimeComparisonInternal;
}

cmFileTimeComparison::~cmFileTimeComparison()
{
  delete this->Internals;
}

bool cmFileTimeComparison::Load(std::string const& fileName,
                                cmFileTime& fileTime)
{
  return this->Internals->Load(fileName, fileTime);
}

bool cmFileTimeComparison::FileTimeCompare(std::string const& f1,
                                           std::string const& f2, int* result)
{
  return this->Internals->FileTimeCompare(f1, f2, result);
}

bool cmFileTimeComparison::FileTimesDiffer(std::string const& f1,
                                           std::string const& f2)
{
  return this->Internals->FileTimesDiffer(f1, f2);
}

bool cmFileTimeComparisonInternal::FileTimeCompare(std::string const& f1,
                                                   std::string const& f2,
                                                   int* result)
{
  // Get the modification time for each file.
  cmFileTime ft1, ft2;
  if (this->Load(f1, ft1) && this->Load(f2, ft2)) {
    // Compare the two modification times.
    *result = ft1.Compare(ft2);
    return true;
  }
  // No comparison available.  Default to the same time.
  *result = 0;
  return false;
}

bool cmFileTimeComparisonInternal::FileTimesDiffer(std::string const& f1,
                                                   std::string const& f2)
{
  // Get the modification time for each file.
  cmFileTime ft1, ft2;
  if (this->Load(f1, ft1) && this->Load(f2, ft2)) {
    // Compare the two modification times.
    return ft1.DifferS(ft2);
  }
  // No comparison available.  Default to different times.
  return true;
}
